#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include "io/text_parser.h"
#include "scope.h"
#include "attribute_container.h"
#include "instance_container.h"
#include "instance.h"
#include "dataset.h"
#include "attribute_spec.h"
#include "expression.h"
#include "string_utility.h"
using namespace std;
namespace mlplus
{
TextParser::TextParser(const std::string& headerFileName): AbstractParser(headerFileName), mDelim(",")
{
    NamesFileReader reader(headerFileName);
    mpSpec = new AttributeSpec(reader);
}
TextParser::~TextParser()
{
    delete mpSpec;
}
DataSet* TextParser::readData(const std::string& filename)
{
    ifstream inFile(filename.c_str());
    if(!inFile.is_open())
    {
        cerr << "\nERROR: Cannot open file <" << filename << ">!!" << endl;
        exit(1);
    }
    VectorAttributeContainer* attributes = new VectorAttributeContainer();
    DenseInstanceContainer* instances = new DenseInstanceContainer();
    DataSet* pDataSet = new DataSet("anonymous", attributes, instances);
    const std::vector<Attribute*>& allAttri = mpSpec->attributesVector();
    const std::vector<Expression*>& expressions = mpSpec->expressionVector();
    Attribute* target = NULL;
    for (unsigned int i = 0; i < allAttri.size(); ++i)
    {
        if (NULL == allAttri[i])  continue;
        target = allAttri[i]->clone();
        attributes->add(target);
        if (mpSpec->isTarget(target))
        {
            pDataSet->setTarget(target);
        }
    }
    int lineCount  = 0;
    vector<string> valuelist;
    vector<float> decodeValue;
    decodeValue.reserve(512);
    Scope scope;
    string line;

    while(getline(inFile, line))
    {
        ++lineCount;
        mlplus::split(line, valuelist, mDelim);
        if (valuelist.size() != mpSpec->explictAttributeCount())
        {
            cerr << "\nERROR at line " << lineCount << " filename " << endl;
            delete pDataSet;
            exit(1);
        }
        decodeValue.clear();
        for (unsigned j = 0; j < mpSpec->explictAttributeCount(); ++j)
        {
            if (allAttri[j])
            {
                int index = allAttri[j]->indexOfValue(valuelist[j]);
                if (index < 0)
                {
                    float v = atof(valuelist[j].c_str());
                    decodeValue.push_back(v);
                    scope.add(allAttri[j]->getName(), v);
                }
                else
                {
                    scope.add(allAttri[j]->getName(), valuelist[j]);
                    decodeValue.push_back(index);
                }
            }
        }
        for (unsigned j = 0; j < mpSpec->implictAttributeCount(); ++j)
        {
           decodeValue.push_back(expressions[j]->evaluate(scope));
        }
        //for implicted attribute
        IInstance* instance = new DenseInstance(decodeValue);
        instance->setDataset(pDataSet);
        instances->add(instance);
    }
    return pDataSet;
}
}

