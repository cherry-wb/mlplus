#include <string>
#include <vector>
#include "gtest/gtest.h"
#include <iostream>
#include "string_utility.h"
using namespace std;
using namespace mlplus;
TEST(convert,toUpper){
    string a = "\tab\t";
    toupper(a);
    EXPECT_EQ(a, "\tAB\t");
}
TEST(convert,toupperCopyTest){
    string a = "\tab\t";
    string b = toupperCopy(a);
    EXPECT_EQ(b, "\tAB\t");
}
TEST(convert,tolowerTest){
    string a = "ABYZaz";
    tolower(a);
    EXPECT_EQ(a, "abyzaz");
}
TEST(convert,tolowerCopyTest){
    string a = "ABYZaz";
    string b = tolowerCopy(a);
    EXPECT_EQ(b, "abyzaz");
    EXPECT_EQ(a, "ABYZaz");
}
