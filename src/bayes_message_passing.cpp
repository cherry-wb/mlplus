#include "bayes_message_passing.h"
#include "instance.h"
#include "dataset.h"
#include "classifier.h"
#include "iterator_interface.h"
#include "attribute_value.h"
#include "string_utility.h"
#include <estimators/estimator_include.h>
#include <stdexcept>
#include <algorithm>
using namespace std;
//http://en.wikipedia.org/wiki/Bayesian_spam_filtering
namespace mlplus
{
BayesMsgPassing::BayesMsgPassing(const string& name, int nClasses):
    Classifier(name), mClassDistribution(NULL), mClassesCount(nClasses)
{
}
void BayesMsgPassing::release()
{
    std::map<AttributeIndex,  PosteriorProbability>::iterator it = mDistributions.begin();
    for(; it != mDistributions.end(); ++it)
    {
        PosteriorProbability& pp = it->second;
        if (pp != NULL)
        {
            delete pp;
            pp = NULL;
        }
    }
    if (mClassDistribution != NULL)
    {
        delete mClassDistribution;
        mClassDistribution = NULL;
    }
}
BayesMsgPassing::~BayesMsgPassing()
{
    release();
}

void BayesMsgPassing::train(DataSet* dataset)
{
    assert(dataset != NULL);
    //mClassesCount = dataset->numTargets();
    //int attributeCount  = dataset->numAttributes();
    setClassDistribution(new DiscreteEstimator(mClassesCount));
    int attIndex = 0;
    AutoAttributeIteratorPtr  attrIt(dataset->newAttributeIterator());
    int targetIndex = dataset->targetIndex();
    while(attrIt->hasMore())
    {
        Attribute* attr = attrIt->next();
        attIndex = attr->getIndex();
        if (attIndex == targetIndex)
        {
            continue;
        }
        PosteriorProbability& pp =  mDistributions[attIndex];
        if (pp == NULL)
        {
            pp = new DiscreteEstimator(mClassesCount, false);//P(Class|word)
        }
    }
    AutoInstanceIteratorPtr instanceIt(dataset->newInstanceIterator());
    while(instanceIt->hasMore())
    {
        update(instanceIt->next());
    }
    DistributionMapType::iterator it = mDistributions.begin();
    for (; it != mDistributions.end(); ++it)
    {
        it->second->smoothing(mClassDistribution, mClassesCount);
    }
}

void  BayesMsgPassing::update(IInstance* instance)
{
    if(instance->targetIsMissing())
        throw runtime_error("missing class in instance");
    int targetValue = (int)instance->targetValue();
    int targetIndex = instance->targetIndex();
    float tfAll = 0;
    int numAttr = instance->numAttributes();
    for (int i = 0; i < numAttr; ++i)
    {
        int aIndex = instance->attributeIndex(i);
        if (aIndex == targetIndex)
        {
            continue;
        }
        if (mDistributions.find(aIndex) == mDistributions.end()) 
        {
            continue;
        }
        PosteriorProbability& pp =  mDistributions[aIndex];
        ValueType value = instance->getValue(aIndex);
        if (!AttributeValue::isMissingValue(value))
        {
            pp->addValue(targetValue, value*instance->getWeight());
            tfAll += value * instance->getWeight();
        }
    }

    mClassDistribution->addValue(targetValue, tfAll);
}

std::vector<double> BayesMsgPassing::targetDistribution(IInstance* instance)
{
    vector<double> v(mClassesCount,0);
    for(int i = 0; i < mClassesCount; i++)
    {
        //v[i] = log(mClassDistribution->getProbability(i));
    }
    int numAttr = instance->numAttributes();
    int targetIndex = instance->targetIndex();

    for (int i = 0; i < numAttr; ++i)
    {
        int aIndex = instance->attributeIndex(i);
        if (aIndex == targetIndex)
        {
            continue;
        }
        DistributionMapType::iterator it = mDistributions.find(aIndex);
        if (it == mDistributions.end()) 
        {
            continue;
        }
        PosteriorProbability& pp = it->second;
        ValueType aValue = instance->getValue(aIndex);
        if(!AttributeValue::isMissingValue(aValue))
        {
            double temp = 0;
            for(int j = 0; j < mClassesCount; j++)
            {
                temp = pp->getProbability(j);
                v[j] += (log(temp) - log(1-temp)) * aValue;
            }
        }
    }
    return sigmoidProb(v);
}

vector<double> BayesMsgPassing::sigmoidProb(const vector<double>& score)
{
    int class_set_size = score.size();
    vector<double> prb(class_set_size, 0);
    for (int i = 0; i < class_set_size; i++) 
    {
        prb[i] = 1/(1 + exp(-score[i]));
    }
    //normalize
    double sum;
    for (int i = 0; i < class_set_size; ++i)
    {
        sum += prb[i];
    }
    for (int i = 0; i < class_set_size; ++i)
    {
        prb[i] /= sum;
    }
    return prb;
}

void BayesMsgPassing::load(istream& input)
{
    string line;
    vector<string> result;
    //class count
    if (getline(input,line))
    {
        split(line,result,"=");
        mClassesCount = atoi(result[1].c_str());
        setClassDistribution(new DiscreteEstimator(mClassesCount));
    }
    //class distribution
    if (getline(input,line))
    {
        split(line,result,"=");
        mClassDistribution->fromString(result.back());
    }
    //attribues
    while(getline(input, line))
    {
        if ('#' == line[0])
        {
            continue;
        }
        vector<string> result;
        split(line, result, ",");
        if (2 == result.size())
        {
            int attIndex = atoi(result[0].c_str());
            PosteriorProbability& pp =  mDistributions[attIndex];
            if (pp == NULL)
            {
                pp = new DiscreteEstimator(mClassesCount);
                pp->fromString(result[1]);
            }
        }
    }
}
void BayesMsgPassing::save(ostream& output)
{
    DistributionMapType::iterator it = mDistributions.begin();
    output << "class_count=" << mClassesCount << "\n";
    output << "class_dist=" <<  mClassDistribution->toString() << "\n";
    output << "#feature_count=" << mDistributions.size()<< "\n";
    output << "#featureid , targetid, estimator_id, estimator_string\n";
    for(; it != mDistributions.end(); ++it)
    {
        PosteriorProbability& pp = it->second;
        if (pp != NULL)
        {
            //feature id
            output << it->first << "," << pp->toString() << "\n";
        }
    }
}
pair<int, double> BayesMsgPassing::predict(IInstance* i) 
{
    vector<double> array = targetDistribution(i);
    vector<double>::const_iterator it = max_element(array.begin(), array.end());
    return make_pair(it - array.begin(), *it);
}
} // namespace mlplus

