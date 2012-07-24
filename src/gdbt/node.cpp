#include <cstring>
#include "mlplus/interface/log.h"
#include "node.h"
#include "loss.h"
#include "sample_set.h"
#include "configuration.h"
#include "cut.h"
#include "sample_matrix.h"
#include "sample_set_manager.h"
#include "mlplus/common/utility.h"

namespace mlplus
{
namespace treelink
{

Node::Node(SampleSet* pSampleSet, const Configuration* pConf, const Loss* pLoss, int nNodeId)
    : mpSampleSet(pSampleSet), mpCut(NULL), mpConf(pConf), mpLeftChild(NULL), mpRightChild(NULL),
      mpParent(NULL), mpLoss(pLoss), mnNodeId(nNodeId), mSampleCount(0)
{
    int nClassCount = pSampleSet->GetClassCount();
    mpCut = new Cut(nClassCount, mpConf->GetDiscreteSeparatorType());
    mpCut->SetNode(this);
}

Node::Node(Cut* pCut, int nNodeId)
    : mpSampleSet(NULL), mpCut(pCut), mpConf(NULL), mpLeftChild(NULL), mpRightChild(NULL),
      mnNodeId(nNodeId), mSampleCount(0)
{
}

Node::~Node()
{
    if(mpSampleSet)
    {
        TERR("there is still a sample set in node %d!", mnNodeId);
    }
    if(mpCut)
    {
        delete mpCut;
    }
}

Node* Node::Climb(const float* pX) const
{
    if(IsLeaf())
        return NULL;
    int nVarIndex = mpCut->GetVariableIndex();
    SampleMatrix::VariableType varType = mpCut->GetVariableType();
    float fVarValue = pX[nVarIndex];
    if(varType == SampleMatrix::REAL)
    {
        if(mpCut->BranchLeft(fVarValue))
            return mpLeftChild;
        else
            return mpRightChild;
    }
    else
    {
        int nVarValue = static_cast<int>(fVarValue);
        if(mpCut->BranchLeft(nVarValue))
            return mpLeftChild;
        else
            return mpRightChild;
    }
    return NULL;
}

float* Node::GetIncrement()
{
    if(!IsLeaf())
        return NULL;
    else
        return mpCut->GetIncrement();
}

Cut* Node::CalculateBestCut()
{
    // only single loss.
    if(mnNodeId != 1)
    {
        UpdateSampleSet();
    }

    Cut* pBestCut = mpSampleSet->CalculateBestCut(mpLoss);
    if(!pBestCut)
        return NULL;

    // Save best cut to mpCut
    mpCut->CopyBestCut(pBestCut);
    if(pBestCut)
        delete pBestCut;
    return mpCut;
} // CalculateBestCut()

// Split the node using best cut.
bool Node::Split(Node*& pLeftChild, Node*& pRightChild)
{
    SampleSet* pLeftSampleSet = NULL;
    SampleSet* pRightSampleSet = NULL;
    int nSampleCount = mpSampleSet->GetSampleCount();
    if(!mpSampleSet->Split(mpCut, pLeftSampleSet, pRightSampleSet))
    {
        return false;
    }
    ClearSampleSet();
    int nLeftSampleCount = pLeftSampleSet->GetSampleCount();
    int nRightSampleCount = pRightSampleSet->GetSampleCount();
    TLOG("split: (%d:%d) => [(%d:%d), (%d:%d)]", mnNodeId,
         nSampleCount, mnNodeId * 2 + 1, nLeftSampleCount, mnNodeId * 2 + 2,
         nRightSampleCount);
    pLeftChild = new Node(pLeftSampleSet, mpConf, mpLoss, mnNodeId * 2 + 1);
    pRightChild = new Node(pRightSampleSet, mpConf, mpLoss, mnNodeId * 2 + 2);
    pLeftChild->mpParent = this;
    pRightChild->mpParent = this;
    mpLeftChild = pLeftChild;
    mpRightChild = pRightChild;
    mpLeftChild->mSampleCount = nLeftSampleCount;
    mpRightChild->mSampleCount = nRightSampleCount;
    return true;
} // Split()

void Node::TransmitIncrementToChildren()
{
    if(IsLeaf())
        return;

    int nClassCount = mpCut->GetClassCount();
    mpLeftChild->UpdateIncrement(mpCut->GetIncrement(), mpCut->GetLeftIncrement(), nClassCount);
    mpRightChild->UpdateIncrement(mpCut->GetIncrement(), mpCut->GetRightIncrement(), nClassCount);
}

void Node::SetSampleSet(SampleSet* pSampleSet)
{
    mpSampleSet = pSampleSet;
}

SampleSet* Node::GetSampleSet()
{
    return mpSampleSet;
}

void Node::SetCut(Cut* pCut)
{
    mpCut = pCut;
}

int Node::GetClassCount() const
{
    return mpCut->GetClassCount();
}

bool Node::UpdateEstimation(float* pEstimation) const
{
    if(!const_cast<Node*>(this)->IsLeaf())
        return false;
    const float* pIncrement = mpCut->GetIncrement();
    int nClassCount = GetClassCount();
    if(nClassCount == 1)
    {
        *pEstimation += *pIncrement;
    }
    else
    {
        common::utility::VectorAdd(pEstimation, pIncrement, GetClassCount());
    }
    return true;
}

void Node::UpdateIncrement(const float* pFatherIncrement, const float* pBranchIncrement, int nClassCount)
{
    float* pInc = mpCut->GetIncrement();
    if(nClassCount == 1)
    {
        *pInc = *pFatherIncrement + *pBranchIncrement;
    }
    else
    {
        std::memcpy(pInc, pFatherIncrement, sizeof(float) * nClassCount);
        common::utility::VectorAdd(pInc, pBranchIncrement, nClassCount);
    }
}

bool Node::UpdateSampleSet()
{
    bool bResult = mpLoss->UpdateSampleSet(mpSampleSet, mpCut->GetIncrement());
    return bResult;
}

Node* Node::GetLeftChild() const
{
    return mpLeftChild;
}

Node* Node::GetRightChild() const
{
    return mpRightChild;
}

void Node::SetLeftChild(Node* pLeftChild)
{
    mpLeftChild = pLeftChild;
}

void Node::SetRightChild(Node* pRightChild)
{
    mpRightChild = pRightChild;
}

Cut* Node::GetCut() const
{
    return mpCut;
}

void Node::ClearSampleSet()
{
    if(mpSampleSet)
    {
        mpSampleSet->GetManager()->PutSampleSet(mpSampleSet);
        mpSampleSet = NULL;
    }
}

bool Node::IsLeaf() const
{
    return !mpLeftChild && !mpRightChild;
}

int Node::GetNodeId() const
{
    return mnNodeId;
}

bool Node::UpdateVariableImportance(std::vector<float>& vecVarImp)
{
    if(!IsLeaf())
    {
        if(!mpCut->UpdateVariableImportance(vecVarImp))
        {
            TERR("failed to update variable importance: cut.");
            return false;
        }

        if(!mpLeftChild->UpdateVariableImportance(vecVarImp) ||
                !mpRightChild->UpdateVariableImportance(vecVarImp))
        {
            TERR("failed to update variable importance: child.");
            return false;
        }
    }
    return true;
}

double Node::GetLabelValue() const
{
    return static_cast<double>(mSampleCount);
}

Node *Node::GetParent()
{
    return mpParent;
}

} // treelink
} // mlplus

