#ifndef MLPLUS_ATTRIBUTE_H
#define MLPLUS_ATTRIBUTE_H

#include <map>
#include <string>
#include <vector>
#include <cmath>
#ifndef NAN
# define NAN builtinnan("")
#endif

namespace mlplus
{

/** attribute represent the meta feature of a sample
 *  a simple application is predict someone's health status by height,weight, gender and atype
 *  we have the attribute matrix as following:
 *
 *  ================================================
 *     is_healthy  height  weight   gender   atype
 *     pefect      180     70       male     1
 *     bad         150     50       male     2
 *     good        160     49       female   1
 *  =================================================
 *
 *  the first attribute with the flowing properties:
 *      name = is_healthy
 *      index  = 0
 *      istarget = true
 *      type =  STRING with type [pefect, good, bad]
 *
 *  the last attribute with the flowing properties:
 *      name = atype
 *      index  = 4
 *      istarget = false
 *      type =  string with type [pefect, good, bad]
 */

struct Range
{
    double lowerBound;
    double upperBound;
    bool lowerBoundIsOpen;
    bool upperBoundIsOpen;
    /*
     * default range is (-inf, +inf)
     */
    Range(double lower = -INFINITY, double upper = INFINITY, bool lowerOpen = true, bool upperOpen = true):
        lowerBound(lower), upperBound(upper), lowerBoundIsOpen(lowerOpen), upperBoundIsOpen(upperOpen)
    {
    }
};

class Attribute
{
public:
    enum AttributeType
    {
        NUMERIC = 0, //the real value, with range. e.x: [0, n]
        STRING = 1,   //the string value.  e.g.  words
        NAMEDNOMINAL = 2, //the multinomial value .e.g. [0, 1, 2] 
        BINARY = 3, //for sparse attribute, means attribute exit in instance
        DATE = 4,
        COMPACTNOMINAL, //the multinomial interger value from 0 to n, [0,n)
        TIME,
        TIMESTAMP,
        UNKNOW
    };
protected:
    std::string mName;
    /*
     * only NAMEDNOMINAL and string attribute has mValues and mValues2Index 
     */
    std::map<std::string, int> mValue2Index; //
    std::vector<std::string> mValues; //index ---> string
    int  mIndex;
    AttributeType mType;
    double mWeight;
    Range mRange;
    int mValuesSize;
    bool mValuesOrdered;
public:

    /** 
     * @brief constructor of attribute
     * 
     * @param attrName The name of the attribute,
     * @param type   The type of the attribute 
     */
    Attribute(const std::string& attrName, AttributeType type = NUMERIC);
    /*
     * @brief construct a compact nominal attribute with size
     * @param attrName The name of the attribute
     * @param nonimalSize the size of the nominal attribute, means the value would range [0, nominalSize),
     *        which elements are all interger number
     * @param ordered the multi-nomial value type is ordered. e.x: low,mid,high
     */
    Attribute(const std::string& attrName, int nominalSize, bool ordered = false);
    /*
     * @brief construct a compact named nominal
     * @param attrName The name of the attribute
     * @param values the named nomial values with string types
     * @param ordered the multi-nomial value type is ordered. e.x: low,mid,high
     */
    Attribute(const std::string& attrName, std::vector<std::string>& values, bool ordered = false);
    virtual ~Attribute();
    inline const std::string&  getName() const
    {
        return mName;
    }
    inline AttributeType getType() const
    {
        return mType;
    }
    inline double getWeight() const
    {
        return mWeight;
    }
    inline void setWeight(double weight) 
    {
        mWeight = weight;
    }
    inline void setOrdered(bool ord) 
    {
        mValuesOrdered = ord;
    }
    inline bool isOrdered() const
    {
        return mValuesOrdered;
    }
    inline bool isNamedNominal() const
    {
        return mType == NAMEDNOMINAL;
    }
    inline bool isCompactNominal() const
    {
        return mType == COMPACTNOMINAL;
    }
    inline bool isBinary() const
    {
        return mType == BINARY;
    }
    inline bool isNumeric() const
    {
        return mType == NUMERIC;
    } 
    inline bool isString() const
    {
        return mType == STRING;
    }
    inline void setIndex(int index) 
    { 
        mIndex = index;
    } 
    inline int getIndex() const 
    { 
        return mIndex;
    }
    inline const Range& getRange() const
    {
        return mRange;
    } 
    inline double getLowerBound() const
    {
        return mRange.lowerBound;
    }
    inline bool lowerBoundIsOpen() const
    {
        return mRange.lowerBoundIsOpen;
    }
    inline double getUpperBound() const
    {
        return mRange.upperBound;
    }
    inline bool upperBoundIsOpen() const
    {
        return mRange.upperBoundIsOpen;
    }
    inline Attribute* clone(void) const
    {
        return new Attribute(*this);
    }
    inline const std::string& getValue(int index)
    {
        return mValues.at(index);
    }
    inline bool setValue(unsigned int index, const std::string& s)
    {
        if (index < mValues.size())
        {
            mValues[index] = s;
            mValue2Index[s] = index;
            return true;
        }
        return false;
    }
    inline void addValue(const std::string& s)
    {
        mValue2Index[s] = mValues.size();
        mValues.push_back(s);
        mValuesSize = mValue2Index.size();
    }
    inline void reserveValues(size_t storage)
    {
        return mValues.reserve(storage);
    }
    inline int numValues() const
    {
        return mValuesSize;/*for named nomial value count  = mValues.size();*/
    }
    int indexOfValue(const std::string& value) const;
    bool isInRange(double value) const;
    bool isRange(const std::string& str) const; 
    std::string toString() const;
    void mapValue2Index();

};
}
#endif
