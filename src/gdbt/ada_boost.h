
#ifndef MLPLUS_TREELINK_ADABOOST_H_
#define MLPLUS_TREELINK_ADABOOST_H_

#include "loss.h"

namespace mlplus
{
namespace treelink
{
// Concrete class: AdaBoost
// loss-function = exp(-y*y^)
class AdaBoost: public Loss
{

public:
    AdaBoost(const Configuration* pConf);
    ~AdaBoost();
    bool InitEstimation(SampleMatrix* pSampleMatrix, float* pInitEst) const;
    float CalculateLoss(SampleMatrix* pSampMat) const;
    bool CalculateGradient(SampleSet* pSampleSet) const;
    bool CalculateOptimal(SampleSet::Variable* pVariable, Cut* pCut) const;
    bool CalculateOptimal(SampleSet* pSampleSet, Cut* pCut) const;
    bool UpdateSampleSet(SampleSet* pSampleSet, const float* pIncrement) const;
    bool UpdateSampleMatrix(SampleMatrix* pSampleMatrix, const Tree* pTree) const;
    std::string GetLossName() const;
private:
    bool CalculateIncrement(SampleSet::Variable* pVariable, Cut* pCut) const;
    bool CalculateGain(SampleSet::Variable* pVariable, Cut* pCut) const;
    float Log(float fPositive, float fNegative) const;
}; // AdaBoost

}
}
#endif

