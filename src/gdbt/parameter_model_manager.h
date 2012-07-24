#ifndef MLLIB_TREELINK_PARAMETER_MODEL_MANAGER_H_
#define MLLIB_TREELINK_PARAMETER_MODEL_MANAGER_H_
#include <string>
#include <iostream>
#include "model_manager.h"

namespace mlplus
{
namespace common
{
class SerialIn;
class SerialOut;
}
namespace treelink
{
class TreeBoost;
class ParameterModelManager : public ModelManager
{
public:
    ParameterModelManager();
    ~ParameterModelManager();
    bool Load(TreeBoost*& pTreeBoost, std::istream& is);
    bool Store(const TreeBoost* pTreeBoost, std::ostream& os);
private:
    bool LoadTree(Tree* pTree, mlplus::common::SerialIn& sin, int nClassCount) const;
    bool LoadNode(Node* pNode, mlplus::common::SerialIn& sin) const;
    bool StoreTree(const Tree* pTree, mlplus::common::SerialOut& sout) const;
    bool StoreNode(const Node* pNode, mlplus::common::SerialOut& sout) const;
}; // ParameterModel
} // treelink
} // mlplus
#endif // MLLIB_TREELINK_PARAMETER_MODEL_MANAGER_H_

