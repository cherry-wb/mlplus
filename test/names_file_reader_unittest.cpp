#include <string>
#include <vector>
#include "gtest/gtest.h"
#include <iostream>
#include "names_file_reader.h"
using namespace std;
using namespace mlplus;
TEST(names_reader,read){
string str = "diagnosis.                     | the target attribute\n" \
    "age:                           continuous.\n" \
    "sex:                           M, F.\n" \
    "on thyroxine:                  f, t.\n" \
    "query on thyroxine:            f, t.\n" \
    "on antithyroid medication:     f, t.\n" \
    "sick:                          f, t.\n" \
    "pregnant:                      f, t.\n" \
    "thyroid surgery:               f, t.\n" \
    "I131 treatment:                f, t.\n" \
    "query hypothyroid:             f, t.\n" \
    "query hyperthyroid:            f, t.\n" \
    "lithium:                       f, t.\n" \
    "tumor:                         f, t.\n" \
    "goitre:                        f, t.\n" \
    "hypopituitary:                 f, t.\n" \
    "psych:                         f, t.\n" \
    "TSH:                           continuous.\n" \
    "T3:                            continuous.\n" \
    "TT4:                           continuous.\n" \
    "T4U:                           continuous.\n" \
    "FTI:=                          TT4 / T4U.\n" \
    "referral source:               WEST, STMW, SVHC, SVI, SVHD, other.\n" \
    "diagnosis:                     primary, compensated, secondary, negative.\n" \
    "ID:                            label.\n";
    stringstream ss(str); 
    NamesFileReader reader;
    reader.read(ss);

    AttributeDesc r = reader.getAttributeMeta("id");
    EXPECT_EQ(r.name,"id");
    EXPECT_EQ(r.value,"label");
    EXPECT_TRUE(NamesFileReader::testBit(r.type,NamesFileReader::LABEL));
    EXPECT_FALSE(NamesFileReader::testBit(r.type,NamesFileReader::NUMERIC));

    r = reader.getAttributeMeta("referral source");
    EXPECT_EQ(r.name,"referral source");
    EXPECT_EQ(r.value,"WEST, STMW, SVHC, SVI, SVHD, other");
    EXPECT_FALSE(NamesFileReader::testBit(r.type,NamesFileReader::LABEL));
    EXPECT_FALSE(NamesFileReader::testBit(r.type,NamesFileReader::ORDERED));
    EXPECT_FALSE(NamesFileReader::testBit(r.type,NamesFileReader::NUMERIC));
    EXPECT_TRUE(NamesFileReader::testBit(r.type,NamesFileReader::NAMEDNOMINAL));
    EXPECT_EQ(r.type, (uint64_t)0x20);

    r = reader.getExplictAttrMeta(21);
    EXPECT_EQ(r.name,"diagnosis");
    EXPECT_EQ(r.value,"primary, compensated, secondary, negative");
    EXPECT_FALSE(NamesFileReader::testBit(r.type,NamesFileReader::LABEL));
    EXPECT_FALSE(NamesFileReader::testBit(r.type,NamesFileReader::ORDERED));
    EXPECT_FALSE(NamesFileReader::testBit(r.type,NamesFileReader::NUMERIC));
    EXPECT_TRUE(NamesFileReader::testBit(r.type,NamesFileReader::NAMEDNOMINAL));
    EXPECT_TRUE(NamesFileReader::testBit(r.type,NamesFileReader::TARGET));

    r = reader.getAttributeMeta("fti");
    EXPECT_EQ(r.name,"fti");
    EXPECT_EQ(r.value,"TT4 / T4U");
    EXPECT_FALSE(NamesFileReader::testBit(r.type,NamesFileReader::LABEL));
    EXPECT_FALSE(NamesFileReader::testBit(r.type,NamesFileReader::ORDERED));
    EXPECT_FALSE(NamesFileReader::testBit(r.type,NamesFileReader::NUMERIC));
    EXPECT_FALSE(NamesFileReader::testBit(r.type,NamesFileReader::NAMEDNOMINAL));
    EXPECT_FALSE(NamesFileReader::testBit(r.type,NamesFileReader::TARGET));
    EXPECT_TRUE(NamesFileReader::testBit(r.type,NamesFileReader::IMPLICIT));
}
