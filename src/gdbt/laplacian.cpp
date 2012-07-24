#include <cmath>
#include <vector>
#include <utility>
#include <algorithm>
#include "loss.h"
#include "laplacian.h"
#include "sample_matrix.h"
#include "sample_set.h"
#include "cut.h"
#include "mlplus/common/utility.h"
#include "configuration.h"

namespace mlplus
{
namespace treelink
{

Laplacian::Laplacian(const Configuration* pConf)
    : Loss(pConf)
{
}

Laplacian::~Laplacian()
{
}

// Laplacian: initializer.
bool Laplacian::InitEstimation(SampleMatrix* pSampleMatrix, float* pInitEst) const
{
    //
    // const-estimation = median-w of {target}.
    //

    int nSampleCount = pSampleMatrix->GetSampleCount();
    bool bWeighting = pSampleMatrix->GetWeighting();

    if(nSampleCount <= 0)
    {
        return false;
    }

    float fInitEst(0.0);
    if(bWeighting)
    {
        std::vector<std::pair<double, double> > vecTargetWeight;
        vecTargetWeight.reserve(nSampleCount);
        for(int k = 0; k < nSampleCount; ++k)
        {
            vecTargetWeight.push_back(std::make_pair(pSampleMatrix->GetTarget(k),
                                      pSampleMatrix->GetWeight(k)));
        }
        fInitEst = common::utility::CalculateWeightedMedian(vecTargetWeight);
    }
    else
    {
        std::vector<double> vecTarget;
        vecTarget.reserve(nSampleCount);
        for(int k = 0; k < nSampleCount; ++k)
        {
            vecTarget.push_back(pSampleMatrix->GetTarget(k));
        }
        fInitEst = common::utility::CalculateMedian(vecTarget);
    }

    // Update sample initial-estimations
    for(int k = 0; k < nSampleCount; ++k)
    {
        // Update initial-estimation.
        * (pSampleMatrix->GetEstimation(k)) = fInitEst;
    }

    *pInitEst = fInitEst;

    return true;
}

float Laplacian::CalculateLoss(SampleMatrix* pSampMat) const
{
    double fLoss(0.0);
    bool bWeighting = pSampMat->GetWeighting();
    float fWeight(1.0);
    int nSampCount = pSampMat->GetSampleCount();

    for(int idxSamp = 0; idxSamp < nSampCount; ++idxSamp)
    {
        if(bWeighting)
            fWeight = pSampMat->GetWeight(idxSamp);
        float fTarget = pSampMat->GetTarget(idxSamp);
        float fEst = *pSampMat->GetEstimation(idxSamp);
        float fRes = fTarget - fEst;
        fLoss += fWeight * std::fabs(fRes);
    }
    return fLoss;
}

// Laplacian gradient.
// Principle: y~ = w * sign(y - y^)
bool Laplacian::CalculateGradient(SampleSet* pSampleSet) const
{
    bool bWeighting = pSampleSet->GetWeighting();
    float fWeight = 1.0;
    for(SampleSet::Iterator iterSample = pSampleSet->Begin(); iterSample != pSampleSet->End();
            ++iterSample)
    {
        if(bWeighting)
        {
            fWeight = iterSample.GetWeight();
        }
        float fTarget = iterSample.GetTarget();
        float fEstimation = *iterSample.GetEstimation();
        float fGrad = fWeight * (fTarget - fEstimation <= 0.0 ? -1.0 : 1.0);
        *iterSample.GetGradient() = fGrad;
    }
    return true;
}

bool Laplacian::CalculateOptimal(SampleSet* pSampleSet, Cut* pCut) const
{
    bool bWeighting = pSampleSet->GetWeighting();
    int nSampleCount = pSampleSet->GetSampleCount();
    float fIncrement(0.0);
    if(bWeighting)
    {
        std::vector<std::pair<double, double> > vecResidueWeight;
        vecResidueWeight.reserve(nSampleCount);

        for(SampleSet::Iterator iterSample = pSampleSet->Begin(); iterSample != pSampleSet->End();
                ++iterSample)
        {
            float fTarget = iterSample.GetTarget();
            float fEstimation = *iterSample.GetEstimation();
            float fWeight = iterSample.GetWeight();
            float fResidue(fTarget - fEstimation);
            vecResidueWeight.push_back(std::make_pair(fResidue, fWeight));
        }
        fIncrement = common::utility::CalculateWeightedMedian(vecResidueWeight);
    }
    else
    {
        std::vector<double> vecResidue;
        vecResidue.reserve(nSampleCount);
        for(SampleSet::Iterator iterSample = pSampleSet->Begin(); iterSample != pSampleSet->End();
                ++iterSample)
        {
            float fTarget = iterSample.GetTarget();
            float fEstimation = *iterSample.GetEstimation();
            float fResidue(fTarget - fEstimation);
            vecResidue.push_back(fResidue);
        }
        fIncrement = common::utility::CalculateMedian(vecResidue);
    }

    fIncrement *= mpConf->GetShrinkage();

    *pCut->GetIncrement() = fIncrement;

    return true;
}

// Laplacian optimal.
bool Laplacian::CalculateOptimal(SampleSet::Variable* pVariable, Cut* pCut) const
{
    // Best increment & save it to cut.
    float* pLeftInc = NULL;
    float* pRightInc = NULL;
    pCut->GetIncrement(pLeftInc, pRightInc);

    // Calculate best increment.
    //CalculateIncrement(pVariable, pCut);
    bool bWeighting = pVariable->GetWeighting();
    int nSampleCount = pVariable->GetSampleCount();
    bool bReal = pVariable->GetVariableType() == SampleMatrix::REAL;
    float fLeftIncrement(0.0), fRightIncrement(0.0);
    double fGain(0.0);
    if(bWeighting)
    {
        std::vector<std::pair<double, double> > vecLeftResidueWeight;
        vecLeftResidueWeight.reserve(nSampleCount);
        std::vector<std::pair<double, double> > vecRightResidueWeight;
        vecRightResidueWeight.reserve(nSampleCount);
        // Collect residue-weight pairs.
        for(SampleSet::Variable::Iterator iterSample = pVariable->Begin();
                iterSample != pVariable->End(); ++iterSample)
        {
            float fTarget = iterSample.GetTarget();
            float fEstimation = *iterSample.GetEstimation();
            float fWeight = iterSample.GetWeight();
            float fResidue(fTarget - fEstimation);
            bool bBranchLeft = false;
            if(bReal)
            {
                if(pCut->BranchLeft(iterSample.GetVariableDouble()))
                    bBranchLeft = true;
            }
            else
            {
                if(pCut->BranchLeft(iterSample.GetVariableInteger()))
                    bBranchLeft = true;
            }
            if(bBranchLeft)
                vecLeftResidueWeight.push_back(std::make_pair(fResidue, fWeight));
            else
                vecRightResidueWeight.push_back(std::make_pair(fResidue, fWeight));
        }
        fLeftIncrement = common::utility::CalculateWeightedMedian(vecLeftResidueWeight);
        fRightIncrement = common::utility::CalculateWeightedMedian(vecRightResidueWeight);

        // Apply shrinkage.
        float fShrinkage = mpConf->GetShrinkage();
        fLeftIncrement *= fShrinkage;
        fRightIncrement  *= fShrinkage;

        // Calculate gain.
        // Left part gain.
        for(std::vector<std::pair<double, double> >::iterator iterResWeight = vecLeftResidueWeight.begin();
                iterResWeight != vecLeftResidueWeight.end(); ++iterResWeight)
        {
            float fRes = (*iterResWeight).first;
            float fWeight = (*iterResWeight).second;
            fGain += fWeight * (std::fabs(fRes) - std::fabs(fRes - fLeftIncrement));
        }
        // Right part gain.
        for(std::vector<std::pair<double, double> >::iterator iterResWeight = vecRightResidueWeight.begin();
                iterResWeight != vecRightResidueWeight.end(); ++iterResWeight)
        {
            float fRes = (*iterResWeight).first;
            float fWeight = (*iterResWeight).second;
            fGain += fWeight * (std::fabs(fRes) - std::fabs(fRes - fRightIncrement));
        }
    }
    else
    {
        std::vector<double> vecLeftResidue;
        vecLeftResidue.reserve(nSampleCount);
        std::vector<double> vecRightResidue;
        vecRightResidue.reserve(nSampleCount);
        for(SampleSet::Variable::Iterator iterSample = pVariable->Begin();
                iterSample != pVariable->End(); ++iterSample)
        {
            float fTarget = iterSample.GetTarget();
            float fEstimation = *iterSample.GetEstimation();
            float fResidue(fTarget - fEstimation);
            bool bBranchLeft = false;
            if(bReal)
            {
                if(pCut->BranchLeft(iterSample.GetVariableDouble()))
                    bBranchLeft = true;
            }
            else
            {
                if(pCut->BranchLeft(iterSample.GetVariableInteger()))
                    bBranchLeft = true;
            }
            if(bBranchLeft)
                vecLeftResidue.push_back(fResidue);
            else
                vecRightResidue.push_back(fResidue);
        }
        fLeftIncrement = common::utility::CalculateMedian(vecLeftResidue);
        fRightIncrement = common::utility::CalculateMedian(vecRightResidue);

        // Apply shrinkage.
        float fShrinkage = mpConf->GetShrinkage();
        fLeftIncrement *= fShrinkage;
        fRightIncrement  *= fShrinkage;


        // Calculate gain.
        // Left part gain.
        for(std::vector<double>::iterator iterRes = vecLeftResidue.begin();
                iterRes != vecLeftResidue.end(); ++iterRes)
        {
            float fRes = (*iterRes);
            fGain += std::fabs(fRes) - std::fabs(fRes - fLeftIncrement);
        }
        // Right part gain.
        for(std::vector<double>::iterator iterRes = vecRightResidue.begin();
                iterRes != vecRightResidue.end(); ++iterRes)
        {
            float fRes = (*iterRes);
            fGain += std::fabs(fRes) - std::fabs(fRes - fRightIncrement);
        }
    }

    // Save both increments into pCut
    *pLeftInc = fLeftIncrement;
    *pRightInc = fRightIncrement;

    // Save gain into pCut
    pCut->SetGain(fGain);

    return true;
}

// Update Sample set (Before each iteration of leaf-try-to-split).
bool Laplacian::UpdateSampleSet(SampleSet* pSampleSet, const float* pIncrement) const
{
    // Use default implementation is OK.
    return Loss::UpdateSampleSet(pSampleSet, pIncrement);
}

// Update Sample matrix (Before each iterator of tree-grow)
bool Laplacian::UpdateSampleMatrix(SampleMatrix* pSampleMatrix, const Tree* pTree) const
{
    // Use default implementation is OK.
    return Loss::UpdateSampleMatrix(pSampleMatrix, pTree);
}

std::string Laplacian::GetLossName() const
{
    return "laplacian";
}

} // treelink
} // mlplus


