#include <list>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include "mlplus/interface/log.h"
#include "mlplus/common/utility.h"
#include "tree.h"
#include "node.h"
#include "loss.h"
#include "configuration.h"
#include "cut.h"

namespace mlplus
{
namespace treelink
{

Tree::Tree()
    : mpRoot(NULL), mnNodeCount(0)
{
}

Tree::~Tree()
{
    ClearAllNodes();
}

void Tree::Clear()
{
    ClearAllNodes();
    mpRoot = NULL;
}

bool Tree::UpdateEstimation(const float* pX, float* pY) const
{
    Node* pCurrentNode = NULL;
    Node* pNextNode = mpRoot;

    // Search for proper leaf
    while(pNextNode != NULL)
    {
        pCurrentNode = pNextNode;
        pNextNode = pCurrentNode->Climb(pX);
    }

    if(pCurrentNode == NULL)
    {
        TERR("failed to update estimation: empty tree node.");
        return false;
    }
    return pCurrentNode->UpdateEstimation(pY);
    //return true;
}

// Grow the tree.
bool Tree::Grow(SampleSet* pSampleSet, const Configuration* pConf, const Loss* pLoss)
{
    mSampleCount = pSampleSet->GetSampleCount();
    if(pSampleSet->GetSampleCount() <= 1)
    {
        TERR("failed to grow a tree: sample set has too few samples (<=1).");
        return false;
    }
    mpRoot = new Node(pSampleSet, pConf, pLoss, 0);
    mnNodeCount = 1;
    std::list<Cut*> lstCut;
    std::list<Node*> lstNode;
    lstNode.push_back(mpRoot);
    int nMaxLeaf = pConf->GetMaxLeafCount();
    if(nMaxLeaf <= 1)
    {
        TERR("failed to grow a tree: max leaf count is %d.", nMaxLeaf);
        return false;
    }

    bool bFastTrain = pConf->IsFastTrain();
    if(bFastTrain)
    {
        pLoss->CalculateGradient(pSampleSet);
    }

    for(int idxLeaf = 1; idxLeaf < nMaxLeaf; ++idxLeaf)
    {
        Cut* pCut = NULL;

        if(lstNode.empty())
            break;

        // Update new-leaf list.
        // Get the un-processed node and calculate best cut for them.
        while(!lstNode.empty())
        {
            Node* pLeaf = lstNode.front();
            lstNode.pop_front();
            if(common::utility::Logarithmic(pLeaf->GetNodeId() + 1) + 1 >= pConf->GetMaxTreeDepth())
            {
                pCut = pLeaf->GetCut();
                if(!pCut)
                {
                    TWARN("failed to get cut from a leaf node");
                }
            }
            else
            {
                pCut = pLeaf->CalculateBestCut();
                if(!pCut)
                {
                    pCut = pLeaf->GetCut();
                    if(!pCut)
                    {
                        TWARN("failed to get cut from a leaf node after failed to calculate best cut");
                    }
                }
            }
            if(pCut)
            {
                InsertCutToList(pCut, lstCut);        // Cut evaluation order maintained here.
            }
            else
            {
                pLeaf->ClearSampleSet();
            }
        } // while

        // It should be the front, because the order is maintained during the InsertCutToList() process.
        if(lstCut.empty())
        {
            TWARN("empty cut list.");
            break;
        }
        Cut* pBestCut = lstCut.front();
        if(!pBestCut)
        {
            TWARN("failed to train: best cut is NULL!");
            ClearCutList(lstCut);
            ClearNodeList(lstNode);
            break;
        }
        if(pBestCut->GetGain() <= 0.0)
        {
            TWARN("the best gain is too small!");
            break;
        }

        if(pBestCut->GetVariableType() == SampleMatrix::UNKNOWN)
        {
            TERR("unknown variable type in the best cut.");
            ClearCutList(lstCut);
            ClearNodeList(lstNode);
            return false;
        }

        // Prepare two NULL pointers to get the left and right child.
        Node* pLeftChild = NULL;
        Node* pRightChild = NULL;

        // Split using the best cut.
        Node* pNode = pBestCut->GetNode();
        if(!pNode)
        {
            TWARN("invalid cut-specific node.");
            ClearCutList(lstCut);
            ClearNodeList(lstNode);
            break;
        }
        if(!pNode->Split(pLeftChild, pRightChild))
        {
            TWARN("failed to split node: premature of this tree.");
            if(!bFastTrain)
            {
                ClearCutList(lstCut);
                ClearNodeList(lstNode);
            }
            break;
        }
        lstCut.pop_front();

        if(!pConf->IsFastTrain())
        {
            pNode->TransmitIncrementToChildren();
            pNode->GetCut()->ClearIncrementAll();
            pNode->ClearSampleSet();
        }

        mnNodeCount += 2;

        // Save the two new child into list.
        lstNode.push_back(pLeftChild);
        lstNode.push_back(pRightChild);
    } // for

    for(std::list<Cut*>::iterator iterCutPtr = lstCut.begin(); iterCutPtr != lstCut.end(); ++iterCutPtr)
    {
        Cut* pCut = *iterCutPtr;
        Node* pNode = pCut->GetNode();
        SampleSet* pSampleSet = pNode->GetSampleSet();

        if(pSampleSet == NULL)
        {
            TERR("failed to calculate optimal increment: empty sample set.");
            pNode->ClearSampleSet();
            pCut->ClearIncrementAll();
            ClearCutList(lstCut);
            ClearNodeList(lstNode);
            return false;
        }

        if(!pLoss->CalculateOptimal(pSampleSet, pCut))
        {
            TERR("failed to calculate optimal increment: invalid cut %d in list!", pNode->GetNodeId());
            pNode->ClearSampleSet();
            pCut->ClearIncrementAll();
            ClearCutList(lstCut);
            ClearNodeList(lstNode);
            return false;
        }
        pCut->ClearIncrementLeftRight();
        pNode->ClearSampleSet();
    }
    for(std::list<Node*>::iterator iterNodePtr = lstNode.begin(); iterNodePtr != lstNode.end();
            ++iterNodePtr)
    {
        Node* pNode = *iterNodePtr;
        Cut* pCut = pNode->GetCut();
        SampleSet* pSampleSet = pNode->GetSampleSet();
        if(!pLoss->CalculateOptimal(pSampleSet, pCut))
        {
            TERR("failed to calculate optimal increment: invalid node %d in list!", pNode->GetNodeId());
            pNode->ClearSampleSet();
            pCut->ClearIncrementAll();
            ClearCutList(lstCut);
            ClearNodeList(lstNode);
            return false;
        }
        pNode->ClearSampleSet();
    }

    ClearCutList(lstCut);
    ClearNodeList(lstNode);

    return true;
} // Grow

void Tree::InsertCutToList(Cut* pCut, std::list<Cut*>& lstCut)
{
    // Insert the pCut to proper position in lstCut.
    std::list<Cut*>::iterator iterCut;
    for(iterCut = lstCut.begin(); iterCut != lstCut.end(); ++iterCut)
    {
        if((*iterCut)->GetGain() < pCut->GetGain())
        {
            break;
        }
    }
    lstCut.insert(iterCut, pCut);
} //UpdateCutList

const Node* Tree::GetRoot() const
{
    return mpRoot;
}

void Tree::SetRoot(Node* pRoot)
{
    mpRoot = pRoot;
}

int Tree::GetNodeCount() const
{
    return mnNodeCount;
}

void Tree::SetNodeCount(int nNodeCount)
{
    mnNodeCount = nNodeCount;
}

void Tree::ClearNodeList(std::list<Node*>& lstNode)
{
    // Delete all node-specific SampleSet
    for(std::list<Node*>::iterator iterNode = lstNode.begin(); iterNode != lstNode.end(); ++iterNode)
    {
        (*iterNode)->GetCut()->ClearIncrementLeftRight();
        (*iterNode)->ClearSampleSet();
    }
}

void Tree::ClearCutList(std::list<Cut*>& lstCut)
{
    // Delete all cuts surviving in the list.
    for(std::list<Cut*>::iterator iterCut = lstCut.begin(); iterCut != lstCut.end(); ++iterCut)
    {
        if(*iterCut != NULL)
        {
            (*iterCut)->ClearIncrementLeftRight();
            (*iterCut)->GetNode()->ClearSampleSet();
        }
    }
}

bool Tree::UpdateVariableImportance(std::vector<float>& vecVarImp)
{
    if(!mpRoot)
    {
        TERR("failed to update variable importance: empty tree.");
        return false;
    }
    return mpRoot->UpdateVariableImportance(vecVarImp);
}

double Tree::CalculateLabelValue(const std::vector<float>& x) const
{
    const Node* current = 0;
    const Node* next = mpRoot;
    while(next)
    {
        current = next;
        next = next->Climb(&x.front());
    }
    if(!current)
        throw std::runtime_error("Exception: Tree::CalculateLabelValue");
    const Cut* cut = current->mpCut;
    double norm = 0;
    for(int i = 0; i < cut->mnClassCount; ++i)
    {
        double delta = cut->mpIncrement[i];
        norm += delta * delta;
    }
    norm = sqrt(norm);
    return norm * log(mSampleCount / current->GetLabelValue());
}

void Tree::ClearAllNodes()
{
    if(mpRoot)
    {
        std::list<Node*> lstNode;
        lstNode.push_back(mpRoot);
        while(!lstNode.empty())
        {
            Node* pNode = lstNode.front();
            if(!pNode->IsLeaf())
            {
                lstNode.push_back(pNode->GetLeftChild());
                lstNode.push_back(pNode->GetRightChild());
            }
            delete pNode;
            lstNode.pop_front();
        }
    }
}

} // treelink
} // mlplus

