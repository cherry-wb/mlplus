#ifndef MLPLUS_DECISIONTREEH_H
#define MLPLUS_DECISIONTREEH_H
#include <vector>
namespace mlplus
{
typedef enum
{
    dtnLeaf,
    dtnContinuous,
    dtnDiscrete,
    dtnGrowing
} NodeType;
class DecisionTree;
typedef  DecisionTree* DecisionTreePtr;
class DecisionTree
{
private:
    NodeType mNodeType;
    DataSet* mAttributeSpec;
    void* mGrowingData;
    int mSplitAttribute;
    float mSplitThreshold;/* for continuous attributes */
    float mLower;		/* lower limit of soft threshold */
    float mUpper;		/* upper limit ditto */
    float mMid;		    /* midpoint for soft threshold */
    std::vector<DecisionTreePtr> mChildren;/* mChildren array*/
    DecisionTreePtr mParent;
	//Set         *Subset;	/* subsets of discrete values  */
    int mMyClass;
    float* mClassDistribution;
    float mDistributionInstanceCount;
public:
    DecisionTreePtr newTree(DataSet* spec);
    void free();
    DecisionTreePtr clone();
    int isLeaf();
    int isTreeGrowing();
    int isNodeGrowing();
    int getClass();
    void setClass(int theClass);
    void addToClassDistribution(IInstance* e);
    float getClassProb(int theClass);
    void  setClassProb(int theClass, float prob);
    void zeroClassDistribution();
    void setTypeLeaf();
    void setTypeGrowing();
    void splitOnDiscreteAttribute(int attNum);
    void slitOnContinuousAttribute(int attNum, float threshold);
    int getChildCount();
    DecisionTreePtr getChild(int index);
    DecisionTreePtr oneStepClassify(IInstance* e);
    int classify(IInstance* e);
    void growingNodes(VoidAListPtr list);
    void gatherLeaves(VoidAListPtr list);
    int  countNodes();
    int getMostCommonClass();
    void setGrowingData(void *data);
    void *getGrowingData();
    void print(ostream& out);
    void printStats(ostream& out);
    DecisionTreePtr readC5Bin(istream& in, DataSet* spec);
    DecisionTreePtr readC5Text(istream& in, DataSet* spec);
    DecisionTreePtr read(istream& in, DataSet* spec);
    void write(ostream& out);
private:
}
}
#endif /* DECISIONTREEH */
