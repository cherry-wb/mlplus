#include "log.h"
#include "decision_tree.h"
#include "attribute_spec.h"
#include "instance_interface.h"
#include "attribute_value.h"
#include "string_utility.h"
namespace
{
static char* Prop[] = {"null", "att", "class", "cut", "conds", "elts",
                       "entries", "forks", "freq", "id", "type", "low", "mid", "high",
                       "result", "rules", "val", "lift", "cover", "ok", "default",
                       "costs", "sample", "init"};

static const int PROPS = 23;
static const int ERRORP = 0;
static const int ATTP   = 1;
static const int CLASSP = 2;
static const int CUTP   = 3;
static const int CONDSP  = 4;
static const int ELTSP  = 5;
static const int ENTRIESP = 6;
static const int FORKSP = 7;
static const int FREQP  = 8;
static const int IDP    = 9;
static const int TYPEP  = 10;
static const int LOWP   = 11;
static const int MIDP   = 12;
static const int HIGHP  = 13;
static const int RESULTP    = 14;
static const int RULESP = 15;
static const int VALP   = 16;
static const int LIFTP  = 17;
static const int COVERP = 18;
static const int OKP    = 19;
static const int DEFAULTP = 20;
static const int COSTSP = 21;
static const int SAMPLEP    = 22;
static const int INITP  = 23;
}
using namespace mlplus;
using namespace std;
DecisionTreePtr DecisionTree::newTree(AttributeSpec* spec)
{
    DecisionTreePtr dt = new DecisionTree();
    dt->mNodeType = dtnGrowing;
    dt->mAttributeSpec = spec;
    dt->mGrowingData = 0;
    dt->mSplitAttribute = -1;
    dt->mSplitThreshold = 0;
    dt->mLower = 0;
    dt->mUpper = 0;
    dt->mMid = 0;
    dt->mForks = 0;
    dt->mChildren = NULL;
    dt->mSubset = 0;
    dt->mMyClass = 0;
    dt->mErrors = 0;
    dt->mCases = 0;
    dt->mClassDist = new float[spec->numTarget()];
    dt->zeroClassDistribution();
    return dt;
}

void DecisionTree::free()
{
    int i;
    if(mChildren)
    {
        if(mNodeType == dtnDiscrete || mNodeType == dtnContinuous)
        {
            for(i = mForks ; i >= 0 ; i--)
            {
                mChildren[i]->free();
            }
            delete[] mChildren;
        }
    }
    if (mClassDist) delete[] mClassDist;
    if (mSubset) delete[] mSubset;
}

DecisionTreePtr DecisionTree::clone()
{
    int i;
    DecisionTreePtr clone = newTree(mAttributeSpec);
    *clone = *this;
    if (NULL != mChildren)
    {
        clone->mChildren = new DecisionTreePtr[mForks];
    }
    if (NULL != mSubset)
    {
        clone->mSubset = new Set64[mForks];
        memcpy(clone->mSubset, mSubset, sizeof(Set64) * mForks);
    }
    if (NULL != mClassDist)
    {
        clone->mClassDist = new float[mAttributeSpec->numTarget()];
        memcpy(clone->mClassDist, mClassDist, sizeof(float) * mAttributeSpec->numTarget());
    }
    if(mNodeType == dtnDiscrete || mNodeType == dtnContinuous)
    {
        for(i = 0 ; i < mForks ; i++)
        {
            clone->mChildren[i] = mChildren[i]->clone();
        }
    }
    return clone;
}

int DecisionTree::isLeaf()
{
    return mNodeType == dtnLeaf;
}

int DecisionTree::isTreeGrowing()
{
    int i;
    switch(mNodeType)
    {
    case dtnLeaf:
        return 0;
    case dtnGrowing:
        return 1;
    case dtnContinuous:
    case dtnDiscrete:
        for(i = 0 ; i < mForks ; i++)
        {
            if(mChildren[i]->isTreeGrowing())
            {
                return 1;
            }
        }
        return 0;
    }
    return 0;
}

int DecisionTree::isNodeGrowing()
{
    return mNodeType == dtnGrowing;
}

int DecisionTree::getClass()
{
    return mMyClass;
}

void DecisionTree::setClass(int theClass)
{
    mMyClass = theClass;
}

float DecisionTree::getClassProb(int theClass)
{
    return mClassDist[theClass] /(float)mCases;
}

void  DecisionTree::setClassProb(int theClass, float prob)
{
    if(mCases == 0)
    {
        mCases = 1;
    }
    mClassDist[theClass] = prob * mCases;
}

void DecisionTree::addToClassDistribution(IInstance* ptr)
{
    mCases++;
    mClassDist[(int)ptr->targetValue()]++;
}

void DecisionTree::zeroClassDistribution()
{
    for(int i = 0 ; i < mAttributeSpec->numTarget() ; i++)
    {
        mClassDist[i] = 0;
    }
    mCases = 0;
}

void DecisionTree::setTypeLeaf()
{
    int i;
    if(mNodeType != dtnLeaf && mNodeType != dtnGrowing)
    {
        /* free mChildren */
        for(i = 0; i < mForks ; ++i)
        {
            mChildren[i]->free();
        }
    }
    mNodeType = dtnLeaf;
}

void DecisionTree::setTypeGrowing()
{
    mNodeType = dtnGrowing;
}

void DecisionTree::splitOnDiscreteAttribute(int attNum)
{
    int i;
    mNodeType = dtnDiscrete;
    mSplitAttribute = attNum;
    Attribute* attr = mAttributeSpec->attributeAt(i);
    resetChild(attr->numValues());
    for(i = 0 ; i < mForks; i++)
    {
        mChildren[i] = newTree(mAttributeSpec);
    }
}
void DecisionTree::resetChild(int childCount)
{
    for (int i = 0; i < mForks; ++i)
    {
        mChildren[i]->free();
    }
    mForks = childCount;
    if (NULL != mChildren)
    {
        delete mChildren;
    }
    mChildren = new DecisionTreePtr[childCount];
}
void DecisionTree::splitOnContinuousAttribute(int attNum, float threshold)
{
    mNodeType = dtnContinuous;
    mSplitAttribute = attNum;
    mSplitThreshold = threshold;
    resetChild(2);
    mChildren = new DecisionTreePtr[mForks];
    mChildren[0] = newTree(mAttributeSpec);
    mChildren[1] = newTree(mAttributeSpec);
}

int DecisionTree::getChildCount()
{
    return mForks;
}

DecisionTreePtr DecisionTree::getChild(int index)
{
    return mChildren[index];
}


DecisionTreePtr DecisionTree::oneStepClassify(IInstance* e)
{
    if(mNodeType == dtnLeaf || mNodeType == dtnGrowing)
    {
        return this;
    }
    ValueType value = e->getValue(mSplitAttribute);
    if(mNodeType == dtnDiscrete)
    {
        if(AttributeValue::isMissingValue(value))
        {
            /* HERE do something smarter */
            return mChildren[0];
        }
        else
        {
            int target = 0;
            int disvalue = int(value);
            if (disvalue != mForks)
            {
                for (int i = 0; i < mForks; ++i)
                {
                    if (testBit(mSubset[i], disvalue))
                    {
                        target = i;
                        break;
                    }
                }
            }
            else
            {
                target = disvalue;
            }
            return mChildren[target];
        }
    }
    else if(mNodeType == dtnContinuous)
    {
        if(AttributeValue::isMissingValue(value))
        {
            /* HERE do something smarter */
            return mChildren[0];
        }
        else if(value >= mSplitThreshold)
        {
            return mChildren[1];
        }
        else     /* the value is < the threshold */
        {
            return mChildren[0];
        }
    }

    return this;
}

int DecisionTree::classify(IInstance* ins)
{
    int depth = 1;
    DecisionTreePtr current, last;
    last = this;
    current = last->oneStepClassify(ins);
    while((depth < 50000) && (last != current))
    {
        last = current;
        current = last->oneStepClassify(ins);
        depth++;
    }
    //WARN_IF(last != current, "%s", "at depth 50000 in,something must be wrong.\n");
    return current->mMyClass;
}

void DecisionTree::gatherLeaves(list<DecisionTreePtr>& list)
{
    int i;
    if(mNodeType == dtnLeaf)
    {
        list.push_back(this);
    }
    else if(mNodeType == dtnDiscrete || mNodeType == dtnContinuous)
    {
        for(i = 0 ; i < mForks; i++)
        {
            mChildren[i]->gatherLeaves(list);
        }
    }
}

void DecisionTree::gatherGrowingNodes(list<DecisionTreePtr>& list)
{
    int i;
    if(mNodeType == dtnGrowing)
    {
        list.push_back(this);
    }
    else if(mNodeType == dtnDiscrete || mNodeType == dtnContinuous)
    {
        for(i = 0 ; i < mForks; i++)
        {
            mChildren[i]->gatherGrowingNodes(list);
        }
    }
}

int  DecisionTree::countNodes()
{
    int sum;
    int i;
    if(mNodeType == dtnGrowing || mNodeType == dtnLeaf)
    {
        sum = 1;
    }
    else if(mNodeType == dtnDiscrete || mNodeType == dtnContinuous)
    {
        sum = 1;
        for(i = 0 ; i < mForks ; i++)
        {
            sum += mChildren[i]->countNodes();
        }
    }
    else
    {
        /* how did we get here? */
        WARN("%s", "There is an odd condition in DecisionTreeCountNodes.\n");
        sum = 1;
    }

    return sum;
}

void DecisionTree::getMostCommonClassHelper(DecisionTreePtr dt, long *counts)
{
    int i;
    if(dt->mNodeType == dtnGrowing)
    {
    }
    else if(dt->mNodeType == dtnLeaf)
    {
        (counts[dt->mMyClass])++;
    }
    else if(dt->mNodeType == dtnDiscrete || dt->mNodeType == dtnContinuous)
    {
        for(i = 0 ; i < dt->mForks ; i++)
        {
            getMostCommonClassHelper(dt->mChildren[i], counts);
        }
    }
}

int DecisionTree::getMostCommonClass()
{
    int targetCount = mAttributeSpec->numTarget();
    long *counts = new long[targetCount];
    int i;
    int mostCommon;

    for(i = 0 ; i < targetCount; i++)
    {
        counts[i] = 0;
    }

    getMostCommonClassHelper(this, counts);

    mostCommon = 0;
    for(i = 1 ; i < targetCount; i++)
    {
        if(counts[i] > counts[mostCommon])
        {
            mostCommon = i;
        }
    }

    delete[] counts;

    return mostCommon;
}

void DecisionTree::setGrowingData(void *data)
{
    mGrowingData = data;
}

void* DecisionTree::getGrowingData()
{
    return mGrowingData;
}

static void _printSpaces(ostream& out, int num)
{
    int i;
    for(i = 0 ; i < num ; i++)
    {
        out << " ";
    }
}

void DecisionTree::printHelper(DecisionTreePtr dt, ostream& out, int indent)
{
    int i;
    _printSpaces(out, indent);
    if(dt->mNodeType == dtnLeaf)
    {
        out << "(leaf:" << dt->mMyClass << "\n";
    }
    else if(dt->mNodeType == dtnDiscrete)
    {
        Attribute* attr = dt->mAttributeSpec->attributeAt(dt->mSplitAttribute);
        out << "(split on " << attr->getName() << "\n";
        for(i = 0 ; i < dt->mForks; i++)
        {
            _printSpaces(out, indent + 1);
            out <<  attr->getValue(i) << "\n";
            printHelper(dt->mChildren[i], out, indent + 2);
        }
        _printSpaces(out, indent);
        out <<  ")\n";
    }
    else if(dt->mNodeType == dtnContinuous)
    {
        Attribute* attr = dt->mAttributeSpec->attributeAt(dt->mSplitAttribute);
        out << "(split on" << attr->getName() << ":\n";
        /* left child */
        _printSpaces(out, indent + 1);
        out << "< " << dt->mSplitThreshold << "\n";
        printHelper(dt->mChildren[0], out, indent + 2);
        /* right child */
        _printSpaces(out, indent + 1);
        out << ">= " << dt->mSplitThreshold << "\n";
        printHelper(dt->mChildren[1], out, indent + 2);
        _printSpaces(out, indent);
        out << ")\n";
    }
    else if(dt->mNodeType == dtnGrowing)
    {
        out << "(growing)\n";
    }
}

void DecisionTree::print(ostream& out)
{
    printHelper(this, out, 0);
}

void DecisionTree::printStatHelper(DecisionTreePtr dt, long *leavesAtLevel, long *leaves, int level, int maxLevel)
{
    int i;

    if(dt->mNodeType == dtnGrowing || dt->mNodeType == dtnLeaf)
    {
        if(level < maxLevel)
        {
            (leavesAtLevel[level])++;
        }
        (*leaves)++;
    }
    else if(dt->mNodeType == dtnDiscrete || dt->mNodeType == dtnContinuous)
    {
        for(i = 0 ; i < dt->mForks; i++)
        {
            printStatHelper(dt->mChildren[i], leavesAtLevel, leaves, level + 1, maxLevel);
        }
    }
}

void DecisionTree::printStats(ostream& out)
{
    long leavesAtLevel[50];
    long leaves;
    int i, maxWithLeaves;

    leaves = 0;
    for(i = 0 ; i < 50 ; i++)
    {
        leavesAtLevel[i] = 0;
    }

    printStatHelper(this, leavesAtLevel, &leaves, 0, 50);

    maxWithLeaves = 0;
    for(i = 0 ; i < 50 ; i++)
    {
        if(leavesAtLevel[i] > 0)
        {
            maxWithLeaves = i;
        }
    }
    for(i = 0 ; i <= maxWithLeaves ; i++)
    {
        out << leavesAtLevel[i] << " ";
    }
    out << "\n";
}

DecisionTreePtr DecisionTree::readC5Bin(istream& in, AttributeSpec* spec)
{
    int i;
    DecisionTreePtr dt;
    short type, leaf, tested, forks;
    float items, errors, lower, upper, cut;

    dt = newTree(spec);
    in.read((char *)(&type), sizeof(short));
    in.read((char *)(&leaf), sizeof(short));
    in.read((char *)(&items), sizeof(float));
    in.read((char *)(&errors), sizeof(float));
    dt->mMyClass = leaf;
    int numTarget = spec->numTarget();
    in.read((char*)(dt->mClassDist), sizeof(float) * numTarget);
    for(i = 0 ; i < numTarget ; i++)
    {
        dt->mCases += dt->mClassDist[i];
    }
    if(type != 0)   /* if we don't have a leaf */
    {
        in.read((char *)(&tested), sizeof(short));
        in.read((char *)(&forks), sizeof(short));
        dt->mSplitAttribute = tested;

        switch(type)
        {
        case 1: /* BrDiscr */
            dt->mNodeType = dtnDiscrete;
            break;
        case 2: /* ThreshContin */
            dt->mNodeType = dtnContinuous;
            in.read((char *)(&cut), sizeof(float));
            in.read((char *)(&upper), sizeof(float));
            in.read((char *)(&lower), sizeof(float));
            dt->mSplitThreshold = cut;
            dt->mUpper = upper;
            dt->mLower = lower;
            break;
        }
        for(i = 0 ; i < forks ; i++)
        {
            dt->mChildren[i] = readC5Bin(in, spec);
        }

    }
    else
    {
        dt->mNodeType = dtnLeaf;
    }
    return dt;
}

int DecisionTree::readProp(istream& is, char *delim, char* propName, char* propVal)
{
    int  c;
    char *p;
    bool Quote = false;
    for(p = propName ; (c = is.get()) != '=' ;)
    {
        if(p - propName >= 19 || c == EOF)
        {
            ERROR("%s", "prop name parse error");
            propName[0] = propVal[0] = *delim = '\00';
            return 0;
        }
        *p++ = c;
    }
    *p = '\00';

    for(p = propVal ; ((c = is.get()) != ' ' && c != '\n') || Quote;)
    {
        if(c == EOF)
        {
            //Error("get prop name error:%s", __FILE__);
            propName[0] = propVal[0] = '\00';
            return 0;
        }
        if(c == '"')
        {
            Quote = !Quote;
        }
        else
        {
            *p++ = c;
            if(c == '\\')//for \"
            {
                *p++ = is.get();
            }
        }
    }
    *p = '\00';
    *delim = c;
    return which(propName, Prop, 1, PROPS);
}
int DecisionTree::which(char* val, char** list, int first, int last)
{
    int n = first;
    while(n <= last && strcmp(val, list[n])) n++;

    return (n <= last ? n : first - 1);
}

DecisionTreePtr DecisionTree::readC5Text(istream& in, AttributeSpec* spec)
{
    int64_t v, subset = 0;
    char    delim, *p;
    int  c = 0;
    int     X;
    double  XD;
    int  classCount = spec->numTarget();
    DecisionTreePtr pTree = new DecisionTree();
    char propName[128];
    char propVal[256];
    string str(propVal);
    do
    {
        switch(readProp(in, &delim, propName, propVal))
        {
        case ERRORP:
            delete pTree;
            return NULL;
        case TYPEP:
            sscanf(propVal, "%d", &X);
            pTree->mNodeType = (TreeNodeType)X;
            break;
        case CLASSP:
            pTree->mMyClass = spec->classNo(propVal);
            break;
        case ATTP:
            str = propVal;
            tolower(str);
            pTree->mSplitAttribute = spec->findIndex(str);
            if(pTree->mSplitAttribute < 0)
            {
                delete pTree;
                return NULL;
            }
            break;
        case CUTP:
            sscanf(propVal, "%lf", &XD);
            pTree->mSplitThreshold = XD;
            pTree->mLower = pTree->mMid = pTree->mUpper = XD;
            break;
        case LOWP:
            sscanf(propVal, "%lf", &XD);
            pTree->mLower = XD;
            break;
        case MIDP:
            sscanf(propVal, "%lf", &XD);
            pTree->mMid = XD;
            break;
        case HIGHP:
            sscanf(propVal, "%lf", &XD);
            pTree->mUpper = XD;
            break;
        case FORKSP:
            sscanf(propVal, "%d", &pTree->mForks);
            break;
        case FREQP:
            if (pTree->mClassDist) delete [] pTree->mClassDist;
            pTree->mClassDist = new float[classCount];
            p = propVal;
            for(c = 0; c < classCount; ++c)
            {
                pTree->mClassDist[c] = strtod(p, &p);
                pTree->mCases += pTree->mClassDist[c];
                p++;
            }
            break;
        case ELTSP:
            if(NULL == mSubset)
            {
                pTree->mSubset = new Set64[pTree->mForks];
            }
            ++subset;
            pTree->mSubset[subset] = makeSubset(propVal, spec->attributeAt(pTree->mSplitAttribute));
            break;
        }
    }
    while(delim == ' ');
    if(pTree->mClassDist)
    {
        pTree->mErrors = pTree->mCases - (int)pTree->mClassDist[pTree->mMyClass];
    }
    else
    {
        //pTree->mClassDist = Alloc(1, CaseCount);
    }
    if(pTree->mNodeType != dtnLeaf)
    {
        //T->Branch = AllocZero(T->Forks + 1, Tree);
        for(v = 0; v < pTree->mForks; ++v)
        {
            pTree->mChildren[v] = readC5Text(in, spec);
        }
    }
    return pTree;
}

DecisionTree::Set64 DecisionTree::makeSubset(char* propVal, Attribute* attr)
{
    Set64    S = 0;
    vector<string> splitVec;
    split(propVal, splitVec, ",");
    for(unsigned int i = 0; i < splitVec.size(); ++i)
    {
        int b = attr->indexOfValue(splitVec[i]);
        if(b < 0) ERROR("undefine attribute %s value:%s", attr->getName().c_str(), splitVec[i].c_str());
        S |= (0x01 < b); //s.setbit at b
    }
    return S;
}
DecisionTreePtr DecisionTree::read(istream& in,  AttributeSpec* spec)
{
    int i, forks;
    DecisionTreePtr dt = newTree(spec);
    in.read((char*)&dt->mNodeType, sizeof(TreeNodeType));
    switch(dt->mNodeType)
    {
    case dtnLeaf:
    case dtnGrowing:
        in.read((char*)&dt->mMyClass, sizeof(int));
        break;
    case dtnContinuous:
    case dtnDiscrete:
        in.read((char*)&dt->mSplitThreshold, sizeof(float));
        in.read((char*)&dt->mSplitAttribute, sizeof(int));
        in.read((char*)&forks, sizeof(int));
        for(i = 0 ; i < forks ; i++)
        {
            dt->mChildren[i] = (readC5Text(in, spec));
        }
        break;
    }
    return dt;
}

void DecisionTree::write(ostream& out)
{
    int i;
    int forks;
    out.write((char*)&(mNodeType), sizeof(TreeNodeType));
    switch(mNodeType)
    {
    case dtnLeaf:
    case dtnGrowing:
        out.write((char *)(&(mMyClass)), sizeof(int));
        break;
    case dtnContinuous:
    case dtnDiscrete:
        out.write((char *)(&(mSplitThreshold)), sizeof(float));
        out.write((char *)(&(mSplitAttribute)), sizeof(int));
        forks = mForks;
        out.write((char *)(&(forks)), sizeof(int));
        for(i = 0 ; i < forks ; i++)
        {
            mChildren[i]->write(out);
        }
        break;
    }
}
