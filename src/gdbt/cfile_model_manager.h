#ifndef TREE_CFILE_MODEL_MANAGER_H_
#define TREE_CFILE_MODEL_MANAGER_H_
#include <stack>
#include <fstream>
#include "cut.h"
#include "node.h"
#include "tree.h"
#include "tree_boost.h"
#include "model_manager.h"
#include "sample_matrix.h"

namespace mlplus
{
namespace treelink
{

class CFileModelManager: public ModelManager
{
public:
    struct StackNode
    {
        Node* node;
        int index;
        int level;
    };
public:
    CFileModelManager();
    ~CFileModelManager();
    bool ExportCFileCore(const TreeBoost* pTreeBoost, std::fstream& os);
private:
    bool Pretreatment(const TreeBoost* pTreeBoost, std::fstream& os);
    bool TreeTraversal(const TreeBoost* pTreeBoost, std::fstream& os);
    void Transform(const TreeBoost* pTreeBoost, std::fstream& os);
    void EndProcess(std::fstream& os);

    bool PushIf(const Node* pNode, std::fstream& os);
    Node* PopElse(std::fstream& os);
    void Accumulation(const Node* pNode, std::fstream& os);
    void RightBracketProcess(std::fstream& os);
    void LogisticTransformOutput(std::fstream& os);
    void AdaboostTransformOutput(std::fstream& os);

private:
    int mnLevel;
    struct StackNode mstStackNode;
    std::stack<struct StackNode> mstkStack1;
    std::stack<struct StackNode> mstkStack2;

}; //CFileModel

} // treelink
} // mlplus

#endif    //    TREE_CFILE_MODEL_MANAGER_H_

