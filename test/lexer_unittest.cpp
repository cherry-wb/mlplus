#include <string>
#include <vector>
#include "gtest/gtest.h"
#include <iostream>
#include "lexer.h"
using namespace std;
using namespace mlplus;

TEST(lexerTest, addTesting){

    string str = "-a + +b";
    Lexer ex;
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
}
TEST(lexerTest, logicExpression){
    string str = "a and b or c + b > 0";
    Lexer ex;
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
}

TEST(lexerTest, operatorExpression){
    string str = "a + b*c + int(b)%c";
    Lexer ex;
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

}
TEST(lexerTest, functionTest){
    string str = "int(log(sin(cos))) + sin(1)";
    char* op[] = {"int", "(", "log", "(", "sin", "(", "cos",")",")", ")", "+", "sin", "(", "1", ")"};
    Lexer ex;
    Token* head = ex.scan(str.c_str());
    Token* tk = head;
    int i = 0;
    while(tk != NULL)
    {
        EXPECT_STREQ(op[i], tk->str);
        tk = tk->next;
        ++i;
    }
}

TEST(lexerTest, stringTest){
    string str = "fun == \"abc\" and c <1.2";
    char* op[] = {"fun", "==", "abc", "and", "c", "<", "1.2"};
    Lexer ex;
    Token* head = ex.scan(str.c_str());
    Token* tk = head;
    int i = 0;
    while(tk != NULL)
    {
        EXPECT_STREQ(op[i], tk->str);
        tk = tk->next;
        ++i;
    }
}

TEST(lexerTest, errorTest){
    string str = "fun == \"\" or sin(x ==\"ab ";
    char* op[] = {"fun", "==", "", "or", "sin", "(", "x", "==", "\"ab "};
    Lexer ex;
    Token* head = ex.scan(str.c_str());
    Token* tk = head;
    int i = 0;
    while(tk != NULL)
    {
        EXPECT_STREQ(op[i], tk->str);
        tk = tk->next;
        ++i;
    }
}
TEST(lexerTest, allSymbleTable){
    string str = "+= -= *= %= ++";
    char* op[] = {"+=", "-=", "*=", "%=", "++"};
    Lexer ex;
    Token* head = ex.scan(str.c_str());
    Token* tk = head;
    int i = 0;
    while(tk != NULL)
    {
        EXPECT_STREQ(op[i], tk->str);
        tk = tk->next;
        ++i;
    }
}
