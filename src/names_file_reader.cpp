#include "names_file_reader.h"
#include "string_utility.h"
#include <fstream>
#include <iostream>
using namespace mlplus;
using namespace std;

NamesFileReader::AttributeDesc::AttributeDesc():
    type(0)
{
}

NamesFileReader::AttributeDesc  NamesFileReader::getExplictAttrMeta(int i) const
{
    return mExplicitAttributeMeta[i];
}
NamesFileReader::AttributeDesc  NamesFileReader::getAttributeMeta(const string& name) const
{
    std::vector<AttributeDesc>::const_iterator it = mExplicitAttributeMeta.begin();
    for(; it != mExplicitAttributeMeta.end(); ++it)
    {
        if (it->name == name)
        {
            return *it;
        }
    }
    it = mImplicitAttributeMeta.begin();
    for(; it != mImplicitAttributeMeta.end(); ++it)
    {
        if (it->name == name)
        {
            return *it;
        }
    }
    return AttributeDesc();
}

int  NamesFileReader::getIdFromName(const std::string& name) const
{
    std::vector<AttributeDesc>::const_iterator it = mExplicitAttributeMeta.begin();
    for(; it != mExplicitAttributeMeta.end(); ++it)
    {
        if (it->name == name)
        {
            return it - mExplicitAttributeMeta.begin();
        }
    }
    return -1;
}
void  NamesFileReader::read(const std::string& filename)
{
    ifstream ifs(filename.c_str());
    read(ifs);
}
void NamesFileReader::read(istream& istr)
{
    string line;
    while(std::getline(istr, line))
    {
        parseLine(line);
    }
    //stuff target attribute
    stuffTarget();
    setAttributeIgnore();
    translateValue();
}

void NamesFileReader::setAttributeIgnore()
{
    if (mIncludeAttr.empty() && mExcludeAttr.empty())
    {
        return;
    }
    std::vector<AttributeDesc>::iterator it = mExplicitAttributeMeta.begin();
    for(; it != mExplicitAttributeMeta.end(); ++it)
    {
        if (!mIncludeAttr.empty())
        {
            if (mIncludeAttr.find(it->name) == mIncludeAttr.end()) 
            {
                it->type |= IGNORE;
            }
        }
        if (!mExcludeAttr.empty())
        {
            if (mExcludeAttr.find(it->name) != mExcludeAttr.end())
            {
                it->type |= IGNORE;
            }
        }
    }
}
bool NamesFileReader::stuffTarget()
{
    bool hasTarget = false;
    std::vector<AttributeDesc>::iterator it = mExplicitAttributeMeta.begin();
    for(; it != mExplicitAttributeMeta.end(); ++it)
    {
        if (mTargetNames.find(it->name)!= mTargetNames.end())
        {
            hasTarget = true;
            it->type |= TARGET;
        }
    }
    return hasTarget;
}


bool NamesFileReader::translateValue()
{
    bool hasError = false;
    std::vector<AttributeDesc>::iterator it = mExplicitAttributeMeta.begin();
    for(; it != mExplicitAttributeMeta.end(); ++it)
    {
        if (it->value.empty())
        {
            it->type |= NUMERIC;
        }
        else if (it->value == "continuous")//The attribute takes numeric it->values.
        {
            it->type |= NUMERIC;
        }
        else if (it->value == "date")//YYYY/MM/DD or YYYY-MM-DD, e.g. 2005/09/30 or 2005-09-30.
        {
            it->type |= DATE;
        }
        else if (it->value == "time")//HH:MM:SS with it->values between 00:00:00 and 23:59:59.
        {
            it->type |= TIME;
        }
        else if (it->value == "timestamp")
        {
            it->type |= TIMESTAMP;
        }
        //This attribute contains an identifying label for each case
        else if (it->value == "label")
        {
            it->type |= LABEL;
        }
        //The it->values of the attribute should be ignored.
        else if (it->value == "ignore")
        {
            it->type |= IGNORE;
        }
        else if (it->value == "binary")
        {
            it->type |= BINARY;
        }
        else if (0==it->value.compare(0,9,"[ordered]"))//grade: [ordered] low, medium, high.
        {
            it->value = it->value.substr(9);
            it->type |= NAMEDNOMINAL;
        }
        else if (isInteger(it->value))
        {
            it->type |= COMPACTNOMINAL;
        }
        else if(!testBit(it->type, IMPLICIT))//grade: low, medium, high
        {
            it->type |= NAMEDNOMINAL;
        }
    }
    return hasError;
}
void NamesFileReader::parseLine(const string& line)
{
    //find first 'comment seperator'
    string::size_type pos = line.find_first_of('|');
    while(pos > 0 && pos != string::npos && line[pos - 1] == '\\')
    {
        pos = line.find_first_of('|', pos + 1);
    }
    //
    vector<string> result;
    split(line.substr(0, pos), result, ":");
    if (!result.empty())
    {
        AttributeDesc desc;
        if (result.size() == 1)//target attribute
        {
            desc.name = trimCopy(result[0]);
            if (*desc.name.rbegin() == '.')
            {
                desc.name.resize(desc.name.size() - 1);
            }
            mTargetNames.insert(desc.name);
        }
        else if (result.size() == 2)
        {
            desc.name = tolowerCopy(trimCopy(result[0]));
            size_t end = result[1].size();
            if (*result[1].rbegin() == '.')
            {
                --end;
            }
            if ('=' == result[1][0])  //that is  a IMPLICIT attribute 
            {
                --end;
                desc.type |= IMPLICIT;
                desc.value = trimCopy(result[1].substr(1,end));
                mImplicitAttributeMeta.push_back(desc);
            }
            else
            {
                desc.value = trimCopy(result[1].substr(0,end));
                char* aInclude[] = {"attributes", "include"};
                char* aExclude[] = {"attributes", "exclude"};
                vector<string> attrInlude(aInclude, aInclude + 2);
                vector<string> attrExclude(aExclude, aExclude + 2); 
                vector<string> key;
                split(desc.name, key);
                if (key == attrInlude)
                {
                    vector<string> values;
                    split(desc.value, values,",");
                    mIncludeAttr.insert(values.begin(), values.end());
                }
                else if (key == attrExclude)
                {
                    vector<string> values;
                    split(desc.value, values,",");
                    mExcludeAttr.insert(values.begin(), values.end());
                }
                else
                {
                    mExplicitAttributeMeta.push_back(desc);
                }
            }
        }
    }
}
Attribute* NamesFileReader::makeAttribute(int index) const
{
    return NULL;
}

