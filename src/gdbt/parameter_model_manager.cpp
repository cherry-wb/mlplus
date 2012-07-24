#include <string>
#include <fstream>
#include <queue>
#include <map>
#include <stdexcept>
#include "mlplus/tree_link/tree_link.h"
#include "mlplus/common/serial.h"
#include "mlplus/interface/log.h"
#include "parameter_model_manager.h"
#include "configuration.h"
#include "tree_boost.h"
#include "tree.h"
#include "node.h"
#include "cut.h"

namespace mlplus
{
namespace treelink
{

ParameterModelManager::ParameterModelManager()
{
}

ParameterModelManager::~ParameterModelManager()
{
}

bool ParameterModelManager::Load(TreeBoost*& pTreeBoost, std::istream& is)
{
    if(pTreeBoost)
    {
        delete pTreeBoost;
        pTreeBoost = NULL;
    }

    mlplus::common::SerialIn sin(is);
    if(sin.Bad())
        return false;

    int version = 0;
    sin >> version;
    if(version != TREELINK_VERSION)
        throw std::runtime_error("ParameterModelManager::Load version mismatch");

    // Load distribution.
    Loss::Type dist(Loss::UNKNOWN);
    sin >> dist;
    if(sin.Bad())
        return false;

    // Load tree count
    int nTreeCount(0);
    sin >> nTreeCount;
    if(sin.Bad())
        return false;
    if(nTreeCount <= 0)
        return false;

    // Load class count
    int nClassCount(0);
    sin >> nClassCount;
    if(sin.Bad())
        return false;
    if(nClassCount <= 0)
        return false;

    pTreeBoost = new TreeBoost(nTreeCount, nClassCount);
    pTreeBoost->SetType(dist);
    pTreeBoost->SetTreeCount(nTreeCount);
    pTreeBoost->SetClassCount(nClassCount);

    int nVariableCount(0);
    sin >> nVariableCount;
    if(sin.Bad())
        return false;
    if(nVariableCount <= 0)
        return false;
    pTreeBoost->SetVariableCount(nVariableCount);

    // Load init-estimation.
    float* pInitEst = pTreeBoost->GetInitEstimation();
    if(!pInitEst)
        return false;
    for(int idxClass = 0; idxClass < nClassCount; ++idxClass)
    {
        sin >> *pInitEst++;
        if(sin.Bad())
            return false;
    }

    // Load trees.
    for(int idxTree = 0; idxTree < nTreeCount; ++idxTree)
    {
        if(!LoadTree(pTreeBoost->GetTree(idxTree), sin, nClassCount))
        {
            return false;
        }
    }

    if(sin.Bad())
    {
        TERR("failed to load TreeLink model.");
        return false;
    }

    return true;
}

bool ParameterModelManager::LoadTree(Tree* pTree, mlplus::common::SerialIn& sin, int nClassCount) const
{
    std::map<int, Node*> mapNode;
    int nNodeCount(0);

    // Load node-count of the current tree.
    sin >> pTree->mSampleCount;
    sin >> nNodeCount;
    if(nNodeCount <= 0)
        return false;

    pTree->SetNodeCount(nNodeCount);
    Node* pRoot = NULL;

    // Load nodes.
    while(nNodeCount--)
    {
        int nNodeId(0);
        sin >> nNodeId;
        Cut* pCut = new Cut(nClassCount);
        Node* pNode = new Node(pCut, nNodeId);
        if(pRoot == NULL)
            pRoot = pNode;
        mapNode[nNodeId] = pNode;
        if(nNodeId > 0)
        {
            int nParentID = (nNodeId - 1) / 2;
            if(nNodeId % 2)
            {
                // Left child
                mapNode[nParentID]->SetLeftChild(pNode);
            }
            else
            {
                // Right child.
                mapNode[nParentID]->SetRightChild(pNode);
            }
        }
        if(!LoadNode(pNode, sin))
            return false;
    }

    pTree->SetRoot(pRoot);

    return true;
}

bool ParameterModelManager::LoadNode(Node* pNode, mlplus::common::SerialIn& sin) const
{
    bool bLeaf(false);

    Cut* pCut = pNode->GetCut();

    // Load whether leaf
    sin >> bLeaf;
    SampleMatrix::VariableType varType;

    if(!bLeaf)
    {
        int nVarIndex(0);
        sin >> nVarIndex;
        pCut->SetVariableIndex(nVarIndex);

        // Load variable type
        sin >> varType;
        pCut->SetVariableType(varType);

        float fGain;
        sin >> fGain;
        pCut->SetGain(fGain);

        // load separator
        if(varType == SampleMatrix::REAL)
        {
            float fSep(0.0);
            sin >> fSep;
            pCut->SetSeparator(fSep);
        }
        else
        {
            int nSep;
            Cut::DiscreteSeparatorType dsType(Cut::UNKNOWN);
            sin >> dsType;
            pCut->SetDiscreteSeparatorType(dsType);
            if(dsType == Cut::LEAVE_ONE_OUT)
            {
                sin >> nSep;
                pCut->SetSeparator(nSep);
            }
            else
            {
                int nSetSize(0);
                sin >> nSetSize;
                std::set<int> setSep;
                while(nSetSize--)
                {
                    int nSep(0);
                    sin >> nSep;
                    setSep.insert(nSep);
                }
                pCut->SetSeparator(setSep);
            }
        }
    }
    else
    {
        // load increment.
        float* pInc = pCut->GetIncrement();
        int nClassCount = pCut->GetClassCount();
        while(nClassCount--)
        {
            sin >> *pInc++;
        }
        // load sample number
        sin >> pNode->mSampleCount;
    }
    return true;
}

bool ParameterModelManager::Store(const TreeBoost* pTreeBoost, std::ostream& os)
{
    if(!pTreeBoost)
        return false;

    mlplus::common::SerialOut sout(os);
    if(sout.Bad())
        return false;

    // check version
    sout << TREELINK_VERSION;

    // Store distribution
    sout << pTreeBoost->GetType();
    if(sout.Bad())
        return false;

    // Store tree count.
    int nTreeCount = pTreeBoost->GetTreeCount();
    sout << nTreeCount;
    if(sout.Bad())
        return false;

    // Store class count
    int nClassCount = pTreeBoost->GetClassCount();
    sout << nClassCount;
    if(sout.Bad())
        return false;

    sout << pTreeBoost->GetVariableCount();
    if(sout.Bad())
        return false;

    // Store initial-estimation
    const float* pInitEst = pTreeBoost->GetInitEstimation();
    for(int idxClass = 0; idxClass < nClassCount; ++idxClass)
    {
        sout << *pInitEst++;
        if(sout.Bad())
            return false;
    }

    // Store Trees
    for(int idxTree = 0; idxTree < nTreeCount; ++idxTree)
    {
        const Tree* pTree = pTreeBoost->GetTree(idxTree);
        if(!StoreTree(pTree, sout))
        {
            return false;
        }
    }

    if(sout.Bad())
    {
        TERR("failed to store TreeLink model.");
        return false;
    }
    return true;
}

bool ParameterModelManager::StoreTree(const Tree* pTree, mlplus::common::SerialOut &sout) const
{
    const Node *pRoot = pTree->GetRoot();
    sout << pTree->mSampleCount;
    sout << pTree->GetNodeCount();
    std::queue<const Node *> queNode;
    queNode.push(pRoot);
    while(!queNode.empty())
    {
        const Node *pNode = queNode.front();
        queNode.pop();
        if(!pNode->IsLeaf())
        {
            queNode.push(pNode->GetLeftChild());
            queNode.push(pNode->GetRightChild());
        }
        if(!StoreNode(pNode, sout))
        {
            return false;
        }
    }
    return true;
}

bool ParameterModelManager::StoreNode(const Node *pNode, mlplus::common::SerialOut &sout) const
{
    sout << pNode->GetNodeId();
    const Cut *pCut = pNode->GetCut();
    const bool bLeaf = pNode->IsLeaf();
    sout << bLeaf;
    if(!bLeaf)
    {
        int nVarIndex = pCut->GetVariableIndex();
        sout << nVarIndex;
        const SampleMatrix::VariableType varType = pCut->GetVariableType();
        sout << varType;
        sout << pCut->GetGain();
        if(varType == SampleMatrix::REAL)
        {
            sout << pCut->GetSeparatorReal();
        }
        else
        {
            const Cut::DiscreteSeparatorType dsType = pCut->GetDiscreteSeparatorType();
            sout << dsType;
            if(dsType == Cut::LEAVE_ONE_OUT)
            {
                // Mode: Leave one out.
                sout << pCut->GetSeparatorNominal();
            }
            else
            {
                // Mode: Arbitrary.
                const std::set<int> *pSepSet = const_cast<Cut *>(pCut)->GetSeparatorSet();
                int nSetSize = pSepSet->size();
                sout << nSetSize;
                for(std::set<int>::const_iterator iterSep = pSepSet->begin();
                        iterSep != pSepSet->end(); ++iterSep)
                {
                    sout << *iterSep;
                }
            }
        }
    }
    else
    {
        int nClassCount = pCut->GetClassCount();
//		sout << nClassCount;
        const float *pInc = const_cast<Cut *>(pCut)->GetIncrement();
        for(int idxClass = 0; idxClass < nClassCount; ++idxClass)
        {
            sout << *pInc++;
        }
        sout << pNode->mSampleCount;
    }
    return true;
}

} // treelink
} // mlplus

