//
// Cut: information about split a node.
//
#ifndef MLLIB_TREELINK_CUT_H_
#define MLLIB_TREELINK_CUT_H_
#include <set>

#include "sample_matrix.h"

namespace mlplus
{
class TreeLink;
namespace treelink
{
class Node;
class Cut
{
public:
    enum DiscreteSeparatorType
    {
        UNKNOWN = 0,
        LEAVE_ONE_OUT,
        ARBITRARY
    };
public:
    Cut(int nClassCount, DiscreteSeparatorType discSepType = UNKNOWN);
    ~Cut();
    const Cut& operator = (const Cut& cut);
    void Shrink(float fShrinkage);
    void GetIncrement(float*& pLeftIncrement, float*& pRightIncrement);
    void GetIncrement(const float*& pLeftIncrement, const float*& pRightIncrement) const;
    bool BranchLeft(float fVarValue) const;
    bool BranchLeft(int nVarValue) const;
    void SetGain(float fGain);
    float GetGain() const;
    void SetDistance(float fDistance);
    float GetDistance() const;
    void SetVariableIndex(int nVariableIndex);
    void SetVariableType(SampleMatrix::VariableType varType);
    SampleMatrix::VariableType GetVariableType() const;
    bool SetSeparator(float fSeparator);
    bool SetSeparator(int nSeparator);
    bool SetSeparator(std::set<int>& setSeparator);
    float GetSeparatorReal() const;
    int GetSeparatorNominal() const;
    std::set<int>* GetSeparatorSet();
    int GetVariableIndex() const;
    void SetNode(Node* pNode);        // Relationship between cut & node.
    Node* GetNode();
    float* GetIncrement();
    int GetClassCount() const;
    float* GetLeftIncrement();
    float* GetRightIncrement();
    void CopyBestCut(Cut* pBestCut);
    DiscreteSeparatorType GetDiscreteSeparatorType() const;
    void SetDiscreteSeparatorType(DiscreteSeparatorType sepType);
    void ClearIncrementLeftRight();
    void ClearIncrementAll();
    bool UpdateVariableImportance(std::vector<float>& vecVarImp);
    void TakeDistanceAsGain();
protected:
    Node* mpNode;
    int mnClassCount;
    SampleMatrix::VariableType mvarType;
    int mnVariableIndex;
    float mfGain;
    float mfDistance;
    float* mpIncrement;
    float* mpLeftIncrement;
    float* mpRightIncrement;
    DiscreteSeparatorType msepType;
    union Separator
    {
        Separator();
        float real;
        int nominal;
        std::set<int>* bag;
    };
    Separator mseparator;
    friend class Tree;
}; // Cut
} // treelink
} // mlplus
#endif // MLLIB_TREELINK_CUT_H_

