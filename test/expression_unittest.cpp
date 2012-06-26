#include <string>
#include <vector>
#include "gtest/gtest.h"
#include <iostream>
#include "lexser.h"
#include "expression.h"
using namespace std;
using namespace mlplus;
TEST(expressionTest, addTesting){
    string str = "-a + +b";
    Lexser lexser(str.c_str());
    Token* head = lexser.getTokenHead();
    Expression ex;
    std::vector<Token*> postExpression;
    ex.parse(head,  postExpression);
    char* op[] = {"a", "-","b", "+", "+"};
    EXPECT_EQ(5u, postExpression.size());
    for (unsigned i = 0; i < postExpression.size(); ++i)
    {
        EXPECT_STREQ(op[i], postExpression[i]->str);
    }

    TokenType tt[] = {OP_VAR,OP_MINUS_1, OP_VAR, OP_ADD_1, OP_ADD};
    for (unsigned i = 0; i < postExpression.size(); ++i)
    {
        EXPECT_EQ(tt[i], postExpression[i]->type);
    }
    EXPECT_TRUE(ex.verify(postExpression));
}

TEST(expressionTest, abracketTesting){
    string str = "h*(d+e)";
    Lexser lexser(str.c_str());
    Token* head = lexser.getTokenHead();
    Expression ex;
    std::vector<Token*> postExpression;
    ex.parse(head,  postExpression);
    EXPECT_EQ(5u, postExpression.size());
    char* pos[] = {"h", "d","e","+","*"};
    for (unsigned i = 0; i < postExpression.size(); ++i)
    {
        EXPECT_STREQ(pos[i], postExpression[i]->str);
    }
}

TEST(expressionTest, bracketTesting){
    string str = "a+b * c + h*(d+e)/sin(x)*cos(y)";
    Lexser lexser(str.c_str());
    Token* head = lexser.getTokenHead();
    char* op[] = {"a", "+","b", "*", "c","+","h","*","(","d","+","e",")","/","sin","(","x",")","*","cos","(","y",")"};

    Token* tail = head;
    int j = 0;
    while(tail)
    {
        EXPECT_STREQ(op[j], tail->str);
        tail = tail->next;
        ++j;
    }
    Expression ex;
    std::vector<Token*> postExpression;
    ex.parse(head,  postExpression);
    EXPECT_EQ(17u, postExpression.size());
    char* pos[] = {"a", "b","c","*","+","h","d","e","+","*","x","sin","/","y","cos","*","+"};
    for (unsigned i = 0; i < postExpression.size(); ++i)
    {
        EXPECT_STREQ(pos[i], postExpression[i]->str);
    }
}
