//
// Tree declaration: as a base learner.
//

#ifndef MLPLUS_TREELINK_TREE_H_
#define MLPLUS_TREELINK_TREE_H_

#include <list>

#include "sample_set.h"

namespace mlplus
{
namespace treelink
{
class Node;
class Cut;
class Configuration;
class Loss;
class Tree
{
public:
    Tree();
    ~Tree();
    bool UpdateEstimation(const float* pX, float* pY) const;        // Give a prediction for boosting
    bool Grow(SampleSet* pSampleSet, const Configuration* pConf, const Loss* pLoss);    // Grow from sample set
    bool UpdateVariableImportance(std::vector<float>& vecVarImp);
    double CalculateLabelValue(const std::vector<float>& x) const;
    int GetNodeCount() const;
protected:
    const Node* GetRoot() const;
    void SetRoot(Node* pRoot);
    void SetNodeCount(int nNodeCount);
    void Clear();
private:
    void ClearAllNodes();
    void InsertCutToList(Cut* pCut, std::list<Cut*>& listCut);
    void ClearCutList(std::list<Cut*>& lstCut);
    void ClearNodeList(std::list<Node*>& lstNode);
    Node* mpRoot;
    int mnNodeCount;
    int mSampleCount;
    friend class ModelManager;
    friend class ParameterModelManager;
    friend class CFileModelManager;
}; // Tree

} // treelink
} // mlplus

#endif // MLLIB_TREELINK_TREE_H_

