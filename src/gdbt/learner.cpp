//
// Implementation of Learner interface.
//
#include <iostream>
#include "learner.h"
#include "predictor.h"
#include "dataset.h"
#include "configuration.h"
#include "mllib/common/utility.h"

namespace mllib
{
namespace treelink
{
Learner::Learner(DateSet* pDataSet, Configuration* pConf)
    : mpDataset(pDataSet), mpConf(pConf)
{
}

Learner::~Learner()
{
    if(mpDataset)
    {
        delete mpDataset;
    }
    if(mpConf)
    {
        delete mpConf;
    }
}

// Learning to create a predictor, which should be deleted outside after used.
Predictor* Learner::train()
{
    if(mpDataset == NULL || mpConf == NULL)
    {
        ERROR("invalid sample dataset or configuration.");
        return NULL;
    }
    // Print basic information about this training task.
    std::cout << "====basic information of this training task====" << std::endl;
    std::cout << "tree count:                 " << mpConf->getTreeCount() << std::endl;
    std::cout << "max leaf count:             " << mpConf->getMaxLeafCount() << std::endl;
    std::cout << "max tree depth:             " << mpConf->getMaxTreeDepth() << std::endl;
    std::cout << "loss type:                  " << mpConf->getTypeName() << std::endl;
    std::cout << "shrinkage:                  " << mpConf->getShrinkage() << std::endl;
    std::cout << "sample rate:                " << mpConf->getSampleRate() << std::endl;
    std::cout << "variable sample rate:       " << mpConf->getVariableSampleRate() << std::endl;
    std::cout << "split balance:              " << mpConf->getSplitBalance() << std::endl;
    std::cout << "minimum leaf sample count:  " << mpConf->getMinLeafSampleCount() << std::endl;
    std::cout << "use fast-train:             " << (mpConf->IsFastTrain() ? "true" : "false") << std::endl;
    std::cout << "nominal seperating method:  " << (mpConf->getDiscreteSeparatorType()
              == Cut::LEAVE_ONE_OUT ? "leave_one_out" : "arbitrary") << std::endl;
    if(mpConf->getType() == Loss::HUBER)
    {
        std::cout << "quantile rate (huber only): " << mpConf->getQuantileRate() << std::endl;
    }
    if(mpConf->getType() == Loss::PAIRWISE_LS)
    {
        std::cout << "margin (pairwise_ls only):  " << mpConf->getMargin() << std::endl;
        std::cout << "single_list (pairwise_ls only): "
                  << (mpConf->isSingleList() ? "true" : "false") << std::endl;
    }
    std::cout << std::endl;
    // Create a TreeBoost object to manage the learning process.
    TreeBoost* pTreeBoost = new TreeBoost();
    LOG("start to build trees.");
    std::cout << "====start to build TreeLink model.====" << std::endl;
    if(pTreeBoost->spawnTrees(mpDataset, mpConf))
    {
        //Spawn trees sucessfully.
        Predictor* pPredictor = new Predictor(pTreeBoost);    //Create a Predictor to encapsulate the link.
        LOG("build treelink model successfully.");
        std::cout << "====build treelink model successfully.====" << std::endl;
        return pPredictor;
    }
    else
    {
        ERROR("failed to build treeboost.");
        if(pTreeBoost)
            delete pTreeBoost;
        return NULL;
    } // spawnTrees
} //Learner::Train()

LearnerFactory(const std::string& dataHeader, const std::string& datafile, const std::string& strConfigFile)
    : mstrDataHeader(dataHeader), mstrDataFile(datafile), mstrConfigFile(strConfigFile)
{
}

LearnerFactory::~LearnerFactory()
{
}

Learner* LearnerFactory::make() const
{
    /*
    Configuration* pConf = new Configuration(mstrConfigFile);
    if (!pConf->parseConfiguration())
    {
        ERROR("failed to parse treelink configuration file.");
        if (pConf)
            delete pConf;
        return NULL;
    }

    DateSet* pSampleMatrix = new DateSet(mpContainer);
    if (!pSampleMatrix->initialize())
    {
        ERROR("failed to initialize DateSet.");
        if (pConf)
            delete pConf;
        if (pSampleMatrix)
            delete pSampleMatrix;
        return NULL;
    }
    return new Learner(pSampleMatrix, pConf);
    */
    return NULL;
}

} //treelink
} // mllib

