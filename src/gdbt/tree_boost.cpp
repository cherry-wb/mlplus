#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include "tree_boost.h"
#include "loss.h"
#include "tree.h"
#include "configuration.h"
#include "log.h"

namespace mlplus
{
namespace treelink
{

TreeBoost::TreeBoost()
    : mvecpTrees(0), mLossType(Loss::UNKNOWN), mnClassCount(0),
      mnVariableCount(0), mpInitEst(NULL)
{
}

TreeBoost::TreeBoost(int nTreeCount, int nClassCount)
    : mvecpTrees(0), mLossType(Loss::UNKNOWN), mnClassCount(nClassCount),
      mnVariableCount(0), mpInitEst(NULL)
{
    if(nTreeCount <= 0 || nClassCount <= 0)
    {
        ERROR("invalid tree count or class count.");
        return;
    }
    mvecpTrees.resize(nTreeCount);
    mpInitEst = new float [nClassCount];
}

TreeBoost::~TreeBoost()
{
    free();
}

bool  TreeBoost::free()
{
    for(std::vector<Tree*>::const_iterator iterpTree = mvecpTrees.begin();
            iterpTree != mvecpTrees.end(); ++iterpTree)
    {
        if(*iterpTree)
            delete *iterpTree;
    }
    mvecpTrees.clear();
    if(mpInitEst)
    {
        delete [] mpInitEst;
        mpInitEst = NULL;
    }
}
bool TreeBoost::predict(const std::vector<float>& x, float& y) const
{
    std::vector<float> dist;
    if(predict(x, dist))
    {
        if(mLossType == Loss::LOGISTIC)
        {
            size_t idxBestClass = 0;
            float fBestProb = dist[0];
            size_t idxClass = 0;
            for(idxClass = 1; idxClass < dist.size(); ++idxClass)
            {
                if(fBestProb < dist[idxClass])
                {
                    fBestProb = dist[idxClass];
                    idxBestClass = idxClass;
                }
            }
            y = static_cast<float>(idxBestClass);
            return true;
        }
        else
        {
            y = dist[0];
            return true;
        }
    }
    else
    {
        WARN("failed to predict.");
        return false;
    }
}

bool TreeBoost::predict(const std::vector<float>& x, std::vector<float>& y) const
{
    y.resize(mnClassCount);

    const float* pX = &x.front();
    float* pY = &y.front();

    // Initialize estimation
    std::memcpy(pY, mpInitEst, sizeof(float) * mnClassCount);
    size_t nUsedTreeCount = 0;
    for(std::vector<Tree*>::const_iterator iterpTree = mvecpTrees.begin();
            iterpTree != mvecpTrees.end()/* && nUsedTreeCount < mnUsedTreeCount*/; ++iterpTree, ++nUsedTreeCount)
    {
        if(!*iterpTree || !(*iterpTree)->UpdateEstimation(pX, pY))
        {
            WARN("failed to update target when predict.");
            return false;
        }
    }
    // Transform if using logistic.
    if(mLossType == Loss::LOGISTIC)
    {
        common::utility::LogisticTransform(&y.front(), &y.front(), y.size());
    }
    return true;
}

// Spawn trees using data and configuration.
bool TreeBoost::spawnTrees(DataSet* pMatrix, const Configuration* pConf)
{
    LOG("====Preparing to build treeboost.====");
    mLossType = pConf->getType();
    // Create trees in the link.
    int nTreeCount = pConf->getTreeCount();
    mvecpTrees.clear();//free mvecpTrees?
    mvecpTrees.reserve(nTreeCount);

    int nClassCount = pMatrix->getClassCount();
    if(nClassCount >= 2 && pConf->getType() != Loss::LOGISTIC)
    {
        if(nClassCount >= 3)
        {
            ERROR("invalid distribution for target size %d: only logistic loss can do that.", nClassCount);
        }
        else
        {
            ERROR("invalid distribution for target size %d: change it to logistic or "
                 "adaboost (the target size must be 1 and the target must be {-1, 1} in this case).", nClassCount);
        }
        return false;
    }
    if(mLossType == Loss::ADABOOST && !pMatrix->IsTwoClassRegression())
    {
        ERROR("invalid distribution for target: target must be {-1, 1}.");
        return false;
    }
    if(mLossType == Loss::LOGISTIC && nClassCount < 2)
    {
        ERROR("invalid distribution for target: only {gaussian, laplcaian, huber} "
             "can do regression, or use adaboost.");
        return false;
    }
    // Initialize estimation.
    // F0
    LossFactory lossFactory;
    Loss* pLoss = lossFactory.make(pConf);
    if(!pLoss)
    {
        ERROR("failed to make loss: %d.");
        return false;
    }
    if(!pLoss->InitSampleMatrix(*pMatrix))
    {
        ERROR("failed to initialize sample matrix.");
        return false;
    }
    mnVariableCount = pMatrix->getVariableCount();
    if(nClassCount > mnClassCount)
    {
        if(mnClassCount > 0 && mpInitEst)
            delete [] mpInitEst;
        mpInitEst = new float [nClassCount];
    }
    mnClassCount = nClassCount;

    LOG("start to build treeboost.");

    if(!pLoss->InitEstimation(pMatrix, mpInitEst))
    {
        ERROR("failed to initialize estimation.");
        return false;
    }
    float fLastLoss10 = pLoss->CalculateLoss(pMatrix);
    float fLastLoss = fLastLoss10;

    // set pseudo-random seed.
    std::srand(2009);
    SampleSet* pSampleSet = new SampleSet(pMatrix);
    SampleSetManager* pManager = new SampleSetManager(pSampleSet);
    int nLastIter10 = -1;
    // Grow the trees using DataSet.
    for(int idxTree = 0; idxTree < nTreeCount; ++idxTree)
    {
        LOG("****Starting training tree: %d.****", idxTree + 1);

        // Subsample training samples.
        pSampleSet = pMatrix->StochasticSubsample(pManager, pConf->getSampleRate());

        if(pSampleSet == NULL)
        {
            ERROR("failed to subsample.");
            if(pLoss)
                delete pLoss;
            if(pManager)
                delete pManager;
            return false;
        }
        pSampleSet->setConfiguration(pConf);
        pSampleSet->setFastTrain(pConf->IsFastTrain());
        if(pSampleSet->getSampleCount() <= 0)
        {
            ERROR("empty sample subset, break training process.");
            break;
        }
        int nSampleCount = pSampleSet->getSampleCount();
        int nFullSampleCount = pSampleSet->getFullSampleCount();
        float fSampRate = (nFullSampleCount == 0) ? 0.0
                          : (static_cast<float>(nSampleCount) / nFullSampleCount * 100.0);
        LOG("subsample/Full = %d/%d (%.2f\%).", nSampleCount, nFullSampleCount, fSampRate);

        Tree* pTree = new Tree();
        if(!pTree->Grow(pSampleSet, pConf, pLoss))
        {
            delete pTree;
            WARN("failed to grow tree %d.", idxTree + 1);
            if(pLoss)
                delete pLoss;
            if(pManager)
                delete pManager;
            return mvecpTrees.size() > 0;
        }
        mvecpTrees.push_back(pTree);
        // delete pSampleSet;
        // Update sample matrix
        if(!pMatrix)
        {
            ERROR("failed to train tree: invalid DataSet.");
            std::cout << "  failed to train this tree." << std::endl;
            return false;
        }
        pLoss->UpdateSampleMatrix(pMatrix, pTree);
        float fLoss = pLoss->CalculateLoss(pMatrix);
        LOG("iteration[%d]:  loss = %f  gain = %f", idxTree, fLoss, fLastLoss - fLoss);
        fLastLoss = fLoss;

        // Print training status onto stdout.
        if(((idxTree + 1) % 10) == 0 || idxTree + 1 == nTreeCount)
        {
            std::cout << "iteration[" << nLastIter10 + 1 << "-" << idxTree << "]:";
            std::cout << "  loss = " << fLoss;
            std::cout << "  gain = " << fLastLoss10 - fLoss;
            std::cout << std::endl;
            fLastLoss10 = fLoss;
            nLastIter10 = idxTree;
        }

        LOG("****OK: succeeded to train tree: %d.****", idxTree + 1);
    } // for()


    LOG("succeeded to build TreeBoost.");

    if(pLoss)
        delete pLoss;
    if(pManager)
        delete pManager;
    return mvecpTrees.size() > 0;
} //spawnTrees()

float* TreeBoost::getInitEstimation() const
{
    return mpInitEst;
}

int TreeBoost::getClassCount() const
{
    return mnClassCount;
}

void TreeBoost::setClassCount(int nClassCount)
{
    if(nClassCount > mnClassCount)
    {
        if(mnClassCount > 0)
            if(mpInitEst)
                delete [] mpInitEst;
        mpInitEst = new float [nClassCount];
    }
    mnClassCount = nClassCount;
}

Tree* TreeBoost::getTree(int nTreeIndex) const
{
    if(size_t(nTreeIndex) < mvecpTrees.size())
        return mvecpTrees[size_t(nTreeIndex)];
    else
        return NULL;
}

int TreeBoost::getTreeCount() const
{
    return mvecpTrees.size();
}

void TreeBoost::setTreeCount(int nTreeCount)
{
    for(std::vector<Tree*>::const_iterator iterpTree = mvecpTrees.begin();
            iterpTree != mvecpTrees.end(); ++iterpTree)
    {
        if(*iterpTree)
            delete *iterpTree;
    }
    mvecpTrees.clear();
    if(size_t(nTreeCount) > 0)
    {
        mvecpTrees.resize(size_t(nTreeCount));
        for(std::vector<Tree*>::iterator iterpTree = mvecpTrees.begin();
                iterpTree != mvecpTrees.end(); ++iterpTree)
        {
            *iterpTree = new Tree();
        }
    }
}

Loss::Type TreeBoost::getType() const
{
    return mLossType;
}

void TreeBoost::setType(Loss::Type dist)
{
    mLossType = dist;
}

bool TreeBoost::analyzeVariableImportance(std::vector<float>& vecVarImp)
{
    if(mnClassCount <= 0)
    {
        ERROR("failed to do variable importance analyzation: iNvalid treeboost.");
        return false;
    }
    vecVarImp.resize(0);
    vecVarImp.resize(mnVariableCount, 0.0);
    size_t nUsedTreeCount = 0;
    for(std::vector<Tree*>::const_iterator iterpTree = mvecpTrees.begin();
            iterpTree != mvecpTrees.end();
            ++iterpTree, ++nUsedTreeCount)
    {
        if(!(*iterpTree) || !(*iterpTree)->UpdateVariableImportance(vecVarImp))
        {
            ERROR("failed to update variable importance.");
            return false;
        }
    }
    double fTotalVarImp = 0;
    for(std::vector<float>::iterator iterVarImp = vecVarImp.begin();
            iterVarImp != vecVarImp.end(); ++iterVarImp)
    {
        fTotalVarImp += *iterVarImp;
    }

    if(fTotalVarImp > 0)
    {
        for(std::vector<float>::iterator iterVarImp = vecVarImp.begin();
                iterVarImp != vecVarImp.end(); ++iterVarImp)
        {
            *iterVarImp /= fTotalVarImp;
        }
    }
    return true;
}

double TreeBoost::calculateLabelValue(const std::vector<float>& x) const
{
    double value = 0;
    size_t nUsedTreeCount = 0;
    for(std::vector<Tree*>::const_iterator tree = mvecpTrees.begin();
            tree != mvecpTrees.end();
            ++tree, ++nUsedTreeCount)
    {
        if(!*tree)
            throw std::runtime_error("Exception: TreeBoost::calculateLabelValue");
        value += (*tree)->calculateLabelValue(x);
    }
    return value;
}

int TreeBoost::getVariableCount() const
{
    return mnVariableCount;
}

void TreeBoost::setVariableCount(int nVariableCount)
{
    mnVariableCount = nVariableCount;
}

bool TreeBoost::setUsedTreeCount(size_t nUsedTreeCount)
{
    if(mvecpTrees.size() < nUsedTreeCount)
        return false;
    mnUsedTreeCount = nUsedTreeCount;
    return true;
}

} //treelink
} // mlplus

