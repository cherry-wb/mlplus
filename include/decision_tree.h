#ifndef MLPLUS_DECISIONTREEH_H
#define MLPLUS_DECISIONTREEH_H
#include <vector>
#include <list>
#include <iostream>
namespace mlplus
{

typedef enum
{
    dtnLeaf,
    dtnDiscrete,
    dtnContinuous,
    dtnSubset,
    dtnGrowing
} TreeNodeType;
class AttributeSpec;
class IInstance;
class Attribute;
class DecisionTree;
class BoostDecisionTree; 
typedef  DecisionTree* DecisionTreePtr;
class DecisionTree
{
private:
    typedef int64_t Set64;
    TreeNodeType mNodeType;
    AttributeSpec* mAttributeSpec;
    void* mGrowingData;
    int mSplitAttribute;
    float mSplitThreshold;/* for continuous attributes */
    float mLower;		/* lower limit of soft threshold */
    float mUpper;		/* upper limit ditto */
    float mMid;		    /* midpoint for soft threshold */
    int mForks;
    DecisionTreePtr *mChildren;/* mChildren array*/
    Set64       *mSubset;	   /* subsets of discrete values  */
    int mMyClass;
    float mErrors;
    float* mClassDist;
    float mCases;
public:
    static DecisionTreePtr newTree(AttributeSpec* spec);
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
    void resetChild(int childCount);
    void setTypeGrowing();
    void splitOnDiscreteAttribute(int attNum);
    void splitOnContinuousAttribute(int attNum, float threshold);
    int getChildCount();
    DecisionTreePtr getChild(int index);
    DecisionTreePtr oneStepClassify(IInstance* e);
    int classify(IInstance* e, float& confidence);
    void growingNodes(std::list<DecisionTreePtr>& list);
    void gatherLeaves(std::list<DecisionTreePtr>& list);
    void gatherGrowingNodes(std::list<DecisionTreePtr>& list);
    int  countNodes();
    int  getMostCommonClass();
    void setGrowingData(void *data);
    void *getGrowingData();
    static DecisionTreePtr readC5(std::istream& in, AttributeSpec* spec);
    static DecisionTreePtr readC5Bin(std::istream& in,AttributeSpec* spec);
    static DecisionTreePtr readC5Text(std::istream& in, AttributeSpec* spec);
    static DecisionTreePtr read(std::istream& in, AttributeSpec* spec);
    void write(std::ostream& out);
    void print(std::ostream& out);
    void printStats(std::ostream& out);
    inline void setBit(Set64& s, int bit) const
    {
        s |= (0x01 < bit); //s.setbit at b
    }
    inline bool testBit(Set64& s, int bit) const
    {
        return s & (0x01 < bit);
    }
private:
    static void getMostCommonClassHelper(DecisionTreePtr dt, long *counts);
    static void printHelper(DecisionTreePtr dt, std::ostream& out, int indent);
    static void printStatHelper(DecisionTreePtr dt, long *leavesAtLevel, long *leaves, int level, int maxLevel);
    static int which(char* val, char** list, int first, int last);
    static Set64 makeSubset(char* PropVal, Attribute* attr);
    static int readProp(std::istream& is, char *delim, char* propName, char* proVal);
    friend class BoostDecisionTree;
};

class BoostDecisionTree
{
public:
    BoostDecisionTree();
    ~BoostDecisionTree();
    bool read(std::istream& in, AttributeSpec* spec);
    int classify(IInstance* e, float& confidence);
private:
    bool readHead(std::istream& in);
    int mTreeCount;
    int mNumClasses;
    DecisionTreePtr* mppTrees;
};
}
#endif /* DECISIONTREEH */
