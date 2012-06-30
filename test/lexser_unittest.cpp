#include <string>
#include <vector>
#include "gtest/gtest.h"
#include <iostream>
#include "lexser.h"
#include "lexser.h"
using namespace std;
using namespace mlplus;

TEST(lexserTest, addTesting){

    string str = "-a + +b";
    Lexser ex;
    Token* head = ex.scan(str.c_str());
    Token* tk = head;
    EXPECT_EQ(tk->type, OP_MINUS_1);
    EXPECT_STREQ(tk->str, "-");
    tk = tk->next;
    EXPECT_EQ(tk->type, OP_VAR);
    EXPECT_STREQ(tk->str, "a");
    tk = tk->next;
    EXPECT_EQ(tk->type, OP_ADD);
    EXPECT_STREQ(tk->str, "+");
    tk = tk->next;
    EXPECT_EQ(tk->type, OP_ADD_1);
    EXPECT_STREQ(tk->str, "+");
    tk = tk->next;
    EXPECT_EQ(tk->type, OP_VAR);
    EXPECT_STREQ(tk->str, "b");
    ex.freeTokenList(head);
}
TEST(lexserTest, logicExpression){
    string str = "a and b or c + b > 0";
    Lexser ex;
    Token* head = ex.scan(str.c_str());
    Token* tk = head;
    EXPECT_EQ(tk->type, OP_VAR);
    EXPECT_STREQ(tk->str, "a");
    tk = tk->next;
    EXPECT_EQ(tk->type, OP_AND);
    EXPECT_STREQ(tk->str, "and");
    tk = tk->next;
    EXPECT_EQ(tk->type, OP_VAR);
    EXPECT_STREQ(tk->str, "b");
    tk = tk->next;
    EXPECT_EQ(tk->type, OP_OR);
    EXPECT_STREQ(tk->str, "or");
    tk = tk->next;
    EXPECT_EQ(tk->type, OP_VAR);
    EXPECT_STREQ(tk->str, "c");
    tk = tk->next;
    EXPECT_EQ(tk->type, OP_ADD);
    EXPECT_STREQ(tk->str, "+");

    tk = tk->next;
    EXPECT_EQ(tk->type, OP_VAR);
    EXPECT_STREQ(tk->str, "b");

    tk = tk->next;
    EXPECT_EQ(tk->type, OP_GT);
    EXPECT_STREQ(tk->str, ">");

    tk = tk->next;
    EXPECT_EQ(tk->type, OP_CONST);
    EXPECT_STREQ(tk->str, "0");

    tk = tk->next;
    EXPECT_TRUE(tk == 0);
    ex.freeTokenList(head);
}

TEST(lexserTest, operatorExpression){
    string str = "a + b*c + int(b)%c";
    Lexser ex;
    Token* head = ex.scan(str.c_str());
    Token* tk = head;
    EXPECT_EQ(tk->type, OP_VAR);
    EXPECT_STREQ(tk->str, "a");
    tk = tk->next;

    EXPECT_EQ(tk->type, OP_ADD);
    EXPECT_STREQ(tk->str, "+");
    tk = tk->next;
    EXPECT_EQ(tk->type, OP_VAR);
    EXPECT_STREQ(tk->str, "b");

    tk = tk->next;
    EXPECT_EQ(tk->type, OP_PLUS);
    EXPECT_STREQ(tk->str, "*");

    tk = tk->next;
    EXPECT_EQ(tk->type, OP_VAR);
    EXPECT_STREQ(tk->str, "c");

    tk = tk->next;
    EXPECT_EQ(tk->type, OP_ADD);
    EXPECT_STREQ(tk->str, "+");

    tk = tk->next;
    EXPECT_EQ(tk->type, OP_INT);
    EXPECT_STREQ(tk->str, "int");

    tk = tk->next;
    EXPECT_EQ(tk->type, OP_LEFT);
    EXPECT_STREQ(tk->str, "(");

    tk = tk->next;
    EXPECT_EQ(tk->type, OP_VAR);
    EXPECT_STREQ(tk->str, "b");

    tk = tk->next;
    EXPECT_EQ(tk->type, OP_RIGHT);
    EXPECT_STREQ(tk->str, ")");

    tk = tk->next;
    EXPECT_EQ(tk->type, OP_MOD);
    EXPECT_STREQ(tk->str, "%");

    tk = tk->next;
    EXPECT_EQ(tk->type, OP_VAR);
    EXPECT_STREQ(tk->str, "c");

    tk = tk->next;
    EXPECT_TRUE(tk == 0);

    ex.freeTokenList(head);
}
TEST(lexserTest, functionTest){
    string str = "int(log(sin(cos))) + sin(1)";
    char* op[] = {"int", "(", "log", "(", "sin", "(", "cos",")",")", ")", "+", "sin", "(", "1", ")"};
    Lexser ex;
    Token* head = ex.scan(str.c_str());
    Token* tk = head;
    int i = 0;
    while(tk != NULL)
    {
        EXPECT_STREQ(op[i], tk->str);
        tk = tk->next;
        ++i;
    }
    ex.freeTokenList(head);
}

TEST(lexserTest, stringTest){
    string str = "fun == \"abc\" and c <1.2";
    char* op[] = {"fun", "==", "abc", "and", "c", "<", "1.2"};
    Lexser ex;
    Token* head = ex.scan(str.c_str());
    Token* tk = head;
    int i = 0;
    while(tk != NULL)
    {
        EXPECT_STREQ(op[i], tk->str);
        tk = tk->next;
        ++i;
    }
    ex.freeTokenList(head);
}

TEST(lexserTest, errorTest){
    string str = "fun == \"\" or sin(x ==\"ab ";
    char* op[] = {"fun", "==", "", "or", "sin", "(", "x", "==", "\"ab "};
    Lexser ex;
    Token* head = ex.scan(str.c_str());
    Token* tk = head;
    int i = 0;
    while(tk != NULL)
    {
        EXPECT_STREQ(op[i], tk->str);
        tk = tk->next;
        ++i;
    }
    ex.freeTokenList(head);
}
TEST(lexserTest, allSymbleTable){
    string str = "+= -= *= %= ++";
    char* op[] = {"+=", "-=", "*=", "%=", "++"};
    Lexser ex;
    Token* head = ex.scan(str.c_str());
    Token* tk = head;
    int i = 0;
    while(tk != NULL)
    {
        EXPECT_STREQ(op[i], tk->str);
        tk = tk->next;
        ++i;
    }
    ex.freeTokenList(head);
}
