#include <attribute_spec.h>
#include <attribute.h>
#include <vector>
#include <string>
#include "gtest/gtest.h"
using namespace mlplus;
using namespace std;
TEST(AttributeSpecTest, smock) {

    string str = "diagnosis.                     | the target attribute\n" \
    "age:                           continuous.\n" \
    "sex:                           M, F.\n" \
    "thyroxine:                     10.\n" \
    "query on thyroxine:            f, t.\n" \
    "TT4:                           continuous.\n" \
    "T4U:                           continuous.\n" \
    "FTI:=                          TT4 / T4U.\n" \
    "referral source:               WEST, STMW, SVHC, SVI, SVHD, other.\n" \
    "diagnosis:                     primary, compensated, secondary, negative.\n" \
    "ID:                            label.\n" \
    "ID2:                           label.\n";
    stringstream ss(str); 
    NamesFileReader reader;
    reader.read(ss);
    AttributeSpec a(reader);
    EXPECT_EQ(a.numTarget(), 4);
    EXPECT_EQ(a.numAttributes(), 11);
    Attribute *at = NULL;
    at = a.attributeAt(0);
    EXPECT_EQ(at->getName(), "age");
    at = a.attributeAt(1);
    EXPECT_EQ(at->getName(), "sex");
    const string value[] = { "age", "sex", "thyroxine", "query on thyroxine", 
        "tt4", "t4u","referral source", "diagnosis", "id", "id2", "fti"};
    const vector<Attribute*>& avector = a.attributesVector();
    for (unsigned i = 0; i < sizeof(value)/sizeof(value[0]); ++i)
    {
        if (i == 8 || i == 9) 
        {
            EXPECT_TRUE(avector[i] == NULL);
        }
        else
        {
            EXPECT_EQ(avector[i]->getName(), value[i]);
        }
    }
}
