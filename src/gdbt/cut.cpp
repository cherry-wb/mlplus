#include <cstring>
#include "cut.h"
#include "mlplus/common/utility.h"
#include "mlplus/interface/log.h"

namespace mlplus
{
namespace treelink
{

Cut::Separator::Separator()
{
    bag = NULL;
}

// ctor.
Cut::Cut(int nClassCount, DiscreteSeparatorType discSepType)
    : mpNode(NULL), /*mpLeftGrad(NULL), mpRightGrad(NULL),*/ mnClassCount(nClassCount),
      mvarType(SampleMatrix::UNKNOWN), mnVariableIndex(0), mfGain(0.0), mfDistance(0.0), mpIncrement(NULL),
      mpLeftIncrement(NULL), mpRightIncrement(NULL),  msepType(discSepType), mseparator()
{
    mpLeftIncrement = new float [mnClassCount];
    mpRightIncrement = new float [mnClassCount];
    mpIncrement = new float [mnClassCount];
    std::memset(mpLeftIncrement, 0, sizeof(float) * mnClassCount);
    std::memset(mpRightIncrement, 0, sizeof(float) * mnClassCount);
    std::memset(mpIncrement, 0, sizeof(float) * mnClassCount);
}

// dtor
Cut::~Cut()
{
    if(mpLeftIncrement)
        delete [] mpLeftIncrement;
    if(mpRightIncrement)
        delete [] mpRightIncrement;
    if(mpIncrement)
        delete [] mpIncrement;
    if(mvarType == SampleMatrix::NOMINAL && msepType == ARBITRARY)
    {
        if(mseparator.bag)
            delete mseparator.bag;
    }
}

// Copy assignment.
const Cut& Cut::operator = (const Cut& cut)
{
    if(this == &cut)
        return cut;
    mpNode = cut.mpNode;
    mnClassCount = cut.mnClassCount;
    if(!mpLeftIncrement)
        mpLeftIncrement = new float [mnClassCount];
    std::memcpy(mpLeftIncrement, cut.mpLeftIncrement, sizeof(float) * mnClassCount);
    if(!mpRightIncrement)
        mpRightIncrement = new float [mnClassCount];
    std::memcpy(mpRightIncrement, cut.mpRightIncrement, sizeof(float) * mnClassCount);
    if(!mpIncrement)
        mpIncrement = new float [mnClassCount];
    std::memcpy(mpIncrement, cut.mpIncrement, sizeof(float) * mnClassCount);
    mnVariableIndex = cut.mnVariableIndex;
    SetVariableType(cut.mvarType);
    SetDiscreteSeparatorType(cut.msepType);
    if(mvarType == SampleMatrix::NOMINAL && msepType == ARBITRARY)
    {
        if(mseparator.bag == NULL)
            mseparator.bag = new std::set<int>();
        *mseparator.bag = *cut.mseparator.bag;
    }
    else
    {
        mseparator = cut.mseparator;
    }
    mfGain = cut.mfGain;
    mfDistance = cut.mfDistance;
    std::memcpy(mpIncrement, cut.mpIncrement, sizeof(float) * mnClassCount);
    return *this;
}

void Cut::Shrink(float fShrinkage)
{
    for(int idxClass = 0; idxClass < mnClassCount; ++idxClass)
    {
        mpLeftIncrement[idxClass] *= fShrinkage;
        mpRightIncrement[idxClass] *= fShrinkage;
    }
}

void Cut::GetIncrement(float*& pLeftInc, float*& pRightInc)
{
    pLeftInc = mpLeftIncrement;
    pRightInc = mpRightIncrement;
}

void Cut::GetIncrement(const float*& pLeftIncrement, const float*& pRightIncrement) const
{
    pLeftIncrement = mpLeftIncrement;
    pRightIncrement = mpRightIncrement;
}

float* Cut::GetIncrement()
{
    return mpIncrement;
}

bool Cut::BranchLeft(float fVarValue) const
{
    return fVarValue <= mseparator.real;
}

bool Cut::BranchLeft(int nVarValue) const
{
    if(msepType == LEAVE_ONE_OUT)
        return mseparator.nominal == nVarValue;
    else
    {
        return mseparator.bag->find(nVarValue) != mseparator.bag->end();
    }
}

Cut::DiscreteSeparatorType Cut::GetDiscreteSeparatorType() const
{
    return msepType;
}

void Cut::SetDiscreteSeparatorType(DiscreteSeparatorType sepType)
{
    if(mvarType == SampleMatrix::NOMINAL && msepType != sepType)
    {
        if(msepType == ARBITRARY)
        {
            delete mseparator.bag;
            mseparator.bag = NULL;
        }
        else if(sepType == ARBITRARY)
        {
            mseparator.bag = new std::set<int>();
        }
    }
    msepType  = sepType;
}

std::set<int>* Cut::GetSeparatorSet()
{
    if(mvarType == SampleMatrix::NOMINAL && msepType == ARBITRARY)
    {
        return mseparator.bag;
    }
    else
    {
        return NULL;
    }
}

void Cut::SetGain(float fGain)
{
    mfGain = fGain;
}

float Cut::GetGain() const
{
    return mfGain;
}

void Cut::SetDistance(float fDistance)
{
    mfDistance = fDistance;
}

float Cut::GetDistance() const
{
    return mfDistance;
}

void Cut::SetVariableIndex(int nVariableIndex)
{
    mnVariableIndex = nVariableIndex;
}

int Cut::GetVariableIndex() const
{
    return mnVariableIndex;
}

void Cut::SetVariableType(SampleMatrix::VariableType varType)
{
    if(msepType == ARBITRARY && mvarType != varType)
    {
        if(mvarType == SampleMatrix::NOMINAL)
        {
            if(mseparator.bag)
            {
                delete mseparator.bag;
                mseparator.bag = NULL;
            }
        }
        else if(varType == SampleMatrix::NOMINAL)
        {
            mseparator.bag = new std::set<int>();
        }
    }
    mvarType = varType;
}

SampleMatrix::VariableType Cut::GetVariableType() const
{
    return mvarType;
}

bool Cut::SetSeparator(int nSeparator)
{
    if(mvarType == SampleMatrix::NOMINAL && msepType == Cut::LEAVE_ONE_OUT)
    {
        mseparator.nominal = nSeparator;
        return true;
    }
    else
    {
        TERR("failed to set separator: invalid variable or separator type.");
        return false;
    }
}

bool Cut::SetSeparator(float fSeparator)
{
    if(mvarType == SampleMatrix::REAL)
    {
        mseparator.real = fSeparator;
        return true;
    }
    else
    {
        TERR("failed to set separator: invalid variable or separator type.");
        return false;
    }
}

bool Cut::SetSeparator(std::set<int>& setSeparator)
{
    if(mvarType == SampleMatrix::NOMINAL && msepType == Cut::ARBITRARY)
    {
        if(mseparator.bag == NULL)
            mseparator.bag = new std::set<int>();
        *mseparator.bag = setSeparator;
        return true;
    }
    else
    {
        TERR("failed to set separator: invalid variable or separator type.");
        return false;
    }
}

int Cut::GetSeparatorNominal() const
{
    return mseparator.nominal;
}

float Cut::GetSeparatorReal() const
{
    return mseparator.real;
}

void Cut::SetNode(Node* pNode)
{
    mpNode = pNode;
}

Node* Cut::GetNode()
{
    return mpNode;
}

int Cut::GetClassCount() const
{
    return mnClassCount;
}

float* Cut::GetLeftIncrement()
{
    return mpLeftIncrement;
}

float* Cut::GetRightIncrement()
{
    return mpRightIncrement;
}

void Cut::CopyBestCut(Cut* pBestCut)
{
    mvarType = pBestCut->mvarType;
    mnVariableIndex = pBestCut->mnVariableIndex;
    msepType = pBestCut->msepType;
    if(mvarType == SampleMatrix::NOMINAL && msepType == ARBITRARY)
    {
        mseparator.bag = new std::set<int>();
        *mseparator.bag = *pBestCut->mseparator.bag;
    }
    else
    {
        mseparator = pBestCut->mseparator;
    }
    mfGain = pBestCut->mfGain;
    common::utility::VectorCopy(mpLeftIncrement, pBestCut->mpLeftIncrement, mnClassCount);
    common::utility::VectorCopy(mpRightIncrement, pBestCut->mpRightIncrement, mnClassCount);
}

void Cut::ClearIncrementLeftRight()
{
    if(mpLeftIncrement)
    {
        delete [] mpLeftIncrement;
        mpLeftIncrement = NULL;
    }
    if(mpRightIncrement)
    {
        delete [] mpRightIncrement;
        mpRightIncrement = NULL;
    }
}

void Cut::ClearIncrementAll()
{
    if(mpLeftIncrement)
    {
        delete [] mpLeftIncrement;
        mpLeftIncrement = NULL;
    }
    if(mpRightIncrement)
    {
        delete [] mpRightIncrement;
        mpRightIncrement = NULL;
    }
    if(mpIncrement)
    {
        delete [] mpIncrement;
        mpIncrement = NULL;
    }
}

bool Cut::UpdateVariableImportance(std::vector<float>& vecVarImp)
{
    if(mfGain < 0.0)
    {
        TERR("failed to update variable importance: invalid gain.");
        return false;
    }
    vecVarImp[mnVariableIndex] += mfGain;
    return true;
}

void Cut::TakeDistanceAsGain()
{
    mfGain = mfDistance;
}

} //treelink
} //mlplus

