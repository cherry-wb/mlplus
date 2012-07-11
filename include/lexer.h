#ifndef MLPLUS_LEXSER_H
#define MLPLUS_LEXSER_H
#include <inttypes.h>
#include <map>
#include <string>
namespace mlplus
{
enum TokenType
{
    //"pow" "sin", "cos", "tan", "log", "exp", "int"
    OP_POW = 0,
    OP_SIN,    
    OP_COS,    
    OP_TAN,
    OP_LOG,
    OP_EXP,
    OP_INT,
    //{">=", "<=", "!= <>", ">", "<", "="}
    OP_GE,
    OP_LE,
    OP_NE,
    OP_GT,
    OP_LS,
    OP_EQ,
    //and or not && ||
    OP_AND,
    OP_NOT,
    OP_OR,
    // + - * / %
    OP_ADD,
    OP_MINUS,
    OP_ADD_1,
    OP_MINUS_1,
    OP_PLUS,
    OP_DIV,
    OP_MOD,
    OP_ADD_EQ, 
    OP_MINUS_EQ,
    OP_PLUS_EQ,
    OP_DIV_EQ,
    OP_MOD_EQ,

    OP_ADD_ADD,
    OP_MINUS_MINUS,
    //()
    OP_LEFT,
    OP_RIGHT,
    OP_COMMA,
    //bool
    OP_TRUE,
    OP_FALSE,
    OP_STRING,
    OP_CONST,
    OP_VAR,
    OP_UNKNOW
};
class Token
{ 
public:
    TokenType type; //name[reserved function or variable] string bool number operator
    //value
    char str[256];//for string type
    Token* next;
    Token(TokenType t = OP_UNKNOW):type(t), next(0)
    {
        str[0] = '\0';
    }
};
class Lexer
{
public:
    Lexer();
    Lexer(const char*);
    virtual ~Lexer();
    Token* scan(const char* input);
    inline static bool validVariableChar(char c);
    inline static bool isOperator(TokenType);
    inline static bool isOperant(TokenType);
    bool constValue(char* value) const;
    inline Token* getTokenHead();
private:
    void registerTokenTable();
    static void freeTokenList(Token*& head);
    static Token* append(Token* tail, Token* next);
    Token* mHead;
    std::map<std::string, TokenType> mTokenTable;
};

inline Token*  Lexer::getTokenHead()
{
    return mHead;
}
inline bool Lexer::validVariableChar(char c) 
{
    return (c >= 'a' && c  <= 'z') || (c >= 'A' && c >= 'Z') || '_' == c || '.' == c || (c>='0' && c<= '9');
}
inline bool Lexer::isOperator(TokenType type) 
{
    return (type != OP_STRING && type != OP_VAR && type != OP_CONST && type != OP_FALSE && type != OP_TRUE);
}
inline bool  Lexer::isOperant(TokenType type)
{
    return !isOperator(type);
}
}
#endif
