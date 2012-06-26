#ifndef MLPLUS_LEXSER_H
#define MLPLUS_LEXSER_H
#include <inttypes.h>
#include <map>
#include "lexser.h"
namespace mlplus
{

struct Symbol
{
public:
   virtual nud(Symbol*);
   virtual Symbol* led(Symbol*);
   std::string id;
   int lbp;
};
class SymbolTable
{
public:
    void defaultSymbol();
    void symbol(const char* p, int id = 0);
    std::map<std::string, Symbol> mSymbolTable;

};
}
#endif
