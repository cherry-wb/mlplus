
#include <cmath>
#include "sample_matrix.h"
#include "sample_set.h"
#include "loss.h"
#include "ada_boost.h"
#include "cut.h"
#include "tree.h"
#include "configuration.h"

#define MAX_WEIGHT        4.0
#define MIN_WEIGHT        0.25

namespace mlplus
{
namespace treelink
{
AdaBoost::AdaBoost(const Configuration* pConf)
    : Loss(pConf)
{
}

AdaBoost::~AdaBoost()
{
}

// AdaBoost: initializer.
bool AdaBoost::InitEstimation(SampleMatrix* pSampleMatrix, float* pInitEst) const
{
    //
    // const-estimation = 0.5*  ln(sum-of-positive-labeled-weight/sum-of-negative-labeled-weight)
    // see the references for details.
    //
    double fPositiveWeight = 0.0;
    double fNegativeWeight = 0.0;
    bool bWeighting = pSampleMatrix->GetWeighting();
    for(int idxSample = 0; idxSample < pSampleMatrix->GetSampleCount(); ++idxSample)
    {
        float fTarget = pSampleMatrix->GetTarget(idxSample);
        float fWeight = bWeighting ? pSampleMatrix->GetWeight(idxSample) : 1.0;
        if(fTarget < 0)
            fNegativeWeight += fWeight;
        else
            fPositiveWeight += fWeight;
    }

    double fEstimation = 0.5 * Log(fPositiveWeight, fNegativeWeight);

    *pInitEst = fEstimation;

    // Update sample initial-estimations & weight.
    int nSampleCount = pSampleMatrix->GetSampleCount();
    for(int idxSample = 0; idxSample < nSampleCount; ++idxSample)
    {
        // Update initial-estimation.
        *pSampleMatrix->GetEstimation(idxSample) = fEstimation;
        // Update weight.
        // Estimation == Increment!
        float fTarget = pSampleMatrix->GetTarget(idxSample);
        float fWeight = bWeighting ? pSampleMatrix->GetWeight(idxSample) : 1.0;
        fWeight *= std::exp(-fTarget * fEstimation);
        pSampleMatrix->SetWeight(idxSample, fWeight);
    }

    return true;
}

float AdaBoost::CalculateLoss(SampleMatrix* pSampMat) const
{
    double fLoss(0.0);
    int nSampCount = pSampMat->GetSampleCount();
    for(int idxSamp = 0; idxSamp < nSampCount; ++idxSamp)
    {
        float fWeight = pSampMat->GetWeight(idxSamp);
        float fTarget = pSampMat->GetTarget(idxSamp);
        float fEst = *pSampMat->GetEstimation(idxSamp);
        fLoss += fWeight * std::exp(-fTarget * fEst);
    }
    return fLoss;
}

//
// y~ = wi * y * std::exp(-y)
bool AdaBoost::CalculateGradient(SampleSet* pSampleSet) const
{
    for(SampleSet::Iterator iterSample = pSampleSet->Begin();
            iterSample != pSampleSet->End(); ++iterSample)
    {
        float fWeight = iterSample.GetWeight();
        float fTarget = iterSample.GetTarget();
        float fGrad = fWeight * fTarget; // * std::exp(-y*0)
        *iterSample.GetGradient() = fGrad;
    }
    return true;
}

bool AdaBoost::CalculateOptimal(SampleSet* pSampleSet, Cut* pCut) const
{
    struct WeightSum
    {
        WeightSum(): mfPositive(0.0), mfNegative(0.0) {}
        ~WeightSum() {}
        double mfPositive;
        double mfNegative;
    };

    WeightSum weightSum;

    for(SampleSet::Iterator iterSample = pSampleSet->Begin();
            iterSample != pSampleSet->End(); ++iterSample)
    {
        float fWeight = iterSample.GetWeight();
        float fTarget = iterSample.GetTarget();
        if(fTarget >= 0.0)
            weightSum.mfPositive += fWeight;
        else
            weightSum.mfNegative += fWeight;
    }

    if(weightSum.mfPositive <= 0.0)
    {
        if(weightSum.mfNegative  <= 0.0)
            return false;
        else
            *pCut->GetIncrement() = -1.0;
    }
    else
    {
        if(weightSum.mfNegative  <= 0.0)
            *pCut->GetIncrement() = 1.0;
        else
            *pCut->GetIncrement() = 0.5 * Log(weightSum.mfPositive, weightSum.mfNegative);
    }
    float fShrinkage = mpConf->GetShrinkage();
    *pCut->GetIncrement() *= fShrinkage;
    return true;
}

bool AdaBoost::CalculateOptimal(SampleSet::Variable* pVariable, Cut* pCut) const
{
    // Best increment & save it to cut.
    float* pLeftInc = NULL;
    float* pRightInc = NULL;
    pCut->GetIncrement(pLeftInc, pRightInc);

    CalculateIncrement(pVariable, pCut);
    // Apply shrinkage.
    float fShrinkage = mpConf->GetShrinkage();
    pCut->Shrink(fShrinkage);

    // Calculate gain.
    return CalculateGain(pVariable, pCut);
}

bool AdaBoost::CalculateIncrement(SampleSet::Variable* pVariable, Cut* pCut) const
{
    // Calculate two best increment.
    struct WeightSum
    {
        WeightSum(): mfPositive(0.0), mfNegative(0.0) {}
        ~WeightSum() {}
        double mfPositive;
        double mfNegative;
    };

    // Weight sum.
    WeightSum leftWeightSum, rightWeightSum;
    if(pVariable->GetVariableType() == SampleMatrix::REAL)
    {
        // real variable process.
        for(SampleSet::Variable::Iterator iterVar = pVariable->Begin();
                iterVar != pVariable->End(); ++iterVar)
        {
            float fVarValue = iterVar.GetVariableDouble();
            float fWeight = iterVar.GetWeight();
            float fTarget = iterVar.GetTarget();
            if(pCut->BranchLeft(fVarValue))
            {
                // Add to left.
                if(fTarget >= 0.0)
                    leftWeightSum.mfPositive += fWeight;
                else
                    leftWeightSum.mfNegative += fWeight;
            }
            else
            {
                // Add to right.
                if(fTarget >= 0.0)
                    rightWeightSum.mfPositive += fWeight;
                else
                    rightWeightSum.mfNegative += fWeight;
            }
        }
    }
    else
    {
        // Nominal variable process.
        for(SampleSet::Variable::Iterator iterVar = pVariable->Begin();
                iterVar != pVariable->End(); ++iterVar)
        {
            int nVarValue = iterVar.GetVariableInteger();
            float fWeight = iterVar.GetWeight();
            float fTarget = iterVar.GetTarget();
            if(pCut->BranchLeft(nVarValue))
            {
                // Add to left.
                if(fTarget >= 0.0)
                    leftWeightSum.mfPositive += fWeight;
                else
                    leftWeightSum.mfNegative += fWeight;
            }
            else
            {
                // Add to right.
                if(fTarget >= 0.0)
                    rightWeightSum.mfPositive += fWeight;
                else
                    rightWeightSum.mfNegative += fWeight;
            }
        }
    }

    if(leftWeightSum.mfPositive <= 0.0)
    {
        if(leftWeightSum.mfNegative  <= 0.0)
            return false;
        else
            *pCut->GetLeftIncrement() = -1.0;
    }
    else
    {
        if(leftWeightSum.mfNegative  <= 0.0)
            *pCut->GetLeftIncrement() = 1.0;
        else
            *pCut->GetLeftIncrement() = 0.5 * Log(leftWeightSum.mfPositive, leftWeightSum.mfNegative);
    }

    if(rightWeightSum.mfPositive <= 0.0)
    {
        if(rightWeightSum.mfNegative  <= 0.0)
            return false;
        else
            *pCut->GetRightIncrement() = -1.0;
    }
    else
    {
        if(rightWeightSum.mfNegative  <= 0.0)
            *pCut->GetRightIncrement() = 1.0;
        else
            *pCut->GetRightIncrement() = 0.5 * Log(rightWeightSum.mfPositive, rightWeightSum.mfNegative);
    }

    return true;
}

bool AdaBoost::CalculateGain(SampleSet::Variable* pVariable, Cut* pCut) const
{
    double fGain = 0.0;
    float* pLeftInc = NULL;
    float* pRightInc = NULL;
    pCut->GetIncrement(pLeftInc, pRightInc);

    if(pVariable->GetVariableType() == SampleMatrix::REAL)
    {
        // real variable process.
        for(SampleSet::Variable::Iterator iterVar = pVariable->Begin();
                iterVar != pVariable->End(); ++iterVar)
        {
            float fVarValue = iterVar.GetVariableDouble();
            float fWeight = iterVar.GetWeight();
            float fTarget = iterVar.GetTarget();
            float fInc = pCut->BranchLeft(fVarValue) ? *pLeftInc : *pRightInc;
            fGain += fWeight * (1.0 - std::exp(-fTarget * fInc));
        }
    }
    else
    {
        // Nominal variable process.
        for(SampleSet::Variable::Iterator iterVar = pVariable->Begin();
                iterVar != pVariable->End(); ++iterVar)
        {
            int nVarValue = iterVar.GetVariableInteger();
            float fWeight = iterVar.GetWeight();
            float fTarget = iterVar.GetTarget();
            float fInc = pCut->BranchLeft(nVarValue) ? *pLeftInc : *pRightInc;
            fGain += fWeight * (1.0 - std::exp(-fTarget * fInc));
        }
    }

    pCut->SetGain(static_cast<float>(fGain));

    return true;
}

bool AdaBoost::UpdateSampleSet(SampleSet* pSampleSet, const float* pIncrement) const
{
    for(SampleSet::Iterator iterSample = pSampleSet->Begin(); iterSample != pSampleSet->End(); ++iterSample)
    {
        // Update Estimation
        *iterSample.GetEstimation() += *pIncrement;

        // Update weight.
        float fWeight = iterSample.GetWeight();
        float fTarget = iterSample.GetTarget();
        fWeight *= std::exp(-fTarget * *pIncrement);
        iterSample.SetWeight(fWeight);
    }
    return true;
}

bool AdaBoost::UpdateSampleMatrix(SampleMatrix* pSampleMatrix, const Tree* pTree) const
{
    int nSampleCount = pSampleMatrix->GetSampleCount();
    int nVariableCount = pSampleMatrix->GetVariableCount();
    float* pVariable = new float [nVariableCount];
    float fInc = 0.0;
    for(int idxSample = 0; idxSample < nSampleCount; ++idxSample)
    {
        // Load variables.
        if(!pSampleMatrix->GetVariableValue(idxSample, pVariable))
        {
            if(pVariable)
                delete [] pVariable;
            return false;
        }

        // Get increment.
        fInc = 0.0;
        if(!pTree->UpdateEstimation(pVariable, &fInc))
        {
            if(pVariable)
                delete [] pVariable;
            return false;
        }

        // Add the increment to estimation.
        float* pEstimation = pSampleMatrix->GetEstimation(idxSample);
        *pEstimation += fInc;

        // Update weight.
        float fWeight = pSampleMatrix->GetWeight(idxSample);
        float fTarget = pSampleMatrix->GetTarget(idxSample);
        fWeight *= std::exp(-fTarget * fInc);
        pSampleMatrix->SetWeight(idxSample, fWeight);
    }
    if(pVariable)
        delete [] pVariable;

    return true;
}

float AdaBoost::Log(float fPositive, float fNegative) const
{
    if(fNegative <= 0.0)
        return -2.0;
    if(fPositive <= 0.0)
        return 2.0;
    double r = fPositive / fNegative;
    if(r >= 7.38905609)
        return 2.0;
    if(r <= 0.13533528)
        return -2.0;
    return std::log(r);
}

std::string AdaBoost::GetLossName() const
{
    return "adaboost";
}

} // treelink
} // mlplus

