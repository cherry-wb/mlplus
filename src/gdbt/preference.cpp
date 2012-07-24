#include <functional>
#include <algorithm>
#include "mlplus/interface/log.h"
#include "preference.h"

namespace mlplus
{
namespace treelink
{
struct ScoreLess : public std::binary_function < std::pair<size_t, float>,
        std::pair<size_t, float>, bool >
{
    bool operator()(const std::pair<size_t, float>& x, const std::pair<size_t, float>& y)
    {
        return x.second < y.second;
    }
};

RankList::RankList()
    : mRank(), mJumpEdges()
{
}

RankList::~RankList()
{
}

bool RankList::BuildRank(std::vector<std::pair<size_t, float> >& scores)
{
    mRank.clear();
    mJumpEdges.clear();
    if(scores.empty())
    {
        TWARN("empty rank group.");
        return true;
    }
    std::sort(scores.begin(), scores.end(), ScoreLess());
    mRank.reserve(scores.size());
    mJumpEdges.reserve(scores.size());
    float score = scores.front().second;
    size_t index = 0;
    mJumpEdges.push_back(index);
    for(std::vector<std::pair<size_t, float> >::const_iterator iter = scores.begin();
            iter != scores.end(); ++iter, ++index)
    {
        mRank.push_back(iter->first);
        if(score < iter->second)
        {
            score = iter->second;
            mJumpEdges.push_back(index);
        }
    }
    mJumpEdges.push_back(index);
    return true;
}

PreferenceSet::PreferenceSet()
    : mRanks()
{
}

PreferenceSet::~PreferenceSet()
{
    Clear();
}

const RankList* PreferenceSet::GetRankList(size_t index) const
{
    if(index >= mRanks.size())
        return NULL;
    return mRanks[index];
}

void PreferenceSet::Clear()
{
    for(std::vector<RankList*>::const_iterator iter = mRanks.begin();
            iter != mRanks.end(); ++iter)
    {
        if(*iter)
            delete *iter;
    }
    mRanks.clear();
}

void PreferenceSet::SetRanks(const std::vector<RankList*>& ranks)
{
    Clear();
    mRanks = ranks;
}

} // treelink
} // mlplus

