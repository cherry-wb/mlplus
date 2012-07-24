#include <cmath>
#include <cstring>
#include <vector>
#include <utility>
#include <algorithm>
#include "mlplus/common/utility.h"
#include "mlplus/interface/log.h"
#include "loss.h"
#include "pairwise_ls.h"
#include "sample_matrix.h"
#include "sample_set.h"
#include "cut.h"
#include "configuration.h"

namespace mlplus
{
namespace treelink
{

bool PairwiseLs::InitSampleMatrix(SampleMatrix& sampMat) const
{
    return sampMat.BuildPreference(mpConf->IsSingleList());
}

PairwiseLs::PairwiseLs(const Configuration* pConf)
    : Loss(pConf), mfMargin(pConf->GetMargin())
{
}

PairwiseLs::~PairwiseLs()
{
}

// Calculate F0: intialization of treelink
bool PairwiseLs::InitEstimation(SampleMatrix* pSampleMatrix, float* pInitEst) const
{
    // Initialize all estimations with zeros.
    return Loss::InitEstimation(pSampleMatrix, pInitEst);
}

float PairwiseLs::CalculateLoss(SampleMatrix* pSampMat) const
{
    double fLoss(0.0);

    const float* estimation = pSampMat->GetAllEstimation();
    const PreferenceSet& prefSet = pSampMat->GetPreferenceSet();
    const size_t nRlCount = prefSet.GetSize();

    for(size_t idxRl = 0; idxRl < nRlCount; ++idxRl)
    {
        const RankList* rankList = prefSet.GetRankList(idxRl);
        const std::vector<size_t>& rank = rankList->GetRank();
        const std::vector<size_t>& jumpEdge = rankList->GetJumpEdges();
        for(size_t idxJump = 0; idxJump + 1 < jumpEdge.size(); ++idxJump)
        {
            for(size_t idxRank1 = jumpEdge[idxJump]; idxRank1 < jumpEdge[idxJump + 1]; ++idxRank1)
            {
                const size_t index1 = rank[idxRank1];
                for(size_t idxRank2 = jumpEdge[idxJump + 1]; idxRank2 < rank.size(); ++idxRank2)
                {
                    const size_t index2 = rank[idxRank2];
                    // rank[idxRank1] \prev rank[idxRank2]
                    fLoss += common::utility::Square(
                                 std::max(0.0, estimation[index1] - estimation[index2] + mfMargin));
                }
            }
        }
    }

    return fLoss;
}

bool PairwiseLs::CalculateGradient(SampleSet* pSampleSet) const
{
    // Theory: gk = yk - pk
    SampleMatrix* pSampMat = pSampleSet->GetSampleMatrix();
    const int nSampCount = pSampMat->GetSampleCount();
    const float* estimation = pSampMat->GetAllEstimation();
    const bool* isSelected = pSampleSet->GetSelect();
    const PreferenceSet& prefSet = pSampMat->GetPreferenceSet();
    const size_t nRlCount = prefSet.GetSize();

    float* grad = pSampleSet->GetAllGradient();

    for(int sampIndex = 0; sampIndex < nSampCount; ++sampIndex)
    {
        if(isSelected[sampIndex])
            grad[sampIndex] = 0;
    }

    for(size_t idxRl = 0; idxRl < nRlCount; ++idxRl)
    {
        const RankList* rankList = prefSet.GetRankList(idxRl);
        const std::vector<size_t>& rank = rankList->GetRank();
        const std::vector<size_t>& jumpEdge = rankList->GetJumpEdges();

        for(size_t idxJump = 0; idxJump + 1 < jumpEdge.size(); ++idxJump)
        {
            for(size_t idxRank1 = jumpEdge[idxJump]; idxRank1 < jumpEdge[idxJump + 1]; ++idxRank1)
            {
                const size_t index1 = rank[idxRank1];
                for(size_t idxRank2 = jumpEdge[idxJump + 1]; idxRank2 < rank.size(); ++idxRank2)
                {
                    const size_t index2 = rank[idxRank2];
                    // rank[idxRank1] \prev rank[idxRank2]
                    if(isSelected[index1] || isSelected[index2])
                    {
                        double fGrad = std::max(0.0, estimation[index1] - estimation[index2] + mfMargin);
                        if(isSelected[index1])   // if index1 is selected in sample
                        {
                            grad[index1] -= fGrad;
                        }
                        if(isSelected[index2])
                        {
                            grad[index2] += fGrad;
                        }
                    }
                }
            }
        }
    }
    return true;
}

bool PairwiseLs::CalculateOptimal(SampleSet::Variable* pVariable, Cut* pCut) const
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
            float fGrad = *iterVar.GetGradient();
            if(pCut->BranchLeft(fVarValue))
            {
                // Add to left.
                fLeftWeightedResidueSum += fGrad;
                fLeftWeightSum += fWeight;
            }
            else
            {
                // Add to right.
                fRightWeightedResidueSum += fGrad;
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
            float fGrad = *iterVar.GetGradient();
//			float fTarget = iterVar.GetTarget();
//			float fEstimation = *iterVar.GetEstimation();
            if(pCut->BranchLeft(nVarValue))
            {
                // Add to left.
                fLeftWeightedResidueSum += fGrad;
                fLeftWeightSum += fWeight;
            }
            else
            {
                // Add to right.
                fRightWeightedResidueSum += fGrad;
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

    CalculateGain(pVariable, pCut);
    return true;
}

bool PairwiseLs::CalculateOptimal(SampleSet* pSampleSet, Cut* pCut) const
{
    double fWeightedResidueSum = 0.0;
    double fWeightSum = 0.0;

    // Accumulate residues.
    for(SampleSet::Iterator iterSamp = pSampleSet->Begin(); iterSamp != pSampleSet->End();
            ++iterSamp)
    {
        float fWeight = iterSamp.GetWeight();
        float fGrad = *iterSamp.GetGradient();
        fWeightedResidueSum += fGrad;
        fWeightSum += fWeight;
    }

    // Check if the data is invalid
    if(fWeightSum <= 0.0)
    {
        TWARN("pairwise_ls calculate optimal: weight sum is zero.");
        fWeightSum = 1.0;
    }

    *pCut->GetIncrement() = mpConf->GetShrinkage() * fWeightedResidueSum / fWeightSum;
    return true;
}

bool PairwiseLs::UpdateSampleSet(SampleSet* pSampleSet, const float* pIncrement) const
{
    return Loss::UpdateSampleSet(pSampleSet, pIncrement);
}

bool PairwiseLs::UpdateSampleMatrix(SampleMatrix* pSampleMatrix, const Tree* pTree) const
{
    return Loss::UpdateSampleMatrix(pSampleMatrix, pTree);
}

std::string PairwiseLs::GetLossName() const
{
    return "pairwiseLs";
}

bool PairwiseLs::CalculateGain(SampleSet::Variable* pVariable, Cut* pCut) const
{
    SampleSet* pSampleSet = pVariable->GetSampleSet();
    SampleMatrix* pSampMat = pSampleSet->GetSampleMatrix();
    const bool* isSelected = pSampleSet->GetSelect();

    const float* pLeftInc = NULL;
    const float* pRightInc = NULL;
    pCut->GetIncrement(pLeftInc, pRightInc);

    const int nSampCount = pSampMat->GetSampleCount();
    const float *currEstimation = pSampMat->GetAllEstimation();
    std::vector<float> newEstimation(nSampCount);
    std::copy(currEstimation, currEstimation + nSampCount, newEstimation.begin());

    const bool bRealVar = pVariable->GetVariableType() == SampleMatrix::REAL;
    for(SampleSet::Variable::Iterator iterSample = pVariable->Begin();
            iterSample != pVariable->End(); ++iterSample)
    {
        bool bBranchLeft = false;
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
        size_t idxSamp = iterSample.GetIndex();
        newEstimation[idxSamp] += (bBranchLeft ? (*pLeftInc) : (*pRightInc));
    }

    const PreferenceSet& prefSet = pSampMat->GetPreferenceSet();
    const size_t nRlCount = prefSet.GetSize();

    double fGain = 0.0;
    for(size_t idxRl = 0; idxRl < nRlCount; ++idxRl)
    {
        const RankList* rankList = prefSet.GetRankList(idxRl);
        const std::vector<size_t>& rank = rankList->GetRank();
        const std::vector<size_t>& jumpEdge = rankList->GetJumpEdges();
        for(size_t idxJump = 0; idxJump + 1 < jumpEdge.size(); ++idxJump)
        {
            for(size_t idxRank1 = jumpEdge[idxJump]; idxRank1 < jumpEdge[idxJump + 1]; ++idxRank1)
            {
                const size_t index1 = rank[idxRank1];
                if(isSelected[index1])
                {
                    for(size_t idxRank2 = 0; idxRank2 < jumpEdge[idxJump]; ++idxRank2)
                    {
                        const size_t index2 = rank[idxRank2];
                        if(!isSelected[index2])
                        {
                            // rank[idxRank1] \succ rank[idxRank2]
                            fGain += common::utility::Square(
                                         std::max(0.0, currEstimation[index2] - currEstimation[index1] + mfMargin));
                            fGain -= common::utility::Square(
                                         std::max(0.0, newEstimation[index2] - newEstimation[index1] + mfMargin));
                        }
                    }
                    for(size_t idxRank2 = jumpEdge[idxJump + 1]; idxRank2 < rank.size(); ++idxRank2)
                    {
                        const size_t index2 = rank[idxRank2];
                        // rank[idxRank1] \prev rank[idxRank2]
                        fGain += common::utility::Square(
                                     std::max(0.0, currEstimation[index1] - currEstimation[index2] + mfMargin));
                        fGain -= common::utility::Square(
                                     std::max(0.0, newEstimation[index1] - newEstimation[index2] + mfMargin));
                    }
                }
            }
        }
    }
    pCut->SetGain(static_cast<float>(fGain));
    return true;
}

} // treelink
} // mlplus

