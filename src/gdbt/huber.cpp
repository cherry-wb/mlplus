#include <cmath>
#include "loss.h"
#include "huber.h"
#include "sample_matrix.h"
#include "sample_set.h"
#include "cut.h"
#include "mllib/common/utility.h"
#include "configuration.h"

namespace mllib
{
namespace treelink
{

Huber::Huber(const Configuration* pConf)
    : Loss(pConf), mfQuantile(0.0)
{
}

Huber::~Huber()
{
}

// Huber: initializer.
// init-estimation = median-w {target}
// Should share the same process with laplacian-loss.
bool Huber::InitEstimation(SampleMatrix* pSampleMatrix, float* pInitEst) const
{
    //
    // const-estimation = median-w of {target}.

    // Load target & weight into pair-vector
    int nSampleCount = pSampleMatrix->GetSampleCount();
    if(nSampleCount <= 0)
        return false;

    bool bWeighting = pSampleMatrix->GetWeighting();
    float fInitEstim = 0.0;
    if(!bWeighting)
    {
        // Don't use weighting.
        std::vector<double> vecTarget;
        vecTarget.reserve(nSampleCount);
        for(int idxSample = 0; idxSample < nSampleCount; ++idxSample)
        {
            float fTarget = pSampleMatrix->GetTarget(idxSample);
            vecTarget.push_back(fTarget);
        }
        fInitEstim = common::utility::CalculateMedian(vecTarget);
    }
    else
    {
        // Use weighting
        std::vector<std::pair<double, double> > vecTargetWeight;
        vecTargetWeight.reserve(nSampleCount);
        // Load target-weight pairs.
        for(int idxSample = 0; idxSample < nSampleCount; ++idxSample)
        {
            float fTarget = pSampleMatrix->GetTarget(idxSample);
            float fWeight = pSampleMatrix->GetWeight(idxSample);
            vecTargetWeight.push_back(std::make_pair(fTarget, fWeight));
        }
        fInitEstim = common::utility::CalculateWeightedMedian(vecTargetWeight);
    }

    // Update sample initial-estimations
    for(int idxSample = 0; idxSample < nSampleCount; ++idxSample)
    {
        // Update initial-estimation.
        *pSampleMatrix->GetEstimation(idxSample) = fInitEstim;
    }

    // output:
    *pInitEst = fInitEstim;

    return true;
}

float Huber::CalculateLoss(SampleMatrix* pSampMat) const
{
    //
    // loss = sum (weight * huber(y, y^))
    //
    bool bWeighting = pSampMat->GetWeighting();
    int nSampleCount = pSampMat->GetSampleCount();

    if(!bWeighting)
    {
        // Not using weighting.
        // Load residules
        std::vector<double> vecResidueAbs;
        vecResidueAbs.reserve(nSampleCount);
        for(int idxSamp = 0; idxSamp < nSampleCount; ++idxSamp)
        {
            float fResidue = pSampMat->GetTarget(idxSamp) - *pSampMat->GetEstimation(idxSamp);
            vecResidueAbs.push_back(fabs(fResidue));
        }

        // Calculate quantile.
        float fQuantile = common::utility::CalculateQuantile(vecResidueAbs, mpConf->GetQuantileRate());

        // Calculate loss.
        double fLoss = 0.0;
        for(std::vector<double>::iterator iterResAbs = vecResidueAbs.begin();
                iterResAbs != vecResidueAbs.end(); ++iterResAbs)
        {
            fLoss += *iterResAbs <= fQuantile ? *iterResAbs * *iterResAbs : fQuantile * (*iterResAbs - fQuantile / 2.0);
        }

        return fLoss;
    }
    else
    {
        // Using weighting.
        // Load residules
        std::vector<std::pair<double, double> > vecResAbsWeight;
        vecResAbsWeight.reserve(nSampleCount);
        for(int idxSamp = 0; idxSamp < nSampleCount; ++idxSamp)
        {
            float fResidue = pSampMat->GetTarget(idxSamp) - *pSampMat->GetEstimation(idxSamp);
            float fWeight = pSampMat->GetWeight(idxSamp);
            vecResAbsWeight.push_back(std::make_pair(fabs(fResidue), fWeight));
        }

        // Calculate quantile.
        float fQuantile = common::utility::CalculateWeightedQuantile(vecResAbsWeight,
                          mpConf->GetQuantileRate());

        // Calculate loss.
        double fLoss = 0.0;
        for(std::vector<std::pair<double, double> >::iterator iterResAbsWeight = vecResAbsWeight.begin();
                iterResAbsWeight != vecResAbsWeight.end(); ++iterResAbsWeight)
        {
            fLoss += ((*iterResAbsWeight).first <= fQuantile ?
                      (*iterResAbsWeight).first * (*iterResAbsWeight).first / 2.0
                      : fQuantile * ((*iterResAbsWeight).first - fQuantile / 2.0)) * (*iterResAbsWeight).second;
        }

        return fLoss;
    }
    return 0.0;
}

// Huber gradient.
//
bool Huber::CalculateGradient(SampleSet* pSampleSet) const
{
    bool bWeighting = pSampleSet->GetWeighting();
    // Calculate quantile.
    int nSampleCount = pSampleSet->GetSampleCount();
    // Collect residues into vectors.
    if(bWeighting)
    {
        std::vector<std::pair<double, double> > vecResidueAbsWeight;
        vecResidueAbsWeight.reserve(nSampleCount);
        for(SampleSet::Iterator iterSample = pSampleSet->Begin();
                iterSample != pSampleSet->End(); ++iterSample)
        {
            float fTarget = iterSample.GetTarget();
            float fEstimation = *iterSample.GetEstimation();
            float fResidue = fTarget - fEstimation;
            float fWeight = iterSample.GetWeight();
            vecResidueAbsWeight.push_back(std::make_pair(fabs(fResidue), fWeight));
        }
        // quantile.
        float fQuantile = common::utility::CalculateWeightedQuantile(vecResidueAbsWeight,
                          mpConf->GetQuantileRate());
        const_cast<float &>(mfQuantile) = fQuantile;
        // Update gradient.
        for(SampleSet::Iterator iterSample = pSampleSet->Begin();
                iterSample != pSampleSet->End(); ++iterSample)
        {
            float fTarget = iterSample.GetTarget();
            float fEstimation = *iterSample.GetEstimation();
            float fResidue = fTarget - fEstimation;
            float fWeight = iterSample.GetWeight();
            *iterSample.GetGradient() = fWeight * (fabs(fResidue) < fQuantile ?
                                                   fResidue : fQuantile * (fResidue < 0 ? -1.0 : 1.0));
        }
    }
    else
    {
        // Do not use weighting.
        std::vector<double> vecResidueAbs;
        vecResidueAbs.reserve(nSampleCount);
        for(SampleSet::Iterator iterSample = pSampleSet->Begin();
                iterSample != pSampleSet->End(); ++iterSample)
        {
            float fTarget = iterSample.GetTarget();
            float fEstimation = *iterSample.GetEstimation();
            float fResidue = fTarget - fEstimation;
            vecResidueAbs.push_back(fabs(fResidue));
        }

        // quantile.
        float fQuantile = common::utility::CalculateQuantile(vecResidueAbs, mpConf->GetQuantileRate());
        const_cast<float &>(mfQuantile) = fQuantile;

        // Update gradient.
        for(SampleSet::Iterator iterSample = pSampleSet->Begin();
                iterSample != pSampleSet->End(); ++iterSample)
        {
            float fTarget = iterSample.GetTarget();
            float fEstimation = *iterSample.GetEstimation();
            float fResidue = fTarget - fEstimation;
            *iterSample.GetGradient() = fabs(fResidue) < fQuantile ? fResidue : fQuantile * (fResidue < 0 ? -1.0 : 1.0);
        }
    }
    return true;
}

bool Huber::CalculateOptimal(SampleSet* pSampleSet, Cut* pCut) const
{
    bool bWeighting = pSampleSet->GetWeighting();
    int nSampleCount = pSampleSet->GetSampleCount();
    double fIncrement = 0.0;
    if(bWeighting)
    {
        // Calculate median.
        std::vector<std::pair<double, double> > vecResidueWeight;
        vecResidueWeight.reserve(nSampleCount);
        for(SampleSet::Iterator iterSample = pSampleSet->Begin(); iterSample != pSampleSet->End();
                ++iterSample)
        {
            float fWeight = iterSample.GetWeight();
            float fTarget = iterSample.GetTarget();
            float fEstimation = *iterSample.GetEstimation();
            vecResidueWeight.push_back(std::make_pair(fTarget - fEstimation, fWeight));
        }
        float fMedian = common::utility::CalculateWeightedMedian(vecResidueWeight);

        // Calculate Increment (correction). Left
        float fCorrection = 0.0;
        double fWeightedResidueSum = 0.0;
        double fWeightSum = 0.0;
        for(std::vector<std::pair<double, double> >::iterator iterResidueWeight = vecResidueWeight.begin();
                iterResidueWeight != vecResidueWeight.end(); ++iterResidueWeight)
        {
            float fResidue = (*iterResidueWeight).first;
            float fWeight = (*iterResidueWeight).second;
            float fSign = fResidue < 0 ? -1.0 : 1.0;
            float fAbs = fabs(fResidue);
            fWeightedResidueSum += fWeight * (fAbs <= mfQuantile ? fResidue : fSign * mfQuantile);
            fWeightSum += fWeight;
        }
        fCorrection = fWeightSum > 0 ? fWeightedResidueSum / fWeightSum : 0.0;
        fIncrement = fMedian + fCorrection;
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
            vecResidue.push_back(fTarget - fEstimation);
        }
        float fMedian = common::utility::CalculateMedian(vecResidue);

        // Calculate Increment (correction). Left
        float fCorrection = 0.0;
        double fResidueSum = 0.0;
        int nPartCount = 0;
        for(std::vector<double>::iterator iterResidue = vecResidue.begin();
                iterResidue != vecResidue.end(); ++iterResidue)
        {
            float fResidue = *iterResidue;
            float fSign = fResidue < 0 ? -1.0 : 1.0;
            float fAbs = fabs(fResidue);
            fResidueSum += fAbs <= mfQuantile ? fResidue : fSign * mfQuantile;
            ++nPartCount;
        }
        fCorrection = nPartCount > 0 ? fResidueSum / nPartCount : 0.0;
        fIncrement = fMedian + fCorrection;
    }

    *pCut->GetIncrement() = fIncrement * mpConf->GetShrinkage();
    return true;
}

// Huber optimal.
// Calculation of increment and gain can be merged here!
bool Huber::CalculateOptimal(SampleSet::Variable* pVariable, Cut* pCut) const
{
    // Best increment & save it to cut.
    float* pLeftInc = NULL;
    float* pRightInc = NULL;
    pCut->GetIncrement(pLeftInc, pRightInc);

    // Calculate best increment.
    // CalculateIncrement(*pLeftInc, *pRightInc);
    CalculateIncrement(pVariable, pCut, *pLeftInc, *pRightInc);

    // Apply shrinkage.
    float fShrinkage = mpConf->GetShrinkage();
    *pLeftInc *= fShrinkage;
    *pRightInc *= fShrinkage;

    // Calculate gain.
    CalculateGain(pVariable, pCut);

    return true;
}

bool Huber::CalculateIncrement(SampleSet::Variable* pVariable, Cut* pCut,
                               float &fLeftIncrement, float &fRightIncrement) const
{
    // Enumerate all samples in set, and calculate the best-increment.
    bool bWeighting = pVariable->GetWeighting();
    int nSampleCount = pVariable->GetSampleCount();
    if(pVariable->GetVariableType() == SampleMatrix::REAL)
    {
        // Real variable type.
        if(bWeighting)
        {
            // Calculate median.
            std::vector<std::pair<double, double> > vecLeftResidueWeight;
            vecLeftResidueWeight.reserve(nSampleCount);
            std::vector<std::pair<double, double> > vecRightResidueWeight;
            vecRightResidueWeight.reserve(nSampleCount);
            for(SampleSet::Variable::Iterator iterVar = pVariable->Begin(); iterVar != pVariable->End();
                    ++iterVar)
            {
                float fVar = iterVar.GetVariableDouble();
                float fWeight = iterVar.GetWeight();
                float fTarget = iterVar.GetTarget();
                float fEstimation = *iterVar.GetEstimation();
                if(pCut->BranchLeft(fVar))
                    vecLeftResidueWeight.push_back(std::make_pair(fTarget - fEstimation, fWeight));
                else
                    vecRightResidueWeight.push_back(std::make_pair(fTarget - fEstimation, fWeight));
            }
            float fLeftMedian = common::utility::CalculateWeightedMedian(vecLeftResidueWeight);
            float fRightMedian = common::utility::CalculateWeightedMedian(vecRightResidueWeight);

            // Calculate Increment (correction). Left
            float fCorrection = 0.0;
            double fWeightedResidueSum = 0.0;
            double fWeightSum = 0.0;
            for(std::vector<std::pair<double, double> >::iterator iterResidueWeight = vecLeftResidueWeight.begin();
                    iterResidueWeight != vecLeftResidueWeight.end(); ++iterResidueWeight)
            {
                float fResidue = (*iterResidueWeight).first;
                float fWeight = (*iterResidueWeight).second;
                float fSign = fResidue < 0 ? -1.0 : 1.0;
                float fAbs = fabs(fResidue);
                fWeightedResidueSum += fWeight * (fAbs <= mfQuantile ? fResidue : fSign * mfQuantile);
                fWeightSum += fWeight;
            }
            fCorrection = fWeightSum > 0 ? fWeightedResidueSum / fWeightSum : 0.0;
            fLeftIncrement = fLeftMedian + fCorrection;

            // Right correction.
            fCorrection = 0.0;
            fWeightedResidueSum = 0.0;
            fWeightSum = 0.0;
            for(std::vector<std::pair<double, double> >::iterator iterResidueWeight = vecRightResidueWeight.begin();
                    iterResidueWeight != vecRightResidueWeight.end(); ++iterResidueWeight)
            {
                float fResidue = (*iterResidueWeight).first;
                float fWeight = (*iterResidueWeight).second;
                float fSign = fResidue < 0 ? -1.0 : 1.0;
                float fAbs = fabs(fResidue);
                fWeightedResidueSum += fWeight * (fAbs <= mfQuantile ? fResidue : fSign * mfQuantile);
                fWeightSum += fWeight;
            }
            fCorrection = fWeightSum > 0 ? fWeightedResidueSum / fWeightSum : 0.0;
            fRightIncrement = fRightMedian + fCorrection;
        }
        else
        {
            // Calculate median.
            std::vector<double> vecLeftResidue;
            vecLeftResidue.reserve(nSampleCount);
            std::vector<double> vecRightResidue;
            vecRightResidue.reserve(nSampleCount);
            for(SampleSet::Variable::Iterator iterVar = pVariable->Begin(); iterVar != pVariable->End();
                    ++iterVar)
            {
                float fVar = iterVar.GetVariableDouble();
                float fTarget = iterVar.GetTarget();
                float fEstimation = *iterVar.GetEstimation();
                if(pCut->BranchLeft(fVar))
                    vecLeftResidue.push_back(fTarget - fEstimation);
                else
                    vecRightResidue.push_back(fTarget - fEstimation);
            }
            float fLeftMedian = common::utility::CalculateMedian(vecLeftResidue);
            float fRightMedian = common::utility::CalculateMedian(vecRightResidue);

            // Calculate Increment (correction). Left
            float fCorrection = 0.0;
            double fResidueSum = 0.0;
            int nPartCount = 0;
            for(std::vector<double>::iterator iterResidue = vecLeftResidue.begin();
                    iterResidue != vecLeftResidue.end(); ++iterResidue)
            {
                float fResidue = *iterResidue;
                float fSign = fResidue < 0 ? -1.0 : 1.0;
                float fAbs = fabs(fResidue);
                fResidueSum += fAbs <= mfQuantile ? fResidue : fSign * mfQuantile;
                ++nPartCount;
            }
            fCorrection = nPartCount > 0 ? fResidueSum / nPartCount : 0.0;
            fLeftIncrement = fLeftMedian + fCorrection;

            // Right correction.
            fCorrection = 0.0;
            fResidueSum = 0.0;
            nPartCount = 0;
            for(std::vector<double>::iterator iterResidue = vecRightResidue.begin();
                    iterResidue != vecRightResidue.end(); ++iterResidue)
            {
                float fResidue = *iterResidue;
                float fSign = fResidue < 0 ? -1.0 : 1.0;
                float fAbs = fabs(fResidue);
                fResidueSum += fAbs <= mfQuantile ? fResidue : fSign * mfQuantile;
                ++nPartCount;
            }
            fCorrection = nPartCount > 0 ? fResidueSum / nPartCount : 0.0;
            fRightIncrement = fRightMedian + fCorrection;
        }
    }
    else
    {
        // Nominal variable type.
        if(bWeighting)
        {
            // Calculate median.
            std::vector<std::pair<double, double> > vecLeftResidueWeight;
            vecLeftResidueWeight.reserve(nSampleCount);
            std::vector<std::pair<double, double> > vecRightResidueWeight;
            vecRightResidueWeight.reserve(nSampleCount);
            for(SampleSet::Variable::Iterator iterVar = pVariable->Begin(); iterVar != pVariable->End();
                    ++iterVar)
            {
                int nVar = iterVar.GetVariableInteger();
                float fWeight = iterVar.GetWeight();
                float fTarget = iterVar.GetTarget();
                float fEstimation = *iterVar.GetEstimation();
                if(pCut->BranchLeft(nVar))
                    vecLeftResidueWeight.push_back(std::make_pair(fTarget - fEstimation, fWeight));
                else
                    vecRightResidueWeight.push_back(std::make_pair(fTarget - fEstimation, fWeight));
            }
            float fLeftMedian = common::utility::CalculateWeightedMedian(vecLeftResidueWeight);
            float fRightMedian = common::utility::CalculateWeightedMedian(vecRightResidueWeight);

            // Calculate Increment (correction). Left
            float fCorrection = 0.0;
            double fWeightedResidueSum = 0.0;
            double fWeightSum = 0.0;
            for(std::vector<std::pair<double, double> >::iterator iterResidueWeight = vecLeftResidueWeight.begin();
                    iterResidueWeight != vecLeftResidueWeight.end(); ++iterResidueWeight)
            {
                float fResidue = (*iterResidueWeight).first;
                float fWeight = (*iterResidueWeight).second;
                float fSign = fResidue < 0 ? -1.0 : 1.0;
                float fAbs = fabs(fResidue);
                fWeightedResidueSum += fWeight * (fAbs <= mfQuantile ? fResidue : fSign * mfQuantile);
                fWeightSum += fWeight;
            }
            fCorrection = fWeightSum > 0 ? fWeightedResidueSum / fWeightSum : 0.0;
            fLeftIncrement = fLeftMedian + fCorrection;

            // Right correction.
            fCorrection = 0.0;
            fWeightedResidueSum = 0.0;
            fWeightSum = 0.0;
            for(std::vector<std::pair<double, double> >::iterator iterResidueWeight = vecRightResidueWeight.begin();
                    iterResidueWeight != vecRightResidueWeight.end(); ++iterResidueWeight)
            {
                float fResidue = (*iterResidueWeight).first;
                float fWeight = (*iterResidueWeight).second;
                float fSign = fResidue < 0 ? -1.0 : 1.0;
                float fAbs = fabs(fResidue);
                fWeightedResidueSum += fWeight * (fAbs <= mfQuantile ? fResidue : fSign * mfQuantile);
                fWeightSum += fWeight;
            }
            fCorrection = fWeightSum > 0 ? fWeightedResidueSum / fWeightSum : 0.0;
            fRightIncrement = fRightMedian + fCorrection;
        }
        else
        {
            // Calculate median.
            std::vector<double> vecLeftResidue;
            vecLeftResidue.reserve(nSampleCount);
            std::vector<double> vecRightResidue;
            vecRightResidue.reserve(nSampleCount);
            for(SampleSet::Variable::Iterator iterVar = pVariable->Begin(); iterVar != pVariable->End();
                    ++iterVar)
            {
                int nVar = iterVar.GetVariableInteger();
                float fTarget = iterVar.GetTarget();
                float fEstimation = *iterVar.GetEstimation();
                if(pCut->BranchLeft(nVar))
                    vecLeftResidue.push_back(fTarget - fEstimation);
                else
                    vecRightResidue.push_back(fTarget - fEstimation);
            }
            float fLeftMedian = common::utility::CalculateMedian(vecLeftResidue);
            float fRightMedian = common::utility::CalculateMedian(vecRightResidue);

            // Calculate Increment (correction). Left
            float fCorrection = 0.0;
            double fResidueSum = 0.0;
            int nPartCount = 0;
            for(std::vector<double>::iterator iterResidue = vecLeftResidue.begin();
                    iterResidue != vecLeftResidue.end(); ++iterResidue)
            {
                float fResidue = *iterResidue;
                float fSign = fResidue < 0 ? -1.0 : 1.0;
                float fAbs = fabs(fResidue);
                fResidueSum += fAbs <= mfQuantile ? fResidue : fSign * mfQuantile;
                ++nPartCount;
            }
            fCorrection = nPartCount > 0 ? fResidueSum / nPartCount : 0.0;
            fLeftIncrement = fLeftMedian + fCorrection;

            // Right correction.
            fCorrection = 0.0;
            fResidueSum = 0.0;
            nPartCount = 0;
            for(std::vector<double>::iterator iterResidue = vecRightResidue.begin();
                    iterResidue != vecRightResidue.end(); ++iterResidue)
            {
                float fResidue = *iterResidue;
                float fSign = fResidue < 0 ? -1.0 : 1.0;
                float fAbs = fabs(fResidue);
                fResidueSum += fAbs <= mfQuantile ? fResidue : fSign * mfQuantile;
                ++nPartCount;
            }
            fCorrection = nPartCount > 0 ? fResidueSum / nPartCount : 0.0;
            fRightIncrement = fRightMedian + fCorrection;
        }
    }
    return true;
}

bool Huber::CalculateGain(SampleSet::Variable* pVariable, Cut* pCut) const
{
    float fGain = 0.0;
    bool bWeighting = pVariable->GetWeighting();
    float fWeight = 1.0;
    double fWeightSum = 0.0;
    double fWeightedGainSum = 0.0;
    SampleMatrix::VariableType varType = pVariable->GetVariableType();
    for(SampleSet::Variable::Iterator iterVar = pVariable->Begin(); iterVar != pVariable->End();
            ++iterVar)
    {
        if(bWeighting)
            fWeight = iterVar.GetWeight();
        float fTarget = iterVar.GetTarget();
        float fEstimation = *iterVar.GetEstimation();
        float fResidue  = fTarget - fEstimation;
        bool bBranchLeft = true;
        float fNewResidue = 0.0;
        if(varType == SampleMatrix::REAL)
        {
            float fVar = iterVar.GetVariableDouble();
            bBranchLeft = pCut->BranchLeft(fVar);
        }
        else
        {
            int nVar = iterVar.GetVariableInteger();
            bBranchLeft = pCut->BranchLeft(nVar);
        }
        if(bBranchLeft)
            fNewResidue = fResidue - *pCut->GetLeftIncrement();
        else
            fNewResidue = fResidue - *pCut->GetRightIncrement();
        float fResAbs = fabs(fResidue);
        float fLoss = fResAbs <= mfQuantile ? 0.5 * fResAbs * fResAbs : mfQuantile * (fResAbs - mfQuantile / 2);
        float fNewResAbs = fabs(fNewResidue);
        float fNewLoss = fNewResAbs <= mfQuantile ? 0.5 * fNewResAbs * fNewResAbs : mfQuantile * (fNewResAbs - mfQuantile / 2);
        fWeightedGainSum += (fLoss - fNewLoss) * fWeight;
        fWeightSum += fWeight;
    }
    fGain = fWeightedGainSum;
    pCut->SetGain(fGain);
    return true;
}

// Update Sample set (Before each iteration of leaf-try-to-split).
bool Huber::UpdateSampleSet(SampleSet* pSampleSet, const float* pIncrement) const
{
    // Use default implementation is OK.
    return Loss::UpdateSampleSet(pSampleSet, pIncrement);
}

// Update Sample matrix (Before each iterator of tree-grow)
bool Huber::UpdateSampleMatrix(SampleMatrix* pSampleMatrix, const Tree* pTree) const
{
    // Use default implementation is OK.
    return Loss::UpdateSampleMatrix(pSampleMatrix, pTree);
}

std::string Huber::GetLossName() const
{
    return "huber";
}

} // treelink
} // mllib

