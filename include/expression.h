#ifndef MLPLUS_EXPRESSION_H
#define MLPLUS_EXPRESSION_H
#include <inttypes.h>
#include <vector>
#include "lexser.h"
namespace mlplus
{
class Scope;
class Expression
{
public:
    Expression();
    double evaluate(const char* str);
    double evaluate(const char* str, const Scope& scope);
    void parse(Token* head, std::vector<Token*>& postStack) const;
    bool verify(std::vector<Token*>& postStack) const;
    double evaluate(std::vector<Token*>& postStack,const Scope& scope);
private:
    void buildOperatorPriority();
    int operatorTable[OP_UNKNOW];
};
}
#endif
