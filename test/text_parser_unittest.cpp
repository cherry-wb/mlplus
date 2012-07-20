#include "io/text_parser.h"
#include "names_file_reader.h"
#include "attribute_spec.h"
#include "dataset.h"
#include <vector>
#include <string>
#include <memory>
#include "gtest/gtest.h"
using namespace mlplus;
using namespace std;
TEST(textParserText, smock) {
    std::auto_ptr<TextParser> p(new TextParser("example.names"));
    std::auto_ptr<DataSet> pData(p->readData("example.cases"));
    AttributeSpec* spec = p->getAttributeSpec();
    EXPECT_EQ(pData->targetIndex(), 0); //remove all the label attribue
    EXPECT_EQ(pData->numTargets(), 2); 
    EXPECT_EQ(pData->numAttributes(), 11); 
    EXPECT_EQ(spec->explictAttributeCount(), 12u); 
    EXPECT_EQ(pData->numInstances(),2696); 
    float f[] = {1,351,1.02366,3,7.64E-12,117,115,1.939192,62.7151,350.7265,0.1055139};
    IInstance* first = pData->instanceAt(0);
    const vector<ValueType>& values =  first->getValueArray();
    EXPECT_EQ(values.size() ,11u); 
    for (unsigned i = 0; i < values.size(); ++i)
    {
        EXPECT_EQ(values[i] ,f[i]); 
    }
    IInstance* last = pData->instanceAt(2695);
    const vector<ValueType>& v2 =  last->getValueArray();
    EXPECT_EQ(v2.size() ,11u); 
    float l[] = {0,0,0.96331,0,4.48E-12,1,0,0.0625,0,14,0};
    for (unsigned i = 0; i < v2.size(); ++i)
    {
        EXPECT_EQ(v2[i] ,l[i]); 
    }
}
