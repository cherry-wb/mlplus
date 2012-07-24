#include <stdexcept>
#include "mlplus/interface/log.h"
#include "predictor.h"
#include "tree_boost.h"
#include "cfile_model_manager.h"

namespace mlplus
{
namespace treelink
{

Predictor::Predictor()
    : mpTreeBoost(NULL)
{
}

Predictor::Predictor(TreeBoost* pTreeBoost)
    : mpTreeBoost(pTreeBoost)
{
}

Predictor::~Predictor()
{
    if(mpTreeBoost)
        delete mpTreeBoost;
}

// Predict
bool Predictor::Predict(const std::vector<float>& x, float& y) const
{
    return mpTreeBoost->Predict(x, y);
} // Predict()

bool Predictor::Predict(const std::vector<float>& x, std::vector<float>& y) const
{
    return mpTreeBoost->Predict(x, y);
}

bool Predictor::Load(std::istream& is)
{
    ModelManagerFactory mmf(ModelManager::PARAMFILE);
    ModelManager* pModelManager = mmf.Make();
    if(!pModelManager)
    {
        TERR("unable to make ModelManager.");
        return false;
    }
    bool bRet = pModelManager->Load(mpTreeBoost, is);
    if(pModelManager)
        delete pModelManager;
    return bRet;
}

bool Predictor::Store(std::ostream& os)
{
    ModelManagerFactory mmf(ModelManager::PARAMFILE);
    ModelManager* pModelManager = mmf.Make();
    if(!pModelManager)
    {
        TERR("unable to make ModelManager.");
        return false;
    }
    bool bRet = pModelManager->Store(mpTreeBoost, os);
    if(pModelManager)
        delete pModelManager;
    return bRet;
}

bool Predictor::Clear()
{
    if(mpTreeBoost)
    {
        delete mpTreeBoost;
        mpTreeBoost = NULL;
    }
    return true;
}

TreeBoost* Predictor::GetTreeBoost()
{
    return mpTreeBoost;
}

bool Predictor::AnalyzeVariableImportance(std::vector<float>& vecVarImp)
{
    if(!mpTreeBoost)
        return false;
    return mpTreeBoost->AnalyzeVariableImportance(vecVarImp);
}

double Predictor::CalculateLabelValue(const std::vector<float>& x) const
{
    if(!mpTreeBoost)
        throw std::runtime_error("Exception: Predictor::CalculateLabelValue");
    return mpTreeBoost->CalculateLabelValue(x);
}

bool Predictor::SetUsedTreeCount(size_t nUsedTreeCount)
{
    if(!mpTreeBoost)
        return false;
    return mpTreeBoost->SetUsedTreeCount(nUsedTreeCount);
}

} // treelink
} // mlplus

