#include "expression.h"
#include "variant.h"
#include "scope.h"
#include <stack>
#include <map>
#include <cmath>
#include <string>
#include <vector>
using namespace mlplus;
using namespace std;

int Expression::sOperatorTable[OP_UNKNOW] = {0};
Expression::Expression(const char*str):mLexer(str)
{
    buildOperatorPriority();
}
double Expression::evaluate(const Scope& scope)
{
    parse(mLexer.getTokenHead(), mPostStack);
    return evaluate(mPostStack, scope);
}
Expression::Expression()
{
    buildOperatorPriority();
}
void Expression::parse(Token* head, vector<Token*>& postStack) const
{
    postStack.clear();
    stack<Token*> opStack;
    Token* top = NULL;
    while(head)
    {
        TokenType type = head->type;
        if (Lexer::isOperant(type))
        {
            postStack.push_back(head);
        }
        else if (opStack.empty())
        {
            opStack.push(head);
        }
        else
        {
            top = opStack.top();
            int topP = sOperatorTable[top->type];
            int cur = sOperatorTable[type];
            if (type == OP_RIGHT) //for ()
            {
                while(top->type != OP_LEFT)
                {
                    postStack.push_back(top);
                    opStack.pop();
                    if (opStack.empty())
                    {
                        break;
                    }
                    top = opStack.top();
                }
                if (!opStack.empty())
                {
                    opStack.pop();
                }
            } 
            else if (type == OP_LEFT)
            {
                opStack.push(head);
            }
            else if (cur < topP) //current operator with high priority
            {
                opStack.push(head);
            }
            else
            {
                while(cur >= topP)
                {
                    postStack.push_back(top);
                    opStack.pop();
                    if (opStack.empty())
                    {
                        break;
                    }
                    top = opStack.top();
                    topP = sOperatorTable[top->type];
                }
                if (head->type != OP_COMMA)//for function expression, don't support a,b,c
                {
                    opStack.push(head);
                }
            }
        }
        head = head->next;
    }
    if (!opStack.empty())
    {
        do
        {
            top = opStack.top();
            postStack.push_back(top);
            opStack.pop();
        }while(!opStack.empty());
    }
}

bool Expression::verify(std::vector<Token*>& postStack) const
{
    std::vector<Token*>::const_iterator rb = postStack.begin();
    int var = 0;
    for (;rb != postStack.end();++rb)
    {
        switch((*rb)->type)
        {
        case OP_SIN:
        case OP_COS:
        case OP_TAN:
        case OP_LOG:
        case OP_EXP:
        case OP_INT:
        case OP_ADD_1:
        case OP_MINUS_1:
        case OP_ADD_ADD:
        case OP_MINUS_MINUS:
        case OP_NOT:
            {
                if (var < 1)
                {
                    return false;
                }
                break;
            }
            //{">=": "<=": "!= <>": ">": "<": "="}
        case OP_POW:
        case OP_GE:
        case OP_LE:
        case OP_NE:
        case OP_GT:
        case OP_LS:
        case OP_EQ:
            //and or && ||
        case OP_AND:
        case OP_OR:
        case OP_ADD:
        case OP_MINUS:
        case OP_PLUS:
        case OP_DIV:
        case OP_MOD:
        case OP_ADD_EQ: 
        case OP_MINUS_EQ:
        case OP_PLUS_EQ:
        case OP_DIV_EQ:
        case OP_MOD_EQ:
            {
                if (var < 2)
                {
                    return false;
                }
                --var;
                break;
            }
        case OP_TRUE:
        case OP_FALSE:
        case OP_STRING:
        case OP_CONST:
        case OP_VAR:
            {
                ++var;
                break;
            }
        default:
            return false;
        }
    }
    return var == 1;
}

bool Expression::isLogicExpression() const
{
    if (mPostStack.empty())
    {
        return false;
    }
    Token* rb = mPostStack.back();
    switch(rb->type)
    {
    case OP_NOT:
    case OP_GE:
    case OP_LE:
    case OP_NE:
    case OP_GT:
    case OP_LS:
    case OP_EQ:
    case OP_AND:
    case OP_OR:
        return true;
    default:
        return false;
    }
    return false;
}
double Expression::evaluate(std::vector<Token*>& postStack, const Scope& scope)
{
    std::vector<Token*>::const_iterator rb = postStack.begin();
    int var = 0;
    stack<Variant> variables; 
    Variant left = 0;
    Variant right = 0;
    for (;rb != postStack.end();++rb)
    {
        switch((*rb)->type)
        {
        case OP_SIN:
            {
                left = variables.top(); 
                left.setDoubleValue(sin(left.asDouble()));
                variables.pop();
                variables.push(left);
                break;
            }
        case OP_COS:
            {
                left = variables.top(); 
                left.setDoubleValue(cos(left.asDouble()));
                variables.pop();
                variables.push(left);
                break;
            }
        case OP_TAN:
            {
                left = variables.top(); 
                left.setDoubleValue(tan(left.asDouble()));
                variables.pop();
                variables.push(left);
                break;
            }
        case OP_LOG:
            {
                left = variables.top(); 
                left.setDoubleValue(log(left.asDouble()));
                variables.pop();
                variables.push(left);
                break;
            }
        case OP_EXP:
            {
                left = variables.top(); 
                left.setDoubleValue(exp(left.asDouble()));
                variables.pop();
                variables.push(left);
                break;
            }
        case OP_INT:
            {
                left = variables.top(); 
                left.setDoubleValue(floor(left.asDouble()));
                variables.pop();
                variables.push(left);
                break;
            }
        case OP_ADD_1:
            {
                break;
            }
        case OP_MINUS_1:
            {
                left = variables.top(); 
                left.setDoubleValue(-(left.asDouble()));
                variables.pop();
                variables.push(left);
                break;
            }
        case OP_ADD_ADD:
            {
                left = variables.top(); 
                left.setDoubleValue(1+(left.asDouble()));
                variables.pop();
                variables.push(left);
                break;
            }
        case OP_MINUS_MINUS:
            {
                left = variables.top(); 
                left.setDoubleValue(left.asDouble() - 1);
                variables.pop();
                variables.push(left);
                break;
            }
        case OP_NOT:
            {
                left = variables.top(); 
                left.setDoubleValue((double)!left.asBoolean());
                variables.pop();
                variables.push(left);
                break;
            }
            //{">=": "<=": "!= <>": ">": "<": "="}
        case OP_GE:
            {
                right = variables.top(); 
                variables.pop();
                left = variables.top(); 
                variables.pop();
                left.setDoubleValue(double(left.compare(right)>=0));
                variables.push(left);
                break;
            }
        case OP_LE:
            {
                right = variables.top(); 
                variables.pop();
                left = variables.top(); 
                variables.pop();
                left.setDoubleValue(double(left.compare(right)<=0));
                variables.push(left);
                break;
            }
        case OP_NE:
            {
                right = variables.top(); 
                variables.pop();
                left = variables.top(); 
                variables.pop();
                left.setDoubleValue(double(left.compare(right)!=0));
                variables.push(left);
                break;
            }
        case OP_GT:
            {
                right = variables.top(); 
                variables.pop();
                left = variables.top(); 
                variables.pop();
                left.setDoubleValue(double(left.compare(right)>0));
                variables.push(left);
                break;
            }
        case OP_LS:
            {
                right = variables.top(); 
                variables.pop();
                left = variables.top(); 
                variables.pop();
                left.setDoubleValue(double(left.compare(right)<0));
                variables.push(left);
                break;
            }
        case OP_EQ:
            {
                right = variables.top(); 
                variables.pop();
                left = variables.top(); 
                variables.pop();
                left.setDoubleValue(double(left.compare(right)==0));
                variables.push(left);
                break;
            }
            //and or && ||
        case OP_AND:
            {
                right = variables.top(); 
                variables.pop();
                left = variables.top(); 
                variables.pop();
                left.setDoubleValue(double(left.asBoolean() && right.asBoolean()));
                variables.push(left);
                break;
            }
        case OP_OR:
            {
                right = variables.top(); 
                variables.pop();
                left = variables.top(); 
                variables.pop();
                left.setDoubleValue(double((bool)left || (bool)right));
                variables.push(left);
                break;
            }
        case OP_ADD:
        case OP_ADD_EQ: 
            {
                right = variables.top(); 
                variables.pop();
                left = variables.top(); 
                variables.pop();
                left.setDoubleValue((double)left + (double)right);
                variables.push(left);
                break;
            }
        case OP_MINUS:
        case OP_MINUS_EQ:
            {
                right = variables.top(); 
                variables.pop();
                left = variables.top(); 
                variables.pop();
                left.setDoubleValue((double)left - (double)right);
                variables.push(left);
                break;
            }
        case OP_POW:
            {
                right = variables.top(); 
                variables.pop();
                left = variables.top(); 
                variables.pop();
                left.setDoubleValue(pow((double)left, (double)right));
                variables.push(left);
                break;
            }
        case OP_PLUS:
        case OP_PLUS_EQ:
            {
                right = variables.top(); 
                variables.pop();
                left = variables.top(); 
                variables.pop();
                left.setDoubleValue((double)left * (double)right);
                variables.push(left);
                break;
            }
        case OP_DIV:
        case OP_DIV_EQ:
            {
                right = variables.top(); 
                variables.pop();
                left = variables.top(); 
                variables.pop();
                left.setDoubleValue((double)left / (double)right);
                variables.push(left);
                break;
            }
        case OP_MOD:
        case OP_MOD_EQ:
            {
                right = variables.top(); 
                variables.pop();
                left = variables.top(); 
                variables.pop();
                left.setDoubleValue((int64_t)left % (int64_t)right);
                variables.push(left); 
                break;
            }
        case OP_TRUE:
            {
                variables.push(true);
                ++var;
                break;
            }
        case OP_FALSE:
            {
                variables.push(false);
                ++var;
                break;
            }
        case OP_STRING:
            {
                variables.push((*rb)->str);
                ++var;
                break;
            }
        case OP_CONST:
            {
                float a = atof((*rb)->str);
                variables.push(a);
                ++var;
                break;
            }
        case OP_VAR:
            {
                const Variant& t = scope.find((*rb)->str);
                assert(!t.isNULL());
                variables.push(atof((*rb)->str));
                ++var;
                break;
            }
        default:
            return 0;
        }
    }
    return variables.top().asDouble();
}
void  Expression::buildOperatorPriority()
{
    static bool init = false;
    if (!init)
    {
        //"pow" "sin" "cos"] = "tan" "log" "exp" "int"
        sOperatorTable[OP_POW] = 1;
        sOperatorTable[OP_SIN] = 1;
        sOperatorTable[OP_COS] = 1;
        sOperatorTable[OP_TAN] = 1;
        sOperatorTable[OP_LOG] = 1;
        sOperatorTable[OP_EXP] = 1;
        sOperatorTable[OP_INT] = 1;
        //{">=" "<="] = "!= <>" ">" "<" "="}
        sOperatorTable[OP_GE] = 7;
        sOperatorTable[OP_LE] = 7;
        sOperatorTable[OP_NE] = 8;
        sOperatorTable[OP_GT] = 7;
        sOperatorTable[OP_LS] = 7;
        sOperatorTable[OP_EQ] = 8;
        //and or not && ||
        sOperatorTable[OP_AND] = 12;
        sOperatorTable[OP_OR] = 13;
        sOperatorTable[OP_NOT] = 2;
        // + - * / %
        sOperatorTable[OP_ADD_1] = 2;
        sOperatorTable[OP_MINUS_1] = 2;
        sOperatorTable[OP_ADD] = 5;
        sOperatorTable[OP_MINUS] = 5;
        sOperatorTable[OP_PLUS] = 4;
        sOperatorTable[OP_DIV] =  4;
        sOperatorTable[OP_MOD] = 4;

        sOperatorTable[OP_ADD_EQ] = 15;
        sOperatorTable[OP_MINUS_EQ] = 15;
        sOperatorTable[OP_PLUS_EQ] = 15;
        sOperatorTable[OP_DIV_EQ] = 15;
        sOperatorTable[OP_MOD_EQ] = 15;
        sOperatorTable[OP_COMMA] = 18;

        sOperatorTable[OP_ADD_ADD] = 2;
        sOperatorTable[OP_MINUS_MINUS] = 2;
        //()
        sOperatorTable[OP_LEFT] = 100;
        sOperatorTable[OP_RIGHT] = 100;
        //bool
        sOperatorTable[OP_TRUE] = 0;
        sOperatorTable[OP_FALSE] = 0;
        sOperatorTable[OP_STRING] = 0;
        sOperatorTable[OP_CONST] = 0;
        sOperatorTable[OP_VAR] = 0;
        init = true;
    }
}

double Expression::evaluate(const char* str, const Scope& scope)
{
    mLexer.scan(str);
    parse(mLexer.getTokenHead(), mPostStack);
    return evaluate(mPostStack, scope);
}
double Expression::evaluate(const char* str)
{
    return evaluate(str, Scope());
}
