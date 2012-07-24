#ifndef __WEKACPP_CORE_CONVERTERS_ARFFLOADER__
#define __WEKACPP_CORE_CONVERTERS_ARFFLOADER__

#include "attribute.h"
#include "instance_interface.h"
#include "dataset.h"
#include <string>
#include <streambuf>

namespace mlplus
{

class ArffLoader
{
public:
    struct Token
    {
        enum
        {
            ERR,
            COMMA,
            LBRACK,
            RBRACK,
            LPAREN,
            RPAREN,
            LBRACE,
            RBRACE,
            IDENT,
            NUMBER,
            STRING,
            ATTRIBUTE,
            DATA,
            RELATION,
        } kind;
        std::string text;
        Token() : kind(ERR)
        {
        }
        std::string toString();
    };

private:
    enum
    {
        IN_ERR,
        IN_HEADER,
        IN_DATA,
    } mPhase;

    std::streambuf* mStream;
    bool mbHeaderRead;
    std::string mRelationName;
    vector<Attribute *> mAttributes;
    DataSet* mHeader;
    int mLine;

    Token getNextToken();
    void readHeader();

public:
    ArffLoader(streambuf * s);
    ~ArffLoader();
    DataSet* getStructure();
    DataSet* getDataSet();
    IInstance * getNextInstance();
};

} // namespace mlplus

#endif 
