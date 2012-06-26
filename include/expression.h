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
    void parse(Token* head, std::vector<Token*>& postStack) const;
    bool verify(std::vector<Token*>& postStack) const;
    float evaluate(const Scope& scope);
private:
    void buildOperatorPriority();
    int operatorTable[OP_UNKNOW];
};
}
#endif
