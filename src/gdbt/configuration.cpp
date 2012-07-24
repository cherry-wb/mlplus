#include <string>
#include "mllib/interface/mllib_conf.h"
#include "configuration.h"
#include "loss.h"
#include "model_manager.h"
#include "mllib/interface/log.h"

namespace mllib
{
namespace treelink
{

Configuration::Configuration(const std::string &strConfFile)
    : mnTreeCount(0), mnMaxLeafCount(0), mLossType(Loss::UNKNOWN),
      mfShrinkage(0.0), mfSampleRate(0.0), mbFastTrain(false),
      mfQuantileRate(0.0),
      mpConfigParser(NULL), msepType(Cut::UNKNOWN), mfMargin(0),
      mfVariableSampleRate(0.0), mfSplitBalance(0.0), mnMaxTreeDepth(0),
      mnMinLeafSampleCount(0), mbSingleList(false)
{
    mpConfigParser = new mllib::MlPlainTextParser(strConfFile);
}

Configuration::~Configuration()
{
    if(mpConfigParser)
        delete mpConfigParser;
}

// Parse for parameters & validate them.
bool Configuration::ParseConfiguration()
{
    if(!mpConfigParser->Parse())
        return false;

    // get all parameters.
    // Tree count.
    if(!mpConfigParser->getItem(std::string("treelink"), std::string("tree_count"), mnTreeCount)
            || mnTreeCount <= 0)
    {
        TERR("invalid TreeLink configuration parameter: tree_count. it should be greater than 0.");
        return false;
    }

    // Maximum leaf count
    if(!mpConfigParser->getItem(std::string("treelink"), std::string("max_leaf_count"), mnMaxLeafCount)
            || mnMaxLeafCount <= 0)
    {
        TERR("invalid TreeLink configuration parameter: max_leaf_count. it should be greater than 0.");
        return false;
    }

    if(!mpConfigParser->getItem(std::string("treelink"), std::string("max_tree_depth"), mnMaxTreeDepth)
            || mnMaxTreeDepth <= 0)
    {
        TERR("invalid TreeLink configuration parameter: max_tree_depth. it should be greater than 0.");
        return false;
    }

    std::string strParam;

    // get distribution.
    if(!mpConfigParser->getItem(std::string("treelink"), std::string("loss_type"), strParam))
    {
        // get distribution.
        TERR("failed to get treelink parameter: loss_type.");
        return false;
    }
    if(strParam == "gaussian")
        mLossType = Loss::GAUSSIAN;
    else if(strParam == "laplacian")
        mLossType = Loss::LAPLACIAN;
    else if(strParam == "huber")
        mLossType = Loss::HUBER;
    else if(strParam == "adaboost")
        mLossType = Loss::ADABOOST;
    else if(strParam == "logistic")
        mLossType = Loss::LOGISTIC;
    else if(strParam == "pairwise_ls")
        mLossType = Loss::PAIRWISE_LS;
    else
    {
        TERR("invalid TreeLink configuration parameter: distribution. it should be gaussian, "
             "laplacian, huber, adaboost or logistic.");
        return false;
    }

    // get Shrinkage.
    if(!mpConfigParser->getItem(std::string("treelink"), std::string("shrinkage"), mfShrinkage)
            || mfShrinkage <= 0.0 || mfShrinkage > 1.0)
    {
        TERR("invalid TreeLink configuration parameter: shrinkage."
             " it should be greater than 0 but less than or equal to 1.");
        return false;
    }

    // get sample rate.
    if(!mpConfigParser->getItem(std::string("treelink"), std::string("sample_rate"), mfSampleRate)
            || mfSampleRate <= 0.0 || mfSampleRate > 1.0)
    {
        TERR("invalid TreeLink configuration parameter: sample_rate."
             " it should be greater than 0 but less than or equal to 1.");
        return false;
    }

    // get variable sample rate.
    if(!mpConfigParser->getItem(std::string("treelink"),
                                std::string("variable_sample_rate"), mfVariableSampleRate)
            || mfVariableSampleRate <= 0.0 || mfVariableSampleRate > 1.0)
    {
        TERR("invalid TreeLink configuration parameter: variable_sample_rate."
             " it should be greater than 0 but less than or equal to 1.");
        return false;
    }

    if(!mpConfigParser->getItem(std::string("treelink"), std::string("split_balance"),
                                mfSplitBalance) || mfSplitBalance < 0.0 || mfSplitBalance > 0.5)
    {
        TERR("invalid treelink configuration parameter: split_balance."
             " it should be greater than 0 but less or equal to 0.5.");
        return false;
    }

    if(!mpConfigParser->getItem(std::string("treelink"), std::string("min_leaf_sample_count"),
                                mnMinLeafSampleCount) || mnMinLeafSampleCount <= 0)
    {
        TERR("invalid treelink configuration parameter: min_leaf_sample_count."
             " it should be greater than 0");
        return false;
    }

    if(!mpConfigParser->getItem(std::string("treelink"), std::string("fast_train"), mbFastTrain))
    {
        TERR("invalid TreeLink configuration parameter: fast_train.");
        return false;
    }

    // get discrete cut type
    if(!mpConfigParser->getItem(std::string("treelink"),
                                std::string("discrete_separator_type"), strParam))
    {
        // get distribution.
        TERR("failed to get treelink parameter: discrete_separator_type."
             " it should be leave_one_out or arbitrary.");
        return false;
    }
    if(strParam == "leave_one_out")
        msepType = Cut::LEAVE_ONE_OUT;
    else if(strParam == "arbitrary")
        msepType = Cut::ARBITRARY;
    else
    {
        TERR("invalid treelink configuration parameter: discrete_separator_type."
             " it should be leave_one_out or arbitrary.");
        return false;
    }

    // get special parameter.
    if(mLossType == Loss::HUBER)
    {
        if(!mpConfigParser->getItem(std::string("treelink"), std::string("quantile_rate"), mfQuantileRate)
                || mfQuantileRate <= 0.0 || mfQuantileRate > 1.0)
        {
            TERR("invalid TreeLink configration parameter: quantile_rate (for huber loss)."
                 " it should be greater than or 0 but less than or equal to 1.");
            return false;
        }
    }

    if(mLossType == Loss::PAIRWISE_LS)
    {
        if(!mpConfigParser->getItem(std::string("treelink"), std::string("margin"), mfMargin)
                || mfMargin <= 0 || mfMargin > 1)
        {
            TERR("invalid treelink configuration parameter: margin (for pairwise_ls loss)."
                 "it should be in range (0, 1]");
            return false;
        }
        if(!mpConfigParser->getItem(std::string("treelink"), std::string("single_list"), mbSingleList))
        {
            TERR("invalid treelink configuration parameter: single_list (for pairwise_ls loss).");
            return false;
        }
    }

    TLOG("succeeded to load TreeLink configuration parameters.");

    // log parameters
    TLOG("====treelink training parameters:");
    TLOG("loss type:                    %s", getTypeName().c_str());
    TLOG("tree count:                   %d", mnTreeCount);
    TLOG("max leaf count:               %d", mnMaxLeafCount);
    TLOG("shrinkage:                    %f", mfShrinkage);
    TLOG("subsample rate:               %f", mfSampleRate);
    TLOG("variable subsample rate:      %f", mfVariableSampleRate);
    TLOG("split balance:                %f", mfSplitBalance);
    TLOG("max tree depth:               %d", mnMaxTreeDepth);
    TLOG("min leaf sample count:        %d", mnMinLeafSampleCount);
    TLOG("fast_train:                   %s", mbFastTrain ? "true" : "false");
    if(mLossType == Loss::HUBER)
    {
        TLOG("huber quantile:               %f", mfQuantileRate);
    }
    else if(mLossType == Loss::PAIRWISE_LS)
    {
        TLOG("margin:                       %f", mfMargin);
        TLOG("single_list:                  %s", (mbSingleList ? "true" : "false"));
    }
    TLOG("====end====");
    return true;
}

int Configuration::getTreeCount() const
{
    return mnTreeCount;
}

int Configuration::getMaxLeafCount() const
{
    return mnMaxLeafCount;
}

Loss::Type Configuration::getType() const
{
    return mLossType;
}

std::string Configuration::getTypeName() const
{
    switch(mLossType)
    {
    case Loss::ADABOOST:
        return "adaboost";
    case Loss::LOGISTIC:
        return "logistic";
    case Loss::GAUSSIAN:
        return "gaussian";
    case Loss::LAPLACIAN:
        return "laplacian";
    case Loss::HUBER:
        return "huber";
    case Loss::PAIRWISE_LS:
        return "pairwise_ls";
    default:
        return "*fault*";
    }
    return "*fault*";
}

double Configuration::getShrinkage() const
{
    return mfShrinkage;
}

double Configuration::getSampleRate() const
{
    return mfSampleRate;
}

double Configuration::getQuantileRate() const
{
    return mfQuantileRate;
}

Cut::DiscreteSeparatorType Configuration::getDiscreteSeparatorType() const
{
    return msepType;
}

bool Configuration::IsFastTrain() const
{
    return mbFastTrain;
}

double Configuration::getMargin() const
{
    return mfMargin;
}

double Configuration::getVariableSampleRate() const
{
    return mfVariableSampleRate;
}

double Configuration::getSplitBalance() const
{
    return mfSplitBalance;
}

int Configuration::getMaxTreeDepth() const
{
    return mnMaxTreeDepth;
}

int Configuration::getMinLeafSampleCount() const
{
    return mnMinLeafSampleCount;
}

bool Configuration::isSingleList() const
{
    return mbSingleList;
}

} // treelink
} // mllib

