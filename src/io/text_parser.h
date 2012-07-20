#ifndef MLPLUS_TXT_PARSER_H
#define MLPLUS_TXT_PARSER_H
#include "abstract_parser.h"
namespace mlplus
{
class TextParser: public AbstractParser
{
public:
    TextParser(const std::string& headerFileName);
    ~TextParser();
    /*override*/DataSet* readData(const std::string& filename);
    AttributeSpec* getAttributeSpec() {return mpSpec;}
    const std::string& getDelimiter() const {return mDelim;};
    void  setDelimiter(const std::string& st) {mDelim = st;};
private:
    AttributeSpec* mpSpec;
    std::string  mDelim; 
};
} // end of namespace mlplus
#endif 
