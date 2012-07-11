#include "lexer.h"
#include <map>
#include <string>
using namespace mlplus;
using namespace std;
// 1 -->  '>'

Lexer::Lexer():mHead(NULL)
{
    registerTokenTable();
}

Lexer::~Lexer()
{
    if (mHead)
    {
        freeTokenList(mHead);
    }
}
Lexer::Lexer(const char* str):mHead(NULL)
{
    //TODO  init once
    registerTokenTable();
    mHead = scan(str);
}
void Lexer::registerTokenTable()
{
    mTokenTable["pow"] = OP_POW;
    mTokenTable["sin"] = OP_SIN;    
    mTokenTable["cos"] = OP_COS;    
    mTokenTable["tan"] = OP_TAN;
    mTokenTable["log"] = OP_LOG;
    mTokenTable["exp"] =  OP_EXP;
    mTokenTable["int"] = OP_INT;
    //{">=", "<=", "!= <>", ">", "<", "="}
    mTokenTable[">"] = OP_GT;
    mTokenTable["<"] = OP_LS;
    mTokenTable["!"] = OP_NOT;
    mTokenTable["="] = OP_EQ;

    mTokenTable[">="] = OP_GE;
    mTokenTable["<="] = OP_LE;
    mTokenTable["!="] = OP_NE;
    mTokenTable["<>"] = OP_NE;
    mTokenTable["=="] = OP_EQ;
    //and or not
    mTokenTable["and"] = OP_AND;
    mTokenTable["not"] = OP_NOT;
    mTokenTable["or"] =  OP_OR;
    mTokenTable["&&"] = OP_AND;
    mTokenTable["||"] = OP_OR;
    // + - * / %
    mTokenTable["+"] = OP_ADD;
    mTokenTable["-"] = OP_MINUS;
    mTokenTable["*"] = OP_PLUS;
    mTokenTable["/"] = OP_DIV;
    mTokenTable["%"] = OP_MOD;

    mTokenTable["+="] = OP_ADD_EQ;
    mTokenTable["-="] = OP_MINUS_EQ;
    mTokenTable["*="] = OP_PLUS_EQ;
    mTokenTable["/="] = OP_DIV_EQ;
    mTokenTable["%="] = OP_MOD_EQ;

    mTokenTable["++"] = OP_ADD_ADD;
    mTokenTable["--"] = OP_MINUS_MINUS;
    //()
    mTokenTable["("] = OP_LEFT;
    mTokenTable[")"] = OP_RIGHT;
    mTokenTable[","] = OP_COMMA;
}
Token* Lexer::append(Token* tail, Token* next)
{
    if (tail) 
    {
        tail->next = next;
        tail = next;
    }
    else
    {
        tail = next;
    }
    return tail;
}
void Lexer::freeTokenList(Token*& head) 
{
    while(head)
    {
        Token* t = head->next;
        free(head);
        head = t;
    }
}
Token*  Lexer::scan(const char* str)
{
    int len = 0;
    freeTokenList(mHead);
    Token* tail = NULL;
    const char *p = str;
    char c = *p;
    while(*p)
    {
        c = *p;
        switch(c)
        {
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':  //mode
        case '>':
        case '<':
        case '=':
        case '!':
        case '&':
        case '|':
            {
                len = 1;
                if (*(p + 1) == '=' || *(p + 1) == c)
                { 
                    len = 2; 
                }
            }
            break;
        case '(':  //int
        case ')':  //int
            {
                len = 1;
                break;
            }
        case ' ':
        case '\t':
            break;
        case '\'':
        case '"':
            {
                len = 1;
                while (*(p + len) && c != *(p + len)) len++;
                if (*(p + len))
                {
                    Token* t = new Token(OP_STRING);
                    strncpy(t->str, p + 1, len - 1);
                    t->str[len - 1] = '\0';
                    tail = append(tail, t);
                    mHead = mHead == NULL ? tail : mHead;
                    p = p + len;
                    len = 0;//skip ''
                }
            }
            break;
        default:
            {
                while (validVariableChar(*(p + 1 + len++)));
                break;
            }
        }
        if (len > 0)
        {
            Token* t = new Token;
            strncpy(t->str, p, len);
            t->str[len] = '\0';
            map<string, TokenType>::const_iterator it = mTokenTable.find(t->str);
            if (it == mTokenTable.end())
            {
                if (constValue(t->str))
                {
                    t->type = OP_CONST;
                }
                else
                {
                    t->type = OP_VAR;
                }
            }
            else
            {
                if (c == '+' || c == '-')
                {
                    if (!tail || isOperator(tail->type) || tail->type == OP_RIGHT)
                    {
                        if (it->second == OP_MINUS)
                        {
                            t->type = OP_MINUS_1;
                        }
                        else if (it->second == OP_ADD)
                        {
                            t->type = OP_ADD_1;
                        }
                    }
                    else
                    {
                        t->type = it->second;
                    }
                }
                else
                {
                    t->type = it->second;
                }
            }
            tail = append(tail, t);
            mHead = mHead == NULL ? tail : mHead;
            p += len;
        }
        else
        {
            p += 1;
        }
        len = 0;
    }
    return mHead;
}

bool Lexer::constValue(char* value) const
{
    char *p = value;
    bool r = false;
    if(isdigit(*p))
    {
        r = true;
    }
    while(isdigit(*p))
    {
        ++p;
    }
    if (*p == '.' || *p == 'e' || *p == 'E')
    {
        ++p;
    }
    while(isdigit(*p))
    {
        ++p;
    }
    return r && *p == 0;
}
