#include <iostream>

#include "cfile_model_manager.h"
#include "mlplus/interface/log.h"

namespace mlplus
{
namespace treelink
{
CFileModelManager::CFileModelManager()
    : mnLevel(0)
{
}

CFileModelManager::~CFileModelManager()
{
}

bool CFileModelManager::ExportCFileCore(const TreeBoost* pTreeBoost, std::fstream &os)
{
    if(!Pretreatment(pTreeBoost, os))
        return false;
    if(!TreeTraversal(pTreeBoost, os))
        return false;
    Transform(pTreeBoost, os);
    EndProcess(os);
    return true;
}

bool CFileModelManager::Pretreatment(const TreeBoost* pTreeBoost, std::fstream &os)
{
    os << std::endl << "bool Predict(float* pX, int nSizex, float* pY, int nSizey) {" << std::endl;
    ++mnLevel;

    float* pInitEst = pTreeBoost->GetInitEstimation();
    int nClassCount = pTreeBoost->GetClassCount();
    for(int i = 0; i < nClassCount; ++i)
    {
        for(int j = 0; j < mnLevel; ++j)
            os << '\t';
        os << "pY[" << i << "] = " << pInitEst[i] << ";" << std::endl;
    }
    return true;
}

bool CFileModelManager::TreeTraversal(const TreeBoost* pTreeBoost, std::fstream &os)
{
    int pTreeCount = pTreeBoost->GetTreeCount();
    for(int i = 0; i < pTreeCount; ++i)
    {
        os << "//    Tree " << i + 1 << std::endl;
        Tree* pTree = pTreeBoost->GetTree(i);
        Node* pNode = const_cast<Node*>(pTree->GetRoot());
        while(pNode != NULL)
        {
            if(!pNode->IsLeaf())
            {
                if(!PushIf(pNode, os))         // push node and out if
                    return false;
                pNode = const_cast<Node*>(pNode->GetLeftChild());
            }
            else
            {
                Accumulation(pNode, os);     // display & change pY
                pNode = PopElse(os);    // root pop
                if(pNode != NULL)
                    pNode = const_cast<Node*>(pNode->GetRightChild());    // right subtree
            }
        }// end while
        RightBracketProcess(os);
    }// end for
    return true;
}

void CFileModelManager::RightBracketProcess(std::fstream &os)
{
    while(!mstkStack2.empty())
    {
        mnLevel = (mstkStack2.top()).level;
        for(int i = 0; i < mnLevel; ++i)
            os << '\t';
        os << "}" << std::endl;
        mstkStack2.pop();
    }
    os << std::endl;
}

void CFileModelManager::EndProcess(std::fstream &os)
{
    os << '\t' << "return true;" << std::endl;
    os << "}" << std::endl;
}

bool CFileModelManager::PushIf(const Node* pNode, std::fstream &os)
{
    Cut* pCut = pNode->GetCut();
    SampleMatrix::VariableType varType = pCut->GetVariableType();
    int nVarIndex = pCut->GetVariableIndex();

    mstStackNode.node = const_cast<Node*>(pNode);
    mstStackNode.index = nVarIndex;
    mstStackNode.level = mnLevel;
    mstkStack1.push(mstStackNode);

    if(varType == SampleMatrix::REAL)
    {
        // real separator
        float fRealSep = pCut->GetSeparatorReal();
        for(int i = 0; i < mnLevel; ++i)
            os << '\t';
        os << "if (pX[" << nVarIndex << "] <= " << fRealSep << "){" << std::endl;
    }
    else if(varType == SampleMatrix::NOMINAL)
    {
        // nominal separator
        Cut::DiscreteSeparatorType disSeparatorType = pCut->GetDiscreteSeparatorType();
        if(disSeparatorType == Cut::LEAVE_ONE_OUT)
        {
            int nNominalSep = pCut->GetSeparatorNominal();
            for(int i = 0; i < mnLevel; ++i)
                os << '\t' ;
            os << "if ((int)pX[" << nVarIndex << "] == " << nNominalSep << "){" << std::endl;
        }
        else if(disSeparatorType == Cut::ARBITRARY)
        {
            std::set<int>* pSetSeparator = pCut->GetSeparatorSet();
            if(pSetSeparator->empty())
            {
                TLOG("#cfile_model_manager.cc. Set Empty");
                return false;
            }
            for(int i = 0; i < mnLevel; ++i)
                os << '\t' ;
            std::set<int>::iterator setIter = pSetSeparator->begin();
            os << "if (";
            while(setIter != pSetSeparator->end())
            {
                os << "(int)pX[" << nVarIndex << "] == " << *setIter;
                if(++setIter != pSetSeparator->end())
                    os << " || ";
            }
            os << "){" << std::endl;
        }
        else
        {
            TLOG("#cfile_model_manager.cc. Case DiscreteSeparatorType::UNKNOWN");
            return false;
        }
    }
    else
    {
        TLOG("#cfile_model_manager.cc. Case SampleMatrix::VariableType::UNKNOWN");
        return false;
    }
    ++mnLevel;
    return true;
}

void CFileModelManager::Accumulation(const Node* pNode, std::fstream &os)
{
    Cut* pCut = pNode->GetCut();
    int nClassCount = pCut->GetClassCount();
    float* pCutIncrement = pCut->GetIncrement();
    for(int i = 0; i < nClassCount; ++i)
    {
        for(int j = 0; j < mnLevel; ++j)
            os << '\t';
        os << "pY[" << i << "] += " << pCutIncrement[i] << ";" << std::endl;
    }
}

Node* CFileModelManager::PopElse(std::fstream &os)
{
    if(mstkStack1.empty())
        return NULL;
    mstStackNode = mstkStack1.top();
    mstkStack1.pop();
    mnLevel = mstStackNode.level;

    // stack2
    while(!mstkStack2.empty())
    {
        int pStkLevel = (mstkStack2.top()).level;
        if(mstStackNode.level < pStkLevel)
        {
            for(int i = 0; i < pStkLevel; ++i)
                os << '\t';
            os << "}" << std::endl;
            mstkStack2.pop();
        }
        else
            break;
    }
    mstkStack2.push(mstStackNode);

    for(int i = 0; i < mnLevel; ++i)
        os << '\t';
    os << "}else{" << std::endl;
    ++mnLevel;

    return mstStackNode.node;
}

void CFileModelManager::Transform(const TreeBoost* pTreeBoost, std::fstream &os)
{
    Loss::Type distribution = pTreeBoost->GetType();
    if(distribution == Loss::LOGISTIC)
        LogisticTransformOutput(os);
    else if(distribution == Loss::ADABOOST)
        AdaboostTransformOutput(os);
}

void CFileModelManager::AdaboostTransformOutput(std::fstream &os)
{
    os << "//" << '\t' << "AdaboostTransform" << std::endl;
    for(int i = 0; i < mnLevel; ++i)
        os << '\t';
    os << "int i = 0;" << std::endl;

    for(int i = 0; i < mnLevel; ++i)
        os << '\t';
    os << "for(i = 0; i < nSizey; ++i)" << std::endl;

    ++mnLevel;
    for(int i = 0; i < mnLevel; ++i)
        os << '\t';
    os << "pY[i] = (pY[i] > 0.0)? 1 : -1;" << std::endl;
    --mnLevel;
}

void CFileModelManager::LogisticTransformOutput(std::fstream &os)
{
    os << "//" << '\t' << "LogisticTransform" << std::endl;
    for(int i = 0; i < mnLevel; ++i)
        os << '\t';
    os << "float fExpSum(0.0);" << std::endl;

    for(int i = 0; i < mnLevel; ++i)
        os << '\t';
    os << "int i = 0;" << std::endl;

    for(int i = 0; i < mnLevel; ++i)
        os << '\t';
    os << "for(i = 0; i < nSizey; ++i) {" << std::endl;
    ++mnLevel;

    for(int i = 0; i < mnLevel; ++i)
        os << '\t';
    os << "pY[i] = exp(pY[i]);" << std::endl;

    for(int i = 0; i < mnLevel; ++i)
        os << '\t';
    os << "fExpSum += pY[i];" << std::endl;
    --mnLevel;

    for(int i = 0; i < mnLevel; ++i)
        os << '\t';
    os << "}" << std::endl;

    for(int i = 0; i < mnLevel; ++i)
        os << '\t';
    os << "for (i = 0; i < nSizey; ++i)" << std::endl;
    ++mnLevel;

    for(int i = 0; i < mnLevel; ++i)
        os << '\t';
    os << "pY[i] /= fExpSum;" << std::endl;
    --mnLevel;
}

} // treelink
} // mlplus
