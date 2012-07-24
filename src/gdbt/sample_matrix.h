//
// SampleMatrix: holds training data.
//
#ifndef MLLIB_TREELINK_SAMPLEMATRIX_H_
#define MLLIB_TREELINK_SAMPLEMATRIX_H_
#include "mlplus/mllib_data.h"
#include "preference.h"

namespace mlplus
{
namespace treelink
{
typedef mlplus::MlSample Sample;
typedef mlplus::MlSampleContainer SampleContainer;
class SampleSet;
class SampleSetManager;
class SampleMatrix
{
public:
    // enum for specifying data types.
    enum VariableType
    {
        UNKNOWN = 0,
        NOMINAL,
        REAL
    }; // VariableType

public:
    SampleMatrix(const SampleContainer* pContainer);
    ~SampleMatrix();

    bool Initialize();
    SampleSet* StochasticSubsample(SampleSetManager* pManager, float fSampleRate);
    int GetSampleCount() const;
    int GetClassCount() const;
    int GetVariableCount() const;
    VariableType GetVariableType(int nVariableIndex) const;
    float* GetEstimation(int nSampleIndex);
    float GetTarget(int nSampleIndex) const;
    float GetWeight(int nSampleIndex) const;
    void SetWeight(int nSampleIndex, float fWeight);
    float GetOriginalWeight(int nSampleIndex) const;
    const int* GetVariableOrder(int nVariableIndex) const;
    float GetVariableValue(int nSampleIndex, int nVariableIndex);
    void SetWeighting(bool bWeighting);
    bool GetWeighting() const;
    bool GetVariableValue(int nSampleIndex, float* pVariable) const;
    const float* GetAllEstimation() const;
    const float* GetAllWeight() const;
    bool IsTwoClassRegression() const;
    bool BuildPreference(bool bSingleList);
    const PreferenceSet& GetPreferenceSet() const;
protected:
private:
    bool BuildVariableOrder(int nVariableIndex);
    const SampleContainer* mpContainer;
    float** mppEstimation;
    float* mpEstimation;
    float* mpWeight;
    int** mppVariableOrder;
    int* mpVariableOrder;
    int mnSampleCount;
    int mnClassCount;
    int mnVariableCount;
    bool mbWeighting;
    PreferenceSet mPreferenceSet;
    size_t mGroupCount;
}; // SampleMatrix

} // treelink
} // mlplus
#endif //MLLIB_TREELINK_SAMPLEMATRIX_H_

