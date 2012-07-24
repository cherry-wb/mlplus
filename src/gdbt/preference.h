#ifndef MLLIB_TREE_LINK_PREFERENCE_H
#define MLLIB_TREE_LINK_PREFERENCE_H
#include <vector>
#include <utility>

namespace mlplus
{
namespace treelink
{

/**define the preference rank, from low to high.
 */
class RankList
{
public:
    RankList();
    ~RankList();
    /**build rank from scores.
     * @param scores a vector of index-score pairs used to build this rank-list. this parameter may
     * be modified after this process.
     * @return whether build rank succeed.
     */
    bool BuildRank(std::vector<std::pair<size_t, float> >& scores);

    /**get rank vector.
     * this process does not modify anything in this class.
     * @return the rank vector.
     */
    inline const std::vector<size_t>& GetRank() const;

    /**get jump edges.
     * this process does not modify anything in this class.
     * @return the vector of jump edges.
     */
    inline const std::vector<size_t>& GetJumpEdges() const;
private:
    std::vector<size_t> mRank;
    std::vector<size_t> mJumpEdges;
};

/**put all query-specific rank-list into PreferenceSet.
 */
class PreferenceSet
{
public:
    PreferenceSet();
    ~PreferenceSet();
    void Clear();
    void SetRanks(const std::vector<RankList*>& ranks);
    const RankList* GetRankList(size_t index) const;
    inline size_t GetSize() const;
private:
    std::vector<RankList*> mRanks;
};

/**definition of inline functions.
 */
inline const std::vector<size_t>& RankList::GetRank() const
{
    return mRank;
}

inline const std::vector<size_t>& RankList::GetJumpEdges() const
{
    return mJumpEdges;
}

inline size_t PreferenceSet::GetSize() const
{
    return mRanks.size();
}

} // treelink
} // mlplus

#endif // MLLIB_TREE_LINK_PREFERENCE_H

