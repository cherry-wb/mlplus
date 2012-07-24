//
// SampleSet is used to hold sample-selected-flags and temporary data in the computing precess.
//
#ifndef MLPLUS_TREELINK_SAMPLESET_H_
#define MLPLUS_TREELINK_SAMPLESET_H_


#include "sample_matrix.h"
#include "cut.h"

namespace mlplus
{
namespace treelink
{
class Loss;
class SampleMatrix;
class Configuration;
class SampleSetManager;
class SampleSet
{
public:

public:
    class Variable
    {
    public:
        class Iterator
        {
        public:
            Iterator(Variable* pVariable, int nPos);
            Iterator(const Iterator& iter);
            ~Iterator();
            const Iterator& operator = (const Iterator& iter);
            bool operator == (const Iterator& iter) const;
            bool operator != (const Iterator& iter) const;
            const Iterator& operator ++ ();
            const Iterator& operator ++ (int);
            float GetTarget() const;
            float* GetGradient();
            float* GetEstimation();
            float GetVariableDouble() const;
            int GetVariableInteger() const;
            float GetWeight() const;
            void SetWeight(float fWeight);
            inline size_t GetIndex() const;
        private:
            Variable* mpVariable;
            int mnCurrentPos;
        }; //Iterator
    public:
        Variable(SampleSet* pSampleSet, int nVariableIndex);
        ~Variable();
        Iterator& Begin();
        Iterator& End();
        SampleMatrix::VariableType GetVariableType() const;
        int GetClassCount() const;
        SampleSet* GetSampleSet() const;
        float GetLoss(const Loss* pLoss) const;
        bool GetWeighting() const;
        float GetWeight(int nPosition) const;
        void SetWeight(int nPosition, float fWeight);
        int GetSampleCount() const;
    private:
        const int* mpVariableOrder;
        SampleSet* mpSampleSet;
        SampleMatrix::VariableType mvariableType;
        int mnVariableIndex;
        Iterator mbegin;
        Iterator mend;
    }; // Variable

    class Iterator
    {
    public:
        Iterator(SampleSet* pSampleSet, int nPos);
        Iterator(const Iterator& iter);
        ~Iterator();
        const Iterator& operator = (const Iterator& iter);
        bool operator == (const Iterator& iter) const;
        bool operator != (const Iterator& iter) const;
        const Iterator& operator ++ ();
        const Iterator& operator ++ (int);
        float GetTarget() const;
        float* GetEstimation();
        float* GetGradient();
        float GetWeight() const;
        void SetWeight(float fWeight);
        float GetOriginalWeight() const;
        inline size_t GetIndex() const;
    private:
        int mnCurrentPos;
        SampleSet* mpSampleSet;
    }; //Iterator

public:
    SampleSet(SampleMatrix* pMatrix);
    SampleSet(SampleSet& sampSet);
    ~SampleSet();
    void SetConfiguration(const Configuration* pConf);
    float GetTarget(int nSampleIndex) const;
    Variable* GetVariable(int nVariableIndex);
    SampleMatrix::VariableType GetVariableType(int nVariableIndex) const;
    Cut* CalculateBestCut(const Loss* pLoss);
    int GetClassCount() const;
    Iterator& Begin();
    Iterator& End();
    int GetFullSampleCount() const;
    int GetSampleCount() const;
    int GetVariableCount() const;
    void SetSampleCount(int nSampleCount);
    bool Split(const Cut* pCut, SampleSet*& pLeftSampleSet, SampleSet*& pRightSampleSet);
    bool* GetSelect();
    bool GetWeighting() const;
    float GetWeight(int nSampleIndex) const;
    void SetWeight(int nSampleIndex, float fWeight);
    SampleMatrix* GetSampleMatrix();
    float GetOriginalWeight(int nSampleIndex) const;
    float* GetEstimation(int nSampleIndex);
    float* GetGradient(int nSampleIndex);
    float GetVariableValue(int nSampleIndex, int nVariableIndex) const;
    SampleSetManager* GetManager();
    void SetManager(SampleSetManager* pManager);
    bool CopyFrom(SampleSet& sampleSet);
    bool CopyFrom(SampleMatrix* pMatrix);
    void SetFastTrain(bool bFastTrain);
    bool CalculateCutGain(const Loss* pLoss, Cut* pCut);
    float* GetAllGradient() const;
    const float* GetAllWeight() const;
protected:
private:
    bool CalculateVariableCut(int nVariableIndex, const double* pGradSum, int nTotalCount,
                              double fTotalWeight, Cut* pCut);
    void CalculateGradSum(double* pGradSum, int& nTotalCount, double& fTotalWeight);
    bool CalculateVariableBest(int nVariableIndex, double* pLeftGradSum, double* pRightGradSum,
                               int nTotalCount, double fTotalWeight, float& fBestVar, float& fBestDist);
    bool CalculateVariableBest(int nVariableIndex, double* pLeftGradSum, double* pRightGradSum,
                               int nTotalCount, double fTotalWeight, int& nBestVar, float& fBestDist);
    bool CalculateVariableBest(int nVariableIndex, double* pLeftGradSum, double* pRightGradSum,
                               int nTotalCount, double fTotalWeight, std::set<int>& setBestVar, float& fBestDist);
    template <typename T1, typename T2>
    inline void AddVector(T1* dst, const T2* src, int nSize) const;
    template <typename T1, typename T2>
    inline void MinusVector(T1* dst, const T2* src, int nSize) const;
    template <typename T1>
    inline void ResetVector(T1* dst, int nSize) const;
    template <typename T1, typename T2>
    inline void CopyVector(T1* dst, const T2* src, int nSize) const;
    template <typename T1, typename T2>
    inline double DotVector(const T1* first, const T2* second, int nSize) const;
    SampleMatrix* mpSampleMatrix;
    float mfLoss;
    int mnSampleCount;
    float* mpWeight;
    int mnFullSampleCount;
    bool* mpSelect;
    float** mppEstimation;
    float* mpEstimation;
    float** mppGradient;
    float* mpGradient;
    int mnClassCount;
    const Configuration* mpConf;
    SampleSetManager* mpManager;
    Iterator mbegin;
    Iterator mend;
    bool mbRoot;
    bool mbFastTrain;
}; // SampleSet
template <typename T1, typename T2>
inline void SampleSet::AddVector(T1* dst, const T2* src, int nSize) const
{
    for(int i = 0; i < nSize; ++i)
    {
        dst[i] += src[i];
    }
}

template <typename T1, typename T2>
inline void SampleSet::MinusVector(T1* dst, const T2* src, int nSize) const
{
    for(int i = 0; i < nSize; ++i)
    {
        dst[i] -= src[i];
    }
}

template <typename T1>
inline void SampleSet::ResetVector(T1* dst, int nSize) const
{
    for(int i = 0; i < nSize; ++i)
    {
        dst[i] = 0;
    }
}

template <typename T1, typename T2>
inline void SampleSet::CopyVector(T1* dst, const T2* src, int nSize) const
{
    for(int i = 0; i < nSize; ++i)
    {
        dst[i] = src[i];
    }
}

template <typename T1, typename T2>
inline double SampleSet::DotVector(const T1* first, const T2* second, int nSize) const
{
    double fDotProd = 0;
    for(int i = 0; i < nSize; ++i)
    {
        fDotProd += first[i] * second[i];
    }
    return fDotProd;
}

inline size_t SampleSet::Variable::Iterator::GetIndex() const
{
    return size_t(mnCurrentPos);
}

inline size_t SampleSet::Iterator::GetIndex() const
{
    return size_t(mnCurrentPos);
}

} // treelink
} // mlplus
#endif // MLLIB_TREELINK_SAMPLESET_H_

