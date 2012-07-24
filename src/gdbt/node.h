//
// tree node of TreeLink's trees.
//
#ifndef MLLIB_TREELINK_NODE_H_
#define MLLIB_TREELINK_NODE_H_

#include "sample_set.h"

namespace mlplus
{
namespace treelink
{
class SampleSet;
class Cut;
class Configuration;
class Node
{
public:
    Node(SampleSet* pSampleSet, const Configuration* pConf, const Loss* pLoss, int nNodeId);
    Node(Cut* pCut, int nNodeId);
    ~Node();
    float* GetIncrement();                // Get the increment of the node.
    Node* Climb(const float* pX) const;        // Climb the node to its proper child.
    bool IsLeaf() const;
    Cut* CalculateBestCut();            // Get the best cut of the node, need to be deleted outside after used.
    bool Split(Node*& pLeftChild, Node*& pRightChild);
    void SetSampleSet(SampleSet* pSampleSet);
    SampleSet* GetSampleSet();
    void SetCut(Cut* pCut);
    Cut* GetCut() const;
    bool UpdateEstimation(float* pEstimation) const;
    int GetClassCount() const;
    void ClearSampleSet();
    int GetNodeId() const;
    bool UpdateVariableImportance(std::vector<float>& vecVarImp);
    void TransmitIncrementToChildren();
    Node* GetParent();
    double GetLabelValue() const;
protected:
    Node* GetLeftChild() const;
    Node* GetRightChild() const;
    void SetLeftChild(Node* pLeftChild);
    void SetRightChild(Node* pRightChild);
private:
    void UpdateIncrement(const float* pFatherIncrement, const float* pBranchIncrement, int nClassCount);
    bool UpdateSampleSet();
    SampleSet* mpSampleSet;
    Cut* mpCut;
    const Configuration* mpConf;
    Node* mpLeftChild;
    Node* mpRightChild;
    Node* mpParent;
    const Loss* mpLoss;
    int mnNodeId;
    size_t mSampleCount;
    friend class ModelManager;
    friend class ParameterModelManager;
    friend class CFileModelManager;
    friend class Tree;
}; // Node
} // treelink
} // mlplus
#endif // MLLIB_TREELINK_NODE_H_

