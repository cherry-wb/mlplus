//
// 2009-03-03 created by binqiang.zhao, implement by charles.wang
// this file implement the common distributions and their factory.
//
#include <cstring>
#include <cmath>
#include "mlplus/interface/log.h"
#include "loss.h"
#include "ada_boost.h"
#include "logistic.h"
#include "gaussian.h"
#include "laplacian.h"
#include "huber.h"
#include "pairwise_ls.h"
#include "configuration.h"
#include "cut.h"
#include "mlplus/common/utility.h"

namespace mlplus
{
namespace treelink
{

Loss* LossFactory::Make(const Configuration* pConf)
{
    switch(pConf->GetType())
    {
    case Loss::UNKNOWN:
        return NULL;
    case Loss::ADABOOST:
        return new AdaBoost(pConf);
    case Loss::LOGISTIC:
        return new Logistic(pConf);
    case Loss::GAUSSIAN:
        return new Gaussian(pConf);
    case Loss::LAPLACIAN:
        return new Laplacian(pConf);
    case Loss::HUBER:
        return new Huber(pConf);
    case Loss::PAIRWISE_LS:
        return new PairwiseLs(pConf);
    default:
        return NULL;
    }
    return NULL;
}

bool Loss::InitSampleMatrix(SampleMatrix& sampMat) const
{
    return true;
}

// Default estimation initializer.
// All estimations are zeros!
bool Loss::InitEstimation(SampleMatrix* pSampleMatrix, float* pInitEst) const
{
    // Initialize all the estimation to zero.
    int nClassCount = pSampleMatrix->GetClassCount();
    std::memset(pInitEst, 0, sizeof(float) * nClassCount);
    /*
    for (int idxClass = 0; idxClass < nClassCount; ++idxClass)
    {
        pInitEst[idxClass] = 0.0;
    }
    */

    for(int idxSample = 0; idxSample < pSampleMatrix->GetSampleCount(); ++idxSample)
    {
        float* pEstimation = pSampleMatrix->GetEstimation(idxSample);
        memcpy(pEstimation, pInitEst, sizeof(float) * nClassCount);
    }

    return true;
}

// Default gradient calculator.
// Don't use this gradient, because there's no common gradient for losses.
bool Loss::CalculateGradient(SampleSet* pSampleSet) const
{
    (void)pSampleSet;
    return false;
}

float Loss::CalculateLoss(SampleMatrix* pSampMat) const
{
    static_cast<void>(pSampMat);
    return 0.0;
}

// Default gain calculator
// Use this one if you want to use gradient to calculate increment & gain.
// Variable must be sorted ascendantly!
bool Loss::CalculateOptimal(SampleSet::Variable* pVariable, Cut* pCut) const
{
    static_cast<void>(pVariable);
    static_cast<void>(pCut);
    return false;
}

bool Loss::CalculateOptimal(SampleSet* pSampleSet, Cut* pCut) const
{
    static_cast<void>(pSampleSet);
    static_cast<void>(pCut);
    return false;
}

// Default UpdateSampleSet
// Many distributions share this method.
bool Loss::UpdateSampleSet(SampleSet* pSampleSet, const float* pIncrement) const
{
    int nClassCount = pSampleSet->GetClassCount();
    for(SampleSet::Iterator iterSample = pSampleSet->Begin(); iterSample != pSampleSet->End(); ++iterSample)
    {
        // Update Estimation
        float* pEstimation = iterSample.GetEstimation();
        for(int idxClass = 0; idxClass < nClassCount; ++idxClass)
        {
            pEstimation[idxClass] += pIncrement[idxClass];
        }
    }
    return true;

}

// Default UpdateSampleMatrix
// Many distributions share this method.
bool Loss::UpdateSampleMatrix(SampleMatrix* pSampleMatrix, const Tree* pTree) const
{
    int nSampleCount = pSampleMatrix->GetSampleCount();
    int nVariableCount = pSampleMatrix->GetVariableCount();
    int nClassCount = pSampleMatrix->GetClassCount();
    float* pVariable = new float [nVariableCount];
    float* pIncrement = new float [nClassCount];
    for(int idxSample = 0; idxSample < nSampleCount; ++idxSample)
    {
        // Load variables.
        if(!pSampleMatrix->GetVariableValue(idxSample, pVariable))
        {
            if(pVariable)
                delete [] pVariable;
            if(pIncrement)
                delete [] pIncrement;
            TDEBUG("failed to get variable from matrix.");
            return false;
        }

        // Get increment.
        std::memset(pIncrement, 0, sizeof(float) * nClassCount);
        if(!pTree->UpdateEstimation(pVariable, pIncrement))
        {
            TDEBUG("failed to get increment using tree.");
            if(pVariable)
                delete [] pVariable;
            if(pIncrement)
                delete [] pIncrement;
            return false;
        }

        // Add the increment to estimation.
        float* pEstimation = pSampleMatrix->GetEstimation(idxSample);
        for(int idxClass = 0; idxClass < nClassCount; ++idxClass)
        {
            float fOldEst = pEstimation[idxClass];
            float fNewEst = fOldEst + pIncrement[idxClass];
            pEstimation[idxClass] = fNewEst;
        }
    }
    if(pVariable)
        delete [] pVariable;
    if(pIncrement)
        delete [] pIncrement;
    return true;
}

std::string Loss::GetLossName() const
{
    return "unknown";
}

} // treelink
} // mlplus


