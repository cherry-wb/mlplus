#include "model_manager.h"
#include "parameter_model_manager.h"
#include "cfile_model_manager.h"
#include "configuration.h"

namespace mlplus
{
namespace treelink
{

ModelManager::ModelManager()
{
}

ModelManager::~ModelManager()
{
}

ModelManagerFactory::ModelManagerFactory(ModelManager::ModelFileType modelFileType)
    : mmodelFileType(modelFileType)
{
}

ModelManagerFactory::~ModelManagerFactory()
{
}

ModelManager* ModelManagerFactory::Make()
{
    switch(mmodelFileType)
    {
    case ModelManager::PARAMFILE:
        return new ParameterModelManager();
    case ModelManager::CFILE:
        return new CFileModelManager();
    default:
        return NULL;
    }
}

bool ModelManager::Load(TreeBoost*& pTreeBoost, std::istream& is)
{
    static_cast<void>(pTreeBoost);
    static_cast<void>(is);
    return false;
}

bool ModelManager::Store(const TreeBoost* pTreeBoost, std::ostream& os)
{
    static_cast<void>(pTreeBoost);
    static_cast<void>(os);
    return false;
}

bool ModelManager::ExportCFileCore(const TreeBoost* pTreeBoost, std::ofstream& os)
{
    static_cast<void>(pTreeBoost);
    static_cast<void>(os);
    return false;
}

} //treelink
} // mlplus

