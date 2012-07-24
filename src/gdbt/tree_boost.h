#ifndef MLMPLUS_TREELINK_LINK_H_
#define MLPLUS_TREELINK_LINK_H_
#include <string>
#include <vector>
#include "loss.h"

namespace mlplus
{
namespace treelink
{
class DataSet;
class Tree;
class TreeBoost
{
public:
    TreeBoost();
    TreeBoost(int nTreeCount, int nClassCount);
    ~TreeBoost();
    bool predict(const std::vector<float>& x, float& y) const;        // predict from x
    bool predict(const std::vector<float>& x, std::vector<float>& y) const;        // predict from x
    bool spawnTrees(DataSet* pMatrix, const Configuration* pConf);    // Spawn trees from DataSet
    bool analyzeVariableImportance(std::vector<float>& vecVarImp);
    bool setUsedTreeCount(size_t nUsedTreeCount);
    double calculateLabelValue(const std::vector<float>& x) const;
protected:
    int getTreeCount() const;
    void setTreeCount(int nTreeCount);
    Tree* getTree(int nTreeIndex) const;
    void setTrees(Tree* pTrees, int nTreeCount);
    bool free();
    Loss::Type getType() const;
    void setType(Loss::Type dist);
    void setInitEstimation(const float* pInitEst, int nClassCount);
    int getClassCount() const;
    void setClassCount(int nClassCount);
    float* getInitEstimation() const;
    int getVariableCount() const;
    void setVariableCount(int nVariableCount);
private:
    std::vector<Tree*> mvecpTrees;
    size_t mnUsedTreeCount;
    Loss::Type mLossType;
    int mnClassCount;
    int mnVariableCount;
    float* mpInitEst;
    friend class ModelManager;
    friend class CFileModelManager;
    friend class ParameterModelManager;
}; // TreeBoost

} // namespace treelink
} // namespace mlplus

#endif // MLLIB_TREELINK_LINK_H_

