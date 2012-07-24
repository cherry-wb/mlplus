#ifndef MLLIB_TREELINK_CC_
#define MLLIB_TREELINK_CC_

//
//this is the treelink implement wrapper so it can integrate into the mlplus
//
#include <iostream>
#include <stdexcept>
#include "mlplus/mlplus.h"
#include "mlplus/mllib_sample_format.h"
#include "mlplus/interface/log.h"
#include "mlplus/common/utility.h"
#include "mlplus/common/serial.h"

#ifndef MLLIB_TREELINK_H_
#include "mlplus/tree_link/tree_link.h"
#endif

#ifndef TREE_CFILE_MODEL_MANAGER_H_
#include "cfile_model_manager.h"
#endif

#include "learner.h"
#include "predictor.h"



namespace mlplus
{

TreeLink::TreeLink()
    : mpLearner(NULL), mpPredictor(NULL), mstrParamFile(std::string("treelink.conf")),
      mstrModelFile(std::string("treelink.mod")), mstrModelName(std::string("treelink"))
{

}


TreeLink::~TreeLink()
{
    delete mpLearner;
    delete mpPredictor;
}


bool TreeLink::TrainModel(const MlSampleContainer& samples)
{
    if(mpLearner)
        delete mpLearner;
    treelink::LearnerFactory learnerFactory(&samples, mstrParamFile);
    mpLearner = learnerFactory.Make();
    if(mpLearner == NULL)
    {
        return false;
    }

    if(mpPredictor)
        delete mpPredictor;
    mpPredictor = mpLearner->Train();
    if(!mpPredictor)
    {
        return false;
    }

    return true;
}


bool TreeLink::Predict(MlSample& sample, const std::vector<size_t>* candidate /* = 0 */)
{
    if(mpPredictor == NULL)
    {
        return false;
    }

    //if (sample.GetTargetSize()==1)
    int nTargetSize = MlModel::mpFormat->GetTargetSize();
    if(1 == nTargetSize)
    {
        int nSize = sample.GetFeatureSize();
        std::vector<float> vecFeatVal;
        vecFeatVal.reserve(nSize);
        Data_t tmpValue;

        for(int k = 0; k < nSize; ++k)
        {
            if(sample.GetFeatureValue(k, tmpValue))
            {
                vecFeatVal.push_back(tmpValue);
            }
            else
            {
                vecFeatVal.push_back(0.0);
            }
        }
        Target_t fPred = 0;
        if(!mpPredictor->Predict(vecFeatVal, fPred))
            return false;
        sample.SetTargetValue(fPred);

    }
    else if(nTargetSize >= 2)
    {
        std::vector<float> vecFeatVal;
        std::vector<float> vecTarget;
        Data_t tmpValue;

        int nFeatureSize = sample.GetFeatureSize();
        //int nTargetSize = sample.GetTargetSize();
        vecFeatVal.reserve(nFeatureSize);
        vecTarget.reserve(nTargetSize);
        for(int k = 0; k < nFeatureSize; ++k)
        {
            if(sample.GetFeatureValue(k, tmpValue))
            {
                vecFeatVal.push_back(tmpValue);
            }
            else
            {
                vecFeatVal.push_back(0.0);
            }
        }
        sample.AllocateTargetArray(nTargetSize);
        mpPredictor->Predict(vecFeatVal, vecTarget);
        Target_t* pTargetArray = sample.GetTargetArray();

        Target_t maxTarget = 0.0;
        int maxIndex = 0;
        for(int k = 0; k < nTargetSize; ++k)
        {
            if(maxTarget < vecTarget[k])
            {
                maxTarget = vecTarget[k];
                maxIndex = k;
            }
            pTargetArray[k] = vecTarget[k];
        }
        sample.SetTargetValue(maxIndex);
    }
    return true;
}


bool TreeLink::Predict(MlSampleContainer& samples)
{
    if(mpPredictor == NULL)
    {
        return false;
    }

    if(samples.GetTargetSize() == 1)
    {
        std::vector<float> vecFeatVal;
        Data_t tmpValue;

        int nSampleSize = samples.Size();
        int nFeatureSize = samples.GetFeatureSize();
        for(int j = 0; j < nSampleSize; ++j)
        {
            vecFeatVal.clear();
            for(int k = 0; k < nFeatureSize; ++k)
            {
                if(samples[j]->GetFeatureValue(k, tmpValue))
                {
                    vecFeatVal.push_back(tmpValue);
                }
                else
                {
                    vecFeatVal.push_back(0.0);
                }
            }
            float fPred = 0;
            if(!mpPredictor->Predict(vecFeatVal, fPred))
                return false;
            (samples[j])->SetTargetValue(fPred);
        }
    }
    else if(samples.GetTargetSize() >= 2)
    {
        std::vector<float> vecFeatVal;
        std::vector<float> vecTarget;
        Data_t tmpValue;

        int nSampleSize = samples.Size();
        int nFeatureSize = samples.GetFeatureSize();
        int nTargetSize = samples.GetTargetSize();
        for(int j = 0; j < nSampleSize; ++j)
        {
            vecFeatVal.clear();
            for(int k = 0; k < nFeatureSize; ++k)
            {
                if(samples[j]->GetFeatureValue(k, tmpValue))
                {
                    vecFeatVal.push_back(tmpValue);
                }
                else
                {
                    vecFeatVal.push_back(0.0);
                }
            }
            samples[j]->AllocateTargetArray(nTargetSize);
            mpPredictor->Predict(vecFeatVal, vecTarget);
            Target_t* pTargetArray = samples[j]->GetTargetArray();

            Target_t maxTarget = 0.0;
            int maxIndex = 0;
            for(int k = 0; k < nTargetSize; ++k)
            {
                if(maxTarget < vecTarget[k])
                {
                    maxTarget = vecTarget[k];
                    maxIndex = k;
                }
                pTargetArray[k] = vecTarget[k];
            }
            samples[j]->SetTargetValue(maxIndex);
        }
    }
    return true;
}

bool TreeLink::CalculateLabelValue(MlSample& sample)
{
    if(!mpPredictor)
        throw std::runtime_error("TreeLink::CalculateLabelValue");

    size_t nDim = sample.GetFeatureSize();
    const Data_t* feat = 0;
    sample.GetFeatureValues(feat);
    std::vector<float> x(feat, feat + nDim);
    double value = mpPredictor->CalculateLabelValue(x);
    sample.SetTargetValue(value);
    return true;
}

bool TreeLink::CalculateLabelValue(MlSampleContainer& container)
{
    size_t nSampleSize = container.Size();
    for(size_t i = 0; i < nSampleSize; ++i)
    {
        MlSample* sample = container.GetSample(i);
        if(!sample)
            return false;
        CalculateLabelValue(*sample);
    }
    return true;
}

bool TreeLink::LoadModel(std::istream& isModel)
{
    if(isModel.bad())
        return false;

    // The format load is impl'd in MlModel.

    mpPredictor = new treelink::Predictor();
    if(mpPredictor == NULL)
    {
        return false;
    }

    if(!mpPredictor->Load(isModel))
    {
        TERR("failed to load tree.");
        return false;
    }

    return true;
}


bool TreeLink::SaveModel(std::ostream& osModel)
{
    if(osModel.bad())
        return false;

    if(mpPredictor == NULL)
    {
        return false;
    }

    if(!mpPredictor->Store(osModel))
    {
        return false;
    }
    return true;
}


bool TreeLink::Clear()
{
    if(mpPredictor == NULL)
    {
        return true;
    }

    if(MlModel::mpFormat != NULL && MlModel::mbMyFormat == true)
    {
        delete MlModel::mpFormat;
    }

    MlModel::mpFormat = NULL;
    MlModel::mbMyFormat = false;

    return mpPredictor->Clear();
}


bool TreeLink::SetParameters(const char* pParam)
{

    if(pParam == NULL)
    {
        return false;
    }

    mstrParamFile = std::string(pParam);
    return true;
}


std::string& TreeLink::GetName()
{
    return mstrModelName;
}


bool TreeLink::ExportCFileCore(std::fstream& os)
{
    treelink::CFileModelManager* pFileModelManager = new treelink::CFileModelManager;
    pFileModelManager->ExportCFileCore(mpPredictor->GetTreeBoost(), os);
    delete pFileModelManager;
    return true;
}


bool TreeLink::AnalyzeVariableImportance(std::vector<float>& vecVarImp)
{
    if(!mpPredictor)
        return false;
    return mpPredictor->AnalyzeVariableImportance(vecVarImp);
}

bool TreeLink::Print(std::ostream& os) const
{
    os << "mlplus version: " << common::utility::VersionToString(MLLIB_VERSION) << std::endl;
    os << "model version: " << common::utility::VersionToString(mnVersion) << std::endl;
    return true;
}

bool TreeLink::SetUsedTreeCount(size_t nUsedTreeCount)
{
    if(!mpPredictor)
        return false;
    return mpPredictor->SetUsedTreeCount(nUsedTreeCount);
}

} // mlplus

#endif

////////////////////////////End Of Implementation////////////////////////////

