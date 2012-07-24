#ifndef MLLIB_TREELINK_SAMPLE_SET_MANAGER_H_
#define MLLIB_TREELINK_SAMPLE_SET_MANAGER_H_

#include <list>

namespace mlplus
{
namespace treelink
{
class SampleSet;
class SampleSetManager
{
public:
    SampleSetManager(SampleSet* pSampleSet);
    ~SampleSetManager();
    SampleSet* TakeSampleSet(SampleSet* pSampleSet);
    SampleSet* TakeSampleSet(SampleMatrix* pMatrix);
    bool PutSampleSet(SampleSet* pSampleSet);
private:
    std::list<SampleSet*> mlstUsed;
    std::list<SampleSet*> mlstAvail;
};

} // treelink
} // mlplus

#endif // MLLIB_TREELINK_SAMPLE_SET_MANAGER_H_

