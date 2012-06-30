#include "gtest/gtest.h"
#include <iostream>
#include "datetime.h"
using namespace std;
using namespace mlplus;
TEST(DateTime, parseTest){
    DateTime d = DateTime::parseString("2012-1-2 12:12:59.100");
    EXPECT_EQ(2012u, d.getYear());
    EXPECT_EQ(1u,d.getMonth());
    EXPECT_EQ(2u,d.getDay());
    EXPECT_EQ(12u,d.getHour());
    EXPECT_EQ(12u,d.getMinute());
    EXPECT_EQ(59u,d.getSecond());
    EXPECT_EQ(100u,d.getMilliSecond());
};

TEST(DateTime, addMonth){
    DateTime d = DateTime::parseString("2012-1-2 12:12:59.100");
    d.addMonth(1);
    EXPECT_EQ(2012u, d.getYear());
    EXPECT_EQ(2u,d.getMonth());
    EXPECT_EQ(2u,d.getDay());
    EXPECT_EQ(12u,d.getHour());
    EXPECT_EQ(12u,d.getMinute());
    EXPECT_EQ(59u,d.getSecond());
    EXPECT_EQ(100u,d.getMilliSecond());
};
TEST(DateTime, addMonthMore){
    DateTime d = DateTime::parseString("2012-1-2 12:12:59.100");
    d.addMonth(13);
    EXPECT_EQ(2013u, d.getYear());
    EXPECT_EQ(2u,d.getMonth());
    EXPECT_EQ(2u,d.getDay());
    EXPECT_EQ(12u,d.getHour());
    EXPECT_EQ(12u,d.getMinute());
    EXPECT_EQ(59u,d.getSecond());
    EXPECT_EQ(100u,d.getMilliSecond());
};
