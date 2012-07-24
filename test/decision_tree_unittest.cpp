#include <decision_tree.h>
#include <names_file_reader.h>
#include <attribute_spec.h>
#include <attribute.h>
#include <vector>
#include <string>
#include <fstream>
#include "io/text_parser.h"
#include "dataset.h"
#include "gtest/gtest.h"
using namespace mlplus;
using namespace std;
TEST(decisionTreeTest, smock) {
    std::auto_ptr<TextParser> p(new TextParser("example.names"));
    DataSet* pData(p->readData("example.cases"));
    ifstream ifs("example.tree");
    DecisionTreePtr ptr = DecisionTree::readC5Text(ifs, p->getAttributeSpec()); 
    //ptr->print(cout);
    for (int i = 0; i < pData->numInstances(); ++i)
    {
        IInstance* instance = pData->instanceAt(i);
        Attribute* attr = pData->targetAttribute();
        cout << attr->getValue(instance->targetValue()) <<" " <<  attr->getValue(ptr->classify(instance)) << endl;
        //ptr->classify(instance);
    }
    ptr->free();
    delete pData;
}
