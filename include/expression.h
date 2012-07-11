#ifndef MLPLUS_EXPRESSION_H
#define MLPLUS_EXPRESSION_H
#include <inttypes.h>
#include <vector>
#include "lexer.h"
namespace mlplus
{
class Scope;
class Expression
{
public:
    Expression();
    Expression(const char*str);
    double evaluate(const Scope& scope);
    double evaluate(const char* str);
    double evaluate(const char* str, const Scope& scope);
    void parse(Token* head, std::vector<Token*>& postStack) const;
    bool verify(std::vector<Token*>& postStack) const;
    bool isLogicExpression() const;
    double evaluate(std::vector<Token*>& postStack,const Scope& scope);
private:
    std::vector<Token*> mPostStack;
    Lexer mLexer;
    void buildOperatorPriority();
    static int sOperatorTable[OP_UNKNOW];
};
}
#endif
