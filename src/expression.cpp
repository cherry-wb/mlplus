#include "expression.h"
#include <stack>
#include <map>
#include <string>
#include <vector>
using namespace mlplus;
using namespace std;

Expression::Expression()
{
    buildOperatorPriority();
}
void Expression::parse(Token* head, vector<Token*>& postStack) const
{
    stack<Token*> opStack;
    Token* top = NULL;
    while(head)
    {
        TokenType type = head->type;
        if (Lexser::isOperant(type))
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
            int topP = operatorTable[top->type];
            int cur = operatorTable[type];
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
                    topP = operatorTable[top->type];
                }
                opStack.push(head);
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
        case OP_POW:
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
void  Expression::buildOperatorPriority()
{
    //"pow" "sin" "cos"] = "tan" "log" "exp" "int"
    operatorTable[OP_POW] = 1;
    operatorTable[OP_SIN] = 1;
    operatorTable[OP_COS] = 1;
    operatorTable[OP_TAN] = 1;
    operatorTable[OP_LOG] = 1;
    operatorTable[OP_EXP] = 1;
    operatorTable[OP_INT] = 1;
    //{">=" "<="] = "!= <>" ">" "<" "="}
    operatorTable[OP_GE] = 7;
    operatorTable[OP_LE] = 7;
    operatorTable[OP_NE] = 8;
    operatorTable[OP_GT] = 7;
    operatorTable[OP_LS] = 7;
    operatorTable[OP_EQ] = 8;
    //and or not && ||
    operatorTable[OP_AND] = 12;
    operatorTable[OP_OR] = 13;
    operatorTable[OP_NOT] = 2;
    // + - * / %
    operatorTable[OP_ADD_1] = 2;
    operatorTable[OP_MINUS_1] = 2;
    operatorTable[OP_ADD] = 5;
    operatorTable[OP_MINUS] = 5;
    operatorTable[OP_PLUS] = 4;
    operatorTable[OP_DIV] =  4;
    operatorTable[OP_MOD] = 4;

    operatorTable[OP_ADD_EQ] = 15;
    operatorTable[OP_MINUS_EQ] = 15;
    operatorTable[OP_PLUS_EQ] = 15;
    operatorTable[OP_DIV_EQ] = 15;
    operatorTable[OP_MOD_EQ] = 15;

    operatorTable[OP_ADD_ADD] = 2;
    operatorTable[OP_MINUS_MINUS] = 2;
    //()
    operatorTable[OP_LEFT] = 100;
    operatorTable[OP_RIGHT] = 100;
    //bool
    operatorTable[OP_TRUE] = 0;
    operatorTable[OP_FALSE] = 0;
    operatorTable[OP_STRING] = 0;
    operatorTable[OP_CONST] = 0;
    operatorTable[OP_VAR] = 0;
}
