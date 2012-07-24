#include <cstring>
#include <ctime>
#include <cstdlib>
#include <utility>
#include <algorithm>
#include <vector>

#include "mlplus/mlplus.h"
#include "mlplus/interface/log.h"
#include "sample_matrix.h"
#include "sample_set.h"
#include "sample_set_manager.h"

namespace mlplus
{
namespace treelink
{

SampleMatrix::SampleMatrix(const SampleContainer* pContainer)
    : mpContainer(pContainer), mppEstimation(NULL), mpEstimation(NULL),
      mpWeight(NULL), mppVariableOrder(NULL),
      mpVariableOrder(NULL), mnSampleCount(0), mnClassCount(0), mnVariableCount(0),
      mbWeighting(false), mPreferenceSet(), mGroupCount(0)
{
    mnSampleCount = mpContainer->Size();
    mGroupCount = mpContainer->GetGroupCount();
    if(mnSampleCount <= 0)
        return;
    mnVariableCount = mpContainer->GetFeatureSize();
    mnClassCount = mpContainer->GetTargetSize();
    mbWeighting = mpContainer->IsWeighted();
    mpEstimation = new float [mnClassCount * mnSampleCount];
    mppEstimation = new float *[mnSampleCount];
    mpWeight = new float [mnSampleCount];
    mppVariableOrder = new int *[mnVariableCount];
    mpVariableOrder = new int [mnSampleCount * mnVariableCount];
}

SampleMatrix::~SampleMatrix()
{
    if(mpEstimation)
        delete [] mpEstimation;
    if(mppEstimation)
        delete [] mppEstimation;
    if(mpWeight)
        delete [] mpWeight;
    if(mpVariableOrder)
        delete [] mpVariableOrder;
    if(mppVariableOrder)
        delete [] mppVariableOrder;
}

bool SampleMatrix::Initialize()
{
    float* pEstimation = mpEstimation;
    std::memset(mpEstimation, 0, sizeof(float) * mnClassCount * mnSampleCount);
    for(int idxSample = 0; idxSample < mnSampleCount; ++idxSample)
    {
        // Build estimation 2-d array
        mppEstimation[idxSample] = pEstimation;
        pEstimation += mnClassCount;
        // Load weights.
        const MlSample* pSample = mpContainer->GetSample(idxSample);
        mpWeight[idxSample] = pSample->GetWeight();
    }

    int* pVariableOrder = mpVariableOrder;
    for(int idxVar = 0; idxVar < mnVariableCount; ++idxVar)
    {
        // Build variable order 2-d array.
        mppVariableOrder[idxVar] = pVariableOrder;
        pVariableOrder += mnSampleCount;
    }

    // Build variable order index.
    for(int idxVar = 0; idxVar < mnVariableCount; ++idxVar)
    {
        if(!BuildVariableOrder(idxVar))
            return false;
    }

    return true;
}

bool SampleMatrix::BuildPreference(bool bSingleList)
{
    if(mnClassCount != 1 || mGroupCount == 0)
        return false;
    typedef std::vector<std::vector<std::pair<size_t, float> > > TargetMatrix;
    TargetMatrix targetMatrix;
    targetMatrix.resize(mGroupCount);
    for(int index = 0; index < mnSampleCount; ++index)
    {
        const MlSample* pSample = mpContainer->GetSample(index);
        float target = pSample->GetTargetValue();
        size_t group = pSample->GetGroupId();
        if(bSingleList)
        {
            group = 0;
        }
        else
        {
            if(group >= mGroupCount)
                return false;
        }
        targetMatrix[group].push_back(std::make_pair(index, target));
    }

    std::vector<RankList*> ranks(mGroupCount);
    for(size_t group = 0; group < mGroupCount; ++group)
    {
        RankList* pRankList = new RankList();
        ranks[group] = pRankList;
        if(!pRankList->BuildRank(targetMatrix[group]))
        {
            for(size_t i = 0; i <= group; ++i)
            {
                if(ranks[i])
                    delete ranks[i];
            }
            return false;
        }
        const std::vector<size_t>& rank = pRankList->GetRank();
        const std::vector<size_t>& edges = pRankList->GetJumpEdges();
        for(size_t idxEdge = 0; idxEdge + 1 < edges.size(); ++idxEdge)
        {
            size_t nBegin = edges[idxEdge];
            size_t nEnd = edges[idxEdge + 1];
            double fWeight = rank.size() - (nEnd - nBegin);
            for(size_t idxRank = nBegin; idxRank < nEnd; ++idxRank)
            {
                int idxSamp = rank[idxRank];
                MlSample* pSamp = mpContainer->GetSample(idxSamp);
                if(pSamp)
                {
                    pSamp->SetWeight(fWeight);
                    mpWeight[idxSamp] = fWeight;
                }
            }
        }
    }

    mPreferenceSet.SetRanks(ranks);
    return true;
}

const PreferenceSet& SampleMatrix::GetPreferenceSet() const
{
    return mPreferenceSet;
}

SampleSet* SampleMatrix::StochasticSubsample(SampleSetManager* pManager, float fSampleRate)
{
    SampleSet* pSampleSet = pManager->TakeSampleSet(this); // new SampleSet(this);
    bool* pSelect = pSampleSet->GetSelect();
    int nSubsampleCount = 0;
    for(int idxSample = 0; idxSample < mnSampleCount; ++idxSample)
    {
        double fRand = static_cast<double>(std::rand()) / RAND_MAX;
        if(fRand <= fSampleRate)
        {
            *pSelect = true;        // Select this sample.

            ++nSubsampleCount;
        }
        else
        {
            *pSelect = false;        // Don't select this sample.
        }
        ++pSelect;
    }

    pSampleSet->SetSampleCount(nSubsampleCount);
    return pSampleSet;
}

int SampleMatrix::GetSampleCount() const
{
    return mnSampleCount;
}

int SampleMatrix::GetClassCount() const
{
    return mnClassCount;
}

int SampleMatrix::GetVariableCount() const
{
    return mnVariableCount;
}

SampleMatrix::VariableType SampleMatrix::GetVariableType(int nVariableIndex) const
{
    mlplus::TagType tagType = mpContainer->GetFeatureType(nVariableIndex);
    switch(tagType)
    {
    case mlplus::UNKNOWN:
        return UNKNOWN;
    case mlplus::NUMBER:
        return REAL;
    case mlplus::NOMINAL:
    case mlplus::STRING:
        return NOMINAL;
    default:
        return UNKNOWN;
    }
}

float* SampleMatrix::GetEstimation(int nSampleIndex)
{
    return mppEstimation[nSampleIndex];
}

float SampleMatrix::GetTarget(int nSampleIndex) const
{
    const Sample* pSample = mpContainer->GetSample(nSampleIndex);
    return pSample->GetTargetValue();
}

float SampleMatrix::GetWeight(int nSampleIndex) const
{
    return mpWeight[nSampleIndex];
}

void SampleMatrix::SetWeight(int nSampleIndex, float fWeight)
{
    mpWeight[nSampleIndex] = fWeight;
}

const int* SampleMatrix::GetVariableOrder(int nVariableIndex) const
{
    return mppVariableOrder[nVariableIndex];
}

float SampleMatrix::GetVariableValue(int nSampleIndex, int nVariableIndex)
{
    const Sample* pSample = mpContainer->GetSample(nSampleIndex);
    float fVarValue = 0.0;
    if(pSample->GetFeatureValue(nVariableIndex, fVarValue))
    {
        return fVarValue;
    }
    else
    {
        return 0.0;
    }
}

void SampleMatrix::SetWeighting(bool bWeighting)
{
    mbWeighting = bWeighting;
}

bool SampleMatrix::GetWeighting() const
{
    return mbWeighting;
}

bool SampleMatrix::BuildVariableOrder(int nVariableIndex)
{
    // Construct vector & collect the variable-(value, index) pairs.
    std::vector<std::pair<float, int> > vecVarIdxPair;
    vecVarIdxPair.reserve(mnSampleCount);
    for(int idxSample = 0; idxSample < mnSampleCount; ++idxSample)
    {
        vecVarIdxPair.push_back(std::make_pair(GetVariableValue(idxSample, nVariableIndex), idxSample));
    }

    // Sort the variable-(value, index) pairs using built in operator "<"
    std::sort(vecVarIdxPair.begin(), vecVarIdxPair.end());

    int* pVarOrder = mppVariableOrder[nVariableIndex];
    for(std::vector<std::pair<float, int> >::iterator iterVarIdxPair = vecVarIdxPair.begin();
            iterVarIdxPair != vecVarIdxPair.end(); ++iterVarIdxPair)
    {
        *pVarOrder++ = (*iterVarIdxPair).second;
    }

    return true;
}

bool SampleMatrix::GetVariableValue(int nSampleIndex, float* pVariable) const
{
    const Sample* pSample = mpContainer->GetSample(nSampleIndex);
    if(!pSample)
        return false;

    if(!pSample->IsSparse())
    {
        for(int idxVar = 0; idxVar < mnVariableCount; ++idxVar)
        {
            float fVarValue = 0.0;
            pSample->GetFeatureValue(idxVar, fVarValue);
            *pVariable++ = fVarValue;
        }
    }
    else
    {
        const std::pair<int, float>* pFeat = NULL;
        pSample->GetFeatureValues(pFeat);
        int nFeat = pSample->GetFeatureSize();
        std::memset(pVariable, 0, sizeof(float) * mnVariableCount);
        while(nFeat-- > 0)
        {
            pVariable[pFeat->first] = pFeat->second;
            ++pFeat;
        }
    }
    return true;
}

const float* SampleMatrix::GetAllEstimation() const
{
    return mpEstimation;
}

const float* SampleMatrix::GetAllWeight() const
{
    return mpWeight;
}

float SampleMatrix::GetOriginalWeight(int nSampleIndex) const
{
    return mpContainer->GetSample(nSampleIndex)->GetWeight();
}

// Check whether it is two class regression.
bool SampleMatrix::IsTwoClassRegression() const
{
    if(mnClassCount != 1)
        return false;
    for(int idxSamp = 0; idxSamp < mnSampleCount; ++idxSamp)
    {
        int nTarget = static_cast<int>(GetTarget(idxSamp));
        if(nTarget != 1 && nTarget != -1)
            return false;
    }
    return true;
}

} // treelink
} // mlplus

