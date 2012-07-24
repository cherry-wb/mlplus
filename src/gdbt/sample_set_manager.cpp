#include <algorithm>
#include "sample_set.h"
#include "sample_set_manager.h"

namespace mlplus
{
namespace treelink
{

SampleSetManager::SampleSetManager(SampleSet* pSampleSet)
    : mlstUsed(), mlstAvail(1, pSampleSet)
{
    pSampleSet->SetManager(this);
}

SampleSetManager::~SampleSetManager()
{
    for(std::list<SampleSet*>::iterator iterUsed = mlstUsed.begin();
            iterUsed != mlstUsed.end(); ++iterUsed)
    {
        if(*iterUsed)
            delete *iterUsed;
    }
    for(std::list<SampleSet*>::iterator iterAvail = mlstAvail.begin();
            iterAvail != mlstAvail.end(); ++iterAvail)
    {
        if(*iterAvail)
            delete *iterAvail;
    }
}

SampleSet* SampleSetManager::TakeSampleSet(SampleSet* pSampleSet)
{
    if(!pSampleSet)
        return NULL;
    SampleSet* pAvail = NULL;
    if(mlstAvail.empty())
    {
        // Create a new SampleSet;
        pAvail = new SampleSet(*pSampleSet);
        pAvail->SetManager(this);
        // Add the available SampleSet to Used list.
        mlstUsed.push_back(pAvail);
    }
    else
    {
        pAvail = mlstAvail.front();
        mlstAvail.pop_front();
        mlstUsed.push_back(pAvail);
    }
    pAvail->CopyFrom(*pSampleSet);
    return pAvail;
}

SampleSet* SampleSetManager::TakeSampleSet(SampleMatrix* pMatrix)
{
    if(!pMatrix)
        return NULL;
    SampleSet* pAvail = NULL;
    if(mlstAvail.empty())
    {
        // Create a new SampleSet;
        pAvail = new SampleSet(pMatrix);
        pAvail->SetManager(this);
        // Add the available SampleSet to Used list.
        mlstUsed.push_back(pAvail);
    }
    else
    {
        pAvail = mlstAvail.front();
        mlstAvail.pop_front();
        mlstUsed.push_back(pAvail);
    }
    pAvail->CopyFrom(pMatrix);
    return pAvail;
}

bool SampleSetManager::PutSampleSet(SampleSet* pSampleSet)
{
    if(!pSampleSet)
        return false;
    std::list<SampleSet*>::iterator iterPos = std::find(mlstUsed.begin(), mlstUsed.end(), pSampleSet);
    if(iterPos == mlstUsed.end())
        return false;
    mlstUsed.erase(iterPos);
    mlstAvail.push_back(pSampleSet);
    return true;
}

} // treelink
} // mlplus

