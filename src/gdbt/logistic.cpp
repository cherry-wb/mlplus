#include <cmath>
#include <cstring>
#include <vector>
#include <utility>
#include <algorithm>
#include "loss.h"
#include "logistic.h"
#include "sample_matrix.h"
#include "sample_set.h"
#include "cut.h"
#include "mlplus/common/utility.h"
#include "mlplus/interface/log.h"
#include "configuration.h"

#define FLOAT_LOG_MAX 20.0
namespace mlplus
{
namespace treelink
{

Logistic::Logistic(const Configuration* pConf)
    : Loss(pConf)
{
}

Logistic::~Logistic()
{
}

// Calculate F0: intialization of treelink
bool Logistic::InitEstimation(SampleMatrix* pSampleMatrix, float* pInitEst) const
{
    // Initialize all estimations with zeros.
    return Loss::InitEstimation(pSampleMatrix, pInitEst);
}

float Logistic::CalculateLoss(SampleMatrix* pSampMat) const
{
    double fLoss(0.0);
    const int nClassCount = pSampMat->GetClassCount();
    float* pProb = new float [nClassCount];
    const int nSampCount = pSampMat->GetSampleCount();
    float fWeight(1.0);
    bool bWeighting = pSampMat->GetWeighting();

    const double fExpLim = std::exp(-FLOAT_LOG_MAX);
    float fCurrLoss = 0.0;

    for(int idxSamp = 0; idxSamp < nSampCount; ++idxSamp)
    {
        const float* pEst = pSampMat->GetEstimation(idxSamp);
        common::utility::LogisticTransform(pEst, pProb, nClassCount);
        int nTarget = static_cast<int>(pSampMat->GetTarget(idxSamp));
        fCurrLoss = pProb[nTarget] <= fExpLim ? FLOAT_LOG_MAX : -std::log(pProb[nTarget]);
        if(bWeighting)
        {
            fWeight = pSampMat->GetWeight(idxSamp);
            fCurrLoss *= fWeight;
        }
        fLoss += fCurrLoss;
    }

    if(pProb)
        delete [] pProb;
    return fLoss;
}

bool Logistic::CalculateGradient(SampleSet* pSampleSet) const
{
    // Theory: gk = yk - pk
    const int nClassCount = pSampleSet->GetClassCount();
    float* pProb = new float [nClassCount];    // Probability distribution.
    float fWeight = 1.0;
    bool bWeighting = pSampleSet->GetWeighting();
    for(SampleSet::Iterator iterSample = pSampleSet->Begin();
            iterSample != pSampleSet->End(); ++iterSample)
    {
        float* pEst = iterSample.GetEstimation();
        common::utility::LogisticTransform(pEst, pProb, nClassCount);
        int nTarget = static_cast<int>(iterSample.GetTarget());
        float* pGrad = iterSample.GetGradient();
        for(int idxClass = 0; idxClass < nClassCount; ++idxClass)
        {
            float fGrad = 0.0;
            if(idxClass == nTarget)
                fGrad = 1.0 - pProb[idxClass];
            else
                fGrad = -pProb[idxClass];

            pGrad[idxClass] = fGrad;

            if(bWeighting)
            {
                fWeight = iterSample.GetWeight();
                pGrad[idxClass] *= fWeight;
            }
        }
    }
    if(pProb)
        delete [] pProb;
    return true;
}

bool Logistic::CalculateOptimal(SampleSet::Variable* pVariable, Cut* pCut) const
{
    CalculateIncrement(pVariable, pCut);

    // Apply shrinkage.
    float fShrinkage = mpConf->GetShrinkage();
    pCut->Shrink(fShrinkage);

    // Calculate gain.
    CalculateGain(pVariable, pCut);

    return true;
}

bool Logistic::CalculateOptimal(SampleSet* pSampleSet, Cut* pCut) const
{
    // Theory: increment = (K-1)/K * sigma{g:R}/sigma{|g| * (1-|g|): R}
    const int nClassCount = pSampleSet->GetClassCount();
    std::vector<double> vecGradSum(nClassCount, 0.0);
    std::vector<double> vecDiv(nClassCount, 0.0);

    for(SampleSet::Iterator iterSample = pSampleSet->Begin();
            iterSample != pSampleSet->End(); ++iterSample)
    {
        const float* pGrad = iterSample.GetGradient();
        for(int idxClass = 0; idxClass < nClassCount; ++idxClass)
        {
            float fGrad = pGrad[idxClass];
            float fGradAbs = std::fabs(fGrad);
            vecGradSum[idxClass] += fGrad;
            vecDiv[idxClass] += fGradAbs * (1.0 - fGradAbs);
        }
    }

    float fShrinkage = mpConf->GetShrinkage();
    float fFrac = static_cast<float>(nClassCount - 1) / static_cast<float>(nClassCount) * fShrinkage;
    float* pInc = pCut->GetIncrement();

    for(int idxClass = 0; idxClass < nClassCount; ++idxClass)
    {
        if(vecDiv[idxClass] == 0.0)
        {
            pInc[idxClass] = 0.0;
        }
        else
        {
            pInc[idxClass] = fFrac * vecGradSum[idxClass] / vecDiv[idxClass];
        }
    }
    return true;
}

bool Logistic::CalculateIncrement(SampleSet::Variable* pVariable, Cut* pCut) const
{
    // Theory: increment = (K-1)/K * sigma{g:R}/sigma{|g| * (1-|g|): R}
    const int nClassCount = pVariable->GetClassCount();
    std::vector<double> vecLeftGradSum(nClassCount, 0.0);
    std::vector<double> vecRightGradSum(nClassCount, 0.0);
    std::vector<double> vecLeftDiv(nClassCount, 0.0);
    std::vector<double> vecRightDiv(nClassCount, 0.0);
    bool bRealVar = pVariable->GetVariableType() == SampleMatrix::REAL;

    for(SampleSet::Variable::Iterator iterSample = pVariable->Begin();
            iterSample != pVariable->End(); ++iterSample)
    {
        bool bBranchLeft(false);
        if(bRealVar)
        {
            float fVar = iterSample.GetVariableDouble();
            bBranchLeft = pCut->BranchLeft(fVar);
        }
        else
        {
            int nVar = iterSample.GetVariableInteger();
            bBranchLeft = pCut->BranchLeft(nVar);
        }
        const float* pGrad = iterSample.GetGradient();
        if(bBranchLeft)
        {
            for(int idxClass = 0; idxClass < nClassCount; ++idxClass)
            {
                float fGrad = pGrad[idxClass];
                float fGradAbs = std::fabs(fGrad);
                vecLeftGradSum[idxClass] += fGrad;
                vecLeftDiv[idxClass] += fGradAbs * (1.0 - fGradAbs);
            }
        }
        else
        {
            for(int idxClass = 0; idxClass < nClassCount; ++idxClass)
            {
                float fGrad = pGrad[idxClass];
                float fGradAbs = std::fabs(fGrad);
                vecRightGradSum[idxClass] += fGrad;
                vecRightDiv[idxClass] += fGradAbs * (1.0 - fGradAbs);
            }
        }
    }

    float fFrac = static_cast<float>(nClassCount - 1) / static_cast<float>(nClassCount);
    float* pLeftInc(NULL);
    float* pRightInc(NULL);
    pCut->GetIncrement(pLeftInc, pRightInc);

    for(int idxClass = 0; idxClass < nClassCount; ++idxClass)
    {
        if(vecLeftDiv[idxClass] == 0.0)
        {
            pLeftInc[idxClass] = 0.0;
        }
        else
        {
            pLeftInc[idxClass] = fFrac * vecLeftGradSum[idxClass] / vecLeftDiv[idxClass];
        }
        if(vecRightDiv[idxClass] == 0.0)
        {
            pRightInc[idxClass] = 0.0;
        }
        else
        {
            pRightInc[idxClass] = fFrac * vecRightGradSum[idxClass] / vecRightDiv[idxClass];
        }
    }
    return true;
}

bool Logistic::CalculateGain(SampleSet::Variable* pVariable, Cut* pCut) const
{
    double fGain(0.0);
    int nClassCount = pVariable->GetClassCount();
    float* pNewEst = new float [nClassCount];
    float* pProb = new float [nClassCount];
    const float* pLeftInc(NULL);
    const float* pRightInc(NULL);
    const bool bRealVar = pVariable->GetVariableType() == SampleMatrix::REAL;
    const bool bWeighting = pVariable->GetWeighting();
    pCut->GetIncrement(pLeftInc, pRightInc);
    const double fExpLim = std::exp(-FLOAT_LOG_MAX);
    for(SampleSet::Variable::Iterator iterSample = pVariable->Begin();
            iterSample != pVariable->End(); ++iterSample)
    {
        int nTarget = static_cast<int>(iterSample.GetTarget());
        const float* pEst = iterSample.GetEstimation();
        std::memcpy(pNewEst, pEst, sizeof(float) * nClassCount);
        bool bBranchLeft;
        if(bRealVar)
        {
            float fVar = iterSample.GetVariableDouble();
            bBranchLeft = pCut->BranchLeft(fVar);
        }
        else
        {
            int nVar = iterSample.GetVariableInteger();
            bBranchLeft = pCut->BranchLeft(nVar);
        }
        if(bBranchLeft)
        {
            common::utility::VectorAdd(pNewEst, pLeftInc, nClassCount);
        }
        else
        {
            common::utility::VectorAdd(pNewEst, pRightInc, nClassCount);
        }
        common::utility::LogisticTransform(pEst, pProb, nClassCount);
        double fGainInc = (pProb[nTarget] <= fExpLim) ? FLOAT_LOG_MAX : -std::log(pProb[nTarget]);
        common::utility::LogisticTransform(pNewEst, pProb, nClassCount);
        fGainInc -= (pProb[nTarget] <= fExpLim) ? FLOAT_LOG_MAX : -std::log(pProb[nTarget]);
        if(bWeighting)
        {
            float fWeight = iterSample.GetWeight();
            fGainInc *= fWeight;
        }
        fGain += fGainInc;
    }
    if(pNewEst)
        delete [] pNewEst;
    if(pProb)
        delete [] pProb;

    pCut->SetGain(static_cast<float>(fGain));
    return true;
}

bool Logistic::UpdateSampleSet(SampleSet* pSampleSet, const float* pIncrement) const
{
    return Loss::UpdateSampleSet(pSampleSet, pIncrement);
}

bool Logistic::UpdateSampleMatrix(SampleMatrix* pSampleMatrix, const Tree* pTree) const
{
    return Loss::UpdateSampleMatrix(pSampleMatrix, pTree);
}

std::string Logistic::GetLossName() const
{
    return "logistic";
}


} // treelink
} // mlplus

