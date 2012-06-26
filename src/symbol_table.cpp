#include "symbol_table.h"
void  SymbolTable::defaultSymbol();
{
    symbol(")");
    symbol(",");
    symbol("(end)");
    literal("(literal)");
    literal("(name)");

    infix("or", 10);
    infix("and", 11);
    prefix("not", 12);

    infix("==", 20);
    infix("!=", 20);
    infix("<", 20);
    infix(">", 20);
    infix("<=", 20);
    infix("=>", 20);

    infix("+", 30);
    infix("-", 30);
    infix("{MINUS SIGN}", 30);
    infix("*", 40);
    infix("/", 40);
    infix("//", 40);

    infixr("**", 50);

    prefix("+", 70);
    prefix("-", 70);
    prefix("{MINUS SIGN}", 70);
}
