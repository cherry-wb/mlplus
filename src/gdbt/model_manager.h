//
// ModelManager: description of model data.
//
#ifndef MLLIB_TREELINK_MODELMANAGER_H_
#define MLLIB_TREELINK_MODELMANAGER_H_

#include <vector>
#include <iostream>
#include "tree_boost.h"
#include "tree.h"
#include "node.h"

namespace mlplus
{
namespace treelink
{
class ModelManager
{
public:
    enum ModelFileType
    {
        UNKNOWN = 0,
        PARAMFILE,
        CFILE
    };

public:
    ModelManager();
    virtual ~ModelManager();
    virtual bool Load(TreeBoost*& pTreeBoost, std::istream& is);
    virtual bool Store(const TreeBoost* pTreeBoost, std::ostream& os);
    virtual bool ExportCFileCore(const TreeBoost* pTreeBoost, std::ofstream& os);
private:
}; // ModelManager

class ModelManagerFactory
{
public:
    ModelManagerFactory(ModelManager::ModelFileType modelFileType);
    ~ModelManagerFactory();
    ModelManager* Make();
private:
    ModelManager::ModelFileType mmodelFileType;
}; // ModelManagerFactory

} // treelink
} // mlplus
#endif // MLLIB_TREELINK_MODEL_H_

