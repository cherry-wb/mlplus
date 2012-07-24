#include <cmath>
#include "loss.h"
#include "gaussian.h"
#include "sample_matrix.h"
#include "sample_set.h"
#include "cut.h"
#include "configuration.h"

namespace mlplus
{
namespace treelink
{

Gaussian::Gaussian(const Configuration* pConf)
    : Loss(pConf)
{
}

Gaussian::~Gaussian()
{
}

// Gaussian: initializer.
bool Gaussian::InitEstimation(SampleMatrix* pSampleMatrix, float* pInitEst) const
{
    //
    // const-estimation = mean-w of {target}.
    //
    double fWeightSum = 0.0;
    double fTargetSum = 0.0;
    bool bWeighting = pSampleMatrix->GetWeighting();
    for(int idxSample = 0; idxSample < pSampleMatrix->GetSampleCount(); ++idxSample)
    {
        float fTarget = pSampleMatrix->GetTarget(idxSample);
        float fWeight = bWeighting ? pSampleMatrix->GetWeight(idxSample) : 1.0;
        fTargetSum += fTarget * fWeight;
        fWeightSum += fWeight;
    }
    if(fWeightSum <= 0.0)
        return false;	// Invalid data.
    float fInitEst = fTargetSum / fWeightSum;

    // Update sample initial-estimations
    int nSampleCount = pSampleMatrix->GetSampleCount();
    for(int idxSample = 0; idxSample < nSampleCount; ++idxSample)
    {
        // Update initial-estimation.
        *pSampleMatrix->GetEstimation(idxSample) = fInitEst;
    }

    // output:
    *pInitEst = fInitEst;

    return true;
}

float Gaussian::CalculateLoss(SampleMatrix* pSampMat) const
{
    //
    // loss = sum (weight * (y-y^)^2)
    //
    double fLoss = 0.0;
    bool bWeighting = pSampMat->GetWeighting();
    int nSampCnt = pSampMat->GetSampleCount();
    for(int idxSamp = 0; idxSamp < nSampCnt; ++idxSamp)
    {
        if(bWeighting)
        {
            float fWeight = pSampMat->GetWeight(idxSamp);
            float fTarget = pSampMat->GetTarget(idxSamp);
            float fEstimation = *pSampMat->GetEstimation(idxSamp);
            float fResidue = fTarget - fEstimation;
            fLoss += fWeight * fResidue * fResidue;
        }
        else
        {
            float fTarget = pSampMat->GetTarget(idxSamp);
            float fEstimation = *pSampMat->GetEstimation(idxSamp);
            float fResidue = fTarget - fEstimation;
            fLoss += fResidue * fResidue;
        }
    }
    return fLoss;
}

// Gaussian gradient.
// Principle: y~ = w * (y - y^)
bool Gaussian::CalculateGradient(SampleSet* pSampleSet) const
{
    bool bWeighting = pSampleSet->GetWeighting();
    float fWeight = 1.0;
    for(SampleSet::Iterator iterSample = pSampleSet->Begin(); iterSample != pSampleSet->End(); ++iterSample)
    {
        if(bWeighting)
        {
            fWeight = iterSample.GetWeight();
        }
        float fTarget = iterSample.GetTarget();
        float fEstimation = *iterSample.GetEstimation();
        float fGrad = fWeight * (fTarget - fEstimation);
        *iterSample.GetGradient() = fGrad;
    }
    return true;
}

bool Gaussian::CalculateOptimal(SampleSet* pSampleSet, Cut* pCut) const
{
    double fWeightedResidueSum = 0.0;
    double fWeightSum = 0.0;

    // Accumulate residues.
    for(SampleSet::Iterator iterSamp = pSampleSet->Begin(); iterSamp != pSampleSet->End();
            ++iterSamp)
    {
        float fWeight = iterSamp.GetWeight();
        float fTarget = iterSamp.GetTarget();
        float fEstimation = *iterSamp.GetEstimation();
        fWeightedResidueSum += fWeight * (fTarget - fEstimation);
        fWeightSum += fWeight;
    }

    // Check if the data is invalid
    if(fWeightSum <= 0.0)
        return false;

    *pCut->GetIncrement() = mpConf->GetShrinkage() * fWeightedResidueSum / fWeightSum;

    return true;
}

// Gaussian optimal.
// Calculation of increment and gain can be merged here!
bool Gaussian::CalculateOptimal(SampleSet::Variable* pVariable, Cut* pCut) const
{
    // Best increment & save it to cut.
    float* pLeftInc = NULL;
    float* pRightInc = NULL;
    pCut->GetIncrement(pLeftInc, pRightInc);

    // Calculate best increment.
    // CalculateIncrement(*pLeftInc, *pRightInc);
    double fLeftWeightedResidueSum = 0.0;
    double fLeftWeightSum = 0.0;
    double fRightWeightedResidueSum = 0.0;
    double fRightWeightSum = 0.0;
    if(pVariable->GetVariableType() == SampleMatrix::REAL)
    {
        // real variable process.
        for(SampleSet::Variable::Iterator iterVar = pVariable->Begin();
                iterVar != pVariable->End(); ++iterVar)
        {
            float fVarValue = iterVar.GetVariableDouble();
            float fWeight = iterVar.GetWeight();
            float fTarget = iterVar.GetTarget();
            float fEstimation = *iterVar.GetEstimation();
            if(pCut->BranchLeft(fVarValue))
            {
                // Add to left.
                fLeftWeightedResidueSum += fWeight * (fTarget - fEstimation);
                fLeftWeightSum += fWeight;
            }
            else
            {
                // Add to right.
                fRightWeightedResidueSum += fWeight * (fTarget - fEstimation);
                fRightWeightSum += fWeight;
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
            float fEstimation = *iterVar.GetEstimation();
            if(pCut->BranchLeft(nVarValue))
            {
                // Add to left.
                fLeftWeightedResidueSum += fWeight * (fTarget - fEstimation);
                fLeftWeightSum += fWeight;
            }
            else
            {
                // Add to right.
                fRightWeightedResidueSum += fWeight * (fTarget - fEstimation);
                fRightWeightSum += fWeight;
            }
        }
    }

    if(fLeftWeightSum <= 0.0 || fRightWeightSum <= 0.0)
    {
        return false;
    }

    *pLeftInc = fLeftWeightedResidueSum / fLeftWeightSum;
    *pRightInc = fRightWeightedResidueSum / fRightWeightSum;

    // Apply shrinkage.
    float fShrinkage = mpConf->GetShrinkage();
    pCut->Shrink(fShrinkage);

    // Calculate gain.
    // CalculateGain(pVariable, pCut);
    // gain = 2 * increment * weighted-residue-sum - increment^2 * weight-sum.
    float fGain = 2.0 * *pLeftInc * fLeftWeightedResidueSum - *pLeftInc * *pLeftInc * fLeftWeightSum // Left gain part
                  + 2.0 * *pRightInc * fRightWeightedResidueSum - *pRightInc * *pRightInc * fRightWeightSum;	// Right gain part.
    pCut->SetGain(fGain);

    return true;
}

// Update Sample set (Before each iteration of leaf-try-to-split).
bool Gaussian::UpdateSampleSet(SampleSet* pSampleSet, const float* pIncrement) const
{
    // Use default implementation is OK.
    return Loss::UpdateSampleSet(pSampleSet, pIncrement);
}

// Update Sample matrix (Before each iterator of tree-grow)
bool Gaussian::UpdateSampleMatrix(SampleMatrix* pSampleMatrix, const Tree* pTree) const
{
    // Use default implementation is OK.
    return Loss::UpdateSampleMatrix(pSampleMatrix, pTree);
}

std::string Gaussian::GetLossName() const
{
    return "gaussian";
}

} // treelink
} // mlplus

