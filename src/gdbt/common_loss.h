#ifndef MLPLUS_TREELINK_LOSS_H_
#define MLPLUS_TREELINK_LOSS_H_
#include "sample_set.h"

namespace mlplus
{
namespace treelink
{
//==========================================================================
class Configuration;
class SampleMatrix;
class SampleSet;
class Cut;
class Tree;
class Loss
{
public:
    enum Type
    {
        UNKNOWN = 0,
        ADABOOST,
        LOGISTIC,
        GAUSSIAN,
        LAPLACIAN,
        HUBER,
        PAIRWISE_LS
    };

public:
    Loss(const Configuration* pConf): mpConf(pConf)
    {
    }
    virtual ~Loss()
    {
    }
    /**initialize sample matrix.
     * this procedure is specified by concrete loss implementations.
     * there is a default implementation in Loss class.
     * @param sampMat reference to a SampleMatrix object.
     * @return whether this procedure is successful.
     */
    virtual bool InitSampleMatrix(SampleMatrix& sampMat) const;
    virtual bool InitEstimation(SampleMatrix* pSampleMatrix, float* pInitEst) const;
    virtual float CalculateLoss(SampleMatrix* pSampMat) const;
    virtual bool CalculateGradient(SampleSet* pSampleSet) const;
    virtual bool CalculateOptimal(SampleSet::Variable* pVariable, Cut* pCut) const;
    virtual bool CalculateOptimal(SampleSet* pSampleSet, Cut* pCut) const;
    virtual bool UpdateSampleSet(SampleSet* pSampleSet, const float* pIncrement) const;
    virtual bool UpdateSampleMatrix(SampleMatrix* pSampleMatrix, const Tree* pTree) const;
    virtual std::string GetLossName() const;
protected:
    const Configuration* mpConf;
};
// Concrete class: Gaussian, Squre loss function
// loss-function = (y - y^)^2
class Gaussian: public Loss
{
public:
    Gaussian(const Configuration* pConf);
    ~Gaussian();
    bool InitEstimation(SampleMatrix* pSampleMatrix, float* pInitEst) const;
    float CalculateLoss(SampleMatrix* pSampMat) const;
    bool CalculateGradient(SampleSet* pSampleSet) const;
    bool CalculateOptimal(SampleSet::Variable* pVariable, Cut* pCut) const;
    bool CalculateOptimal(SampleSet* pSampleSet, Cut* pCut) const;
    bool UpdateSampleSet(SampleSet* pSampleSet, const float* pIncrement) const;
    bool UpdateSampleMatrix(SampleMatrix* pSampleMatrix, const Tree* pTree) const;
    std::string GetLossName() const;
private:
}; // Gaussian

class Huber: public Loss
{
public:
    Huber(const Configuration* pConf);
    ~Huber();
    bool InitEstimation(SampleMatrix* pSampleMatrix, float* pInitEst) const;
    float CalculateLoss(SampleMatrix* pSampMat) const;
    bool CalculateGradient(SampleSet* pSampleSet) const;
    bool CalculateOptimal(SampleSet::Variable* pVariable, Cut* pCut) const;
    bool CalculateOptimal(SampleSet* pSampleSet, Cut* pCut) const;
    bool UpdateSampleSet(SampleSet* pSampleSet, const float* pIncrement) const;
    bool UpdateSampleMatrix(SampleMatrix* pSampleMatrix, const Tree* pTree) const;
    std::string GetLossName() const;
private:
    bool CalculateIncrement(SampleSet::Variable* pVariable, Cut* pCut,
                            float& fLeftIncrement, float& fRightIncrement) const;
    bool CalculateGain(SampleSet::Variable* pVariable, Cut* pCut) const;
    float mfQuantile;
}; // Huber

// Concrete class: Logistic
class Logistic: public Loss
{

public:
    Logistic(const Configuration* pConf);
    ~Logistic();
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
}; // Logistic

// Concrete class: PairwiseLs
class PairwiseLs: public Loss
{

public:
    PairwiseLs(const Configuration* pConf);
    ~PairwiseLs();
    /* override */
    bool InitSampleMatrix(SampleMatrix& sampMat) const;
    /* override */
    bool InitEstimation(SampleMatrix* pSampleMatrix, float* pInitEst) const;
    /* override */
    float CalculateLoss(SampleMatrix* pSampMat) const;
    /* override */
    bool CalculateGradient(SampleSet* pSampleSet) const;
    /* override */
    bool CalculateOptimal(SampleSet::Variable* pVariable, Cut* pCut) const;
    /* override */
    bool CalculateOptimal(SampleSet* pSampleSet, Cut* pCut) const;
    /* override */
    bool UpdateSampleSet(SampleSet* pSampleSet, const float* pIncrement) const;
    /* override */
    bool UpdateSampleMatrix(SampleMatrix* pSampleMatrix, const Tree* pTree) const;
    /* override */
    std::string GetLossName() const;
private:
    double mfMargin;
//    bool CalculateIncrement(SampleSet::Variable* pVariable, Cut* pCut) const;
    bool CalculateGain(SampleSet::Variable* pVariable, Cut* pCut) const;
}; // PairwiseLs

// Concrete class: Laplacian
// loss-function = |y - y^|
class Laplacian: public Loss
{

public:
    Laplacian(const Configuration* pConf);
    ~Laplacian();
    bool InitEstimation(SampleMatrix* pSampleMatrix, float* pInitEst) const;
    float CalculateLoss(SampleMatrix* pSampMat) const;
    bool CalculateGradient(SampleSet* pSampleSet) const;
    bool CalculateOptimal(SampleSet::Variable* pVariable, Cut* pCut) const;
    bool CalculateOptimal(SampleSet* pSampleSet, Cut* pCut) const;
    bool UpdateSampleSet(SampleSet* pSampleSet, const float* pIncrement) const;
    bool UpdateSampleMatrix(SampleMatrix* pSampleMatrix, const Tree* pTree) const;
    std::string GetLossName() const;
private:
}; // Laplacian

////////////////////////////////////////////////////////////////////////////
class LossFactory
{

public:
    LossFactory()
    {
    }

    ~LossFactory()
    {
    }
    Loss* Make(const Configuration* pConf);
};

} // treelink
} // mlplus
#endif

