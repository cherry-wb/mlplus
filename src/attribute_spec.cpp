#include <cassert>
#include <vector>
#include <algorithm>
#include "attribute_spec.h"
#include "attribute.h"
#include "expression.h"
#include "string_utility.h"
using namespace mlplus;
using namespace std;
AttributeSpec::AttributeSpec():mTargetIndicator(-1)
{
}
AttributeSpec::~AttributeSpec()
{
    std::vector<Expression*>::iterator it =  mImplictExpression.begin();
    for(; it != mImplictExpression.end(); ++it)
    {  
        if (*it)
        {
            delete *it;
        }
        *it = NULL;
    }
    mImplictExpression.clear();
    std::vector<Attribute*>::iterator ita =  mAttributes.begin();
    for(; ita != mAttributes.end(); ++ita)
    {
        if (*ita)
        {
            delete *ita;
        }
        *ita = NULL;
    }
    mAttributes.clear();
}

int AttributeSpec::numTarget() const
{
    return mAttributes.at(mTargetIndicator)->numValues();
}
AttributeSpec::AttributeSpec(NamesFileReader& reader)
{
    typedef const std::vector<AttributeDesc>  AttributeDescVec;
    typedef AttributeDescVec::const_iterator Iterator;
    AttributeDescVec& explict= reader.getExplicitAttributeMeta();
    Iterator exIt = explict.begin();
    int i = 0;
    for (; exIt != explict.end(); ++exIt)
    {
        Attribute* at = makeAttribute(*exIt);
        if (at)
        {
           at->setIndex(i++);
        }
        mAttributes.push_back(at);
        if (NamesFileReader::testBit(exIt->type, NamesFileReader::TARGET))
        {
            mTargetIndicator = exIt - explict.begin();
        }
    }
    //
    AttributeDescVec& implict= reader.getImplicitAttributeMeta();
    Iterator imIt = implict.begin();
    for (; imIt != implict.end(); ++imIt)
    {
        Expression* ex = makeExpression(*imIt);
        mImplictExpression.push_back(ex);
        Attribute* tmp = NULL;
        if (ex->isLogicExpression())
        {
           tmp = new Attribute(imIt->name, Attribute::BINARY);
        }
        else
        {
           tmp = new Attribute(imIt->name, Attribute::NUMERIC);
        }
        tmp->setIndex(i++);
        mAttributes.push_back(tmp);
    }
}
Attribute* AttributeSpec::makeAttribute(const AttributeDesc& desc) const
{
    assert(!NamesFileReader::testBit(desc.type, NamesFileReader::IMPLICIT));
    if (NamesFileReader::testBit(desc.type, NamesFileReader::IGNORE) ||
        NamesFileReader::testBit(desc.type, NamesFileReader::LABEL))
    {
        return NULL;
    }
    if (NamesFileReader::testBit(desc.type, NamesFileReader::NUMERIC))
    {
        return new Attribute(desc.name); 
    }
    else if (NamesFileReader::testBit(desc.type, NamesFileReader::DATE))
    {
        return new Attribute(desc.name, Attribute::DATE); 
    }
    else if (NamesFileReader::testBit(desc.type, NamesFileReader::TIME))
    {
        return new Attribute(desc.name, Attribute::TIME); 
    }
    if (NamesFileReader::testBit(desc.type, NamesFileReader::TIMESTAMP))
    {
        return new Attribute(desc.name, Attribute::TIMESTAMP); 
    }
    if (NamesFileReader::testBit(desc.type, NamesFileReader::BINARY))
    {
        return new Attribute(desc.name, Attribute::BINARY); 
    }
    if (NamesFileReader::testBit(desc.type, NamesFileReader::NAMEDNOMINAL))
    {
        vector<string> vec;
        split(desc.value, vec, ",");
        assert(vec.size() > 1);
        vector<string>::iterator it = vec.begin();
        for(; it != vec.end(); ++it)
        {
            mlplus::trim(*it);
        }
        bool order = (NamesFileReader::testBit(desc.type, NamesFileReader::ORDERED));
        return new Attribute(desc.name, vec, order); 
    }
    if (NamesFileReader::testBit(desc.type, NamesFileReader::COMPACTNOMINAL))
    { 
        int value = atoi(desc.value.c_str());
        return new Attribute(desc.name, value); 
    }
    return NULL;
}
Expression* AttributeSpec::makeExpression(const AttributeDesc& desc) const
{
    assert(NamesFileReader::testBit(desc.type, NamesFileReader::IMPLICIT));
    return new Expression(desc.value.c_str());
}
Attribute* AttributeSpec::findAttribute(int index)
{
    int i = index;
    int sz = mAttributes.size();
    for (; i < sz; ++i)
    {
        if (mAttributes[i] && mAttributes[i]->getIndex() == i)
        {
            return mAttributes[i];
        }
    }
    return NULL;
}
int  AttributeSpec::findIndex(const std::string& name) const
{
    std::vector<Attribute*>::const_iterator it =  mAttributes.begin();
    for(; it != mAttributes.end(); ++it)
    {
        if (NULL != *it)
        {
            if ((*it)->getName() == name)
            {
                return (*it)->getIndex();
            }
        }
    }
    return -1;
}
