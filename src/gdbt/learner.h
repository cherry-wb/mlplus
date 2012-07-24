#ifndef MLPLUS_TREELINK_LEARNER_H
#define MLPLUS_TREELINK_LEARNER_H

// The interface of the gdbt's Learner
namespace mlplus
{
namespace treelink
{
class DateSet;
class Configuration;
class Predictor;
class InstanceContainer
    class Learner
    {
    public:
        /**Construct a learner.
         * @param matrix Dataset which are used to train a predictor.
         * @param pConf A pointer to the configuration class object which stores the configuration for
         * the training process.
         */
        Learner(DateSet* matrix, Configuration* pConf);

        ~Learner();

        /**Train a predictor
         * @return A pointer to the predictor object. Notice that client code should delete the object
         * after usage. No maintainance is in the creating Learner object.
         */
        Predictor* train();
    protected:
    private:
        DateSet* mpDataset;
        Configuration* mpConf;
    };

/**The Learner factory. It encapsulate the details of creating a learner.
 */
class LearnerFactory
{
public:
    LearnerFactory(const std::string& dataHeader, const std::string& datafile, const std::string& strConfigFile);

    ~LearnerFactory();

    Learner* make() const;
private:
    const std::string mstrDataHeader;
    const std::string mstrDataFile;
    const std::string mstrConfigFile;
};

} // gdbt
} // mlplus

#endif

