#ifndef MLPLUS_CLASSIFIERS_BAYES_MESSAGE_PASSING_H
#define MLPLUS_CLASSIFIERS_BAYES_MESSAGE_PASSING_H 
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include "instance.h"
#include "dataset.h"
#include "classifier.h"
#include "estimators/estimator.h"
using namespace mlplus::estimators;
namespace mlplus
{
class BayesMsgPassing: public Classifier
{
public:
    typedef int AttributeIndex;
    typedef Estimator* EstimatorPtr;
    //P(Class|Word)
    typedef EstimatorPtr PosteriorProbability; 
    typedef std::map<AttributeIndex,  PosteriorProbability>  DistributionMapType;
private:
    DistributionMapType mDistributions;
    EstimatorPtr mClassDistribution;
    int mClassesCount;
    void release();
    BayesMsgPassing(const BayesMsgPassing& bas);
public:
    BayesMsgPassing(const string& name, int nClasses);
    virtual ~BayesMsgPassing();
    inline EstimatorPtr getClassDistribution() const;
    inline void setClassDistribution(EstimatorPtr est);
    inline void setEventModel(bool v = true);
    inline bool getEventModel() const;
    virtual void load(istream& input);
    virtual void save(ostream& output);
    virtual void train(DataSet* data);
    virtual std::pair<int, double> predict(IInstance* i);
    virtual void update(IInstance* instance);
    virtual std::vector<double> targetDistribution(IInstance* i);
private:
    std::vector<double> sigmoidProb(const std::vector<double>& score);

};

inline  BayesMsgPassing::EstimatorPtr BayesMsgPassing::getClassDistribution(void) const
{
    return mClassDistribution;
}

inline void BayesMsgPassing::setClassDistribution(EstimatorPtr est)
{
    if (mClassDistribution)
    {
        delete mClassDistribution;
    }
    mClassDistribution = est;
}
}
#endif
