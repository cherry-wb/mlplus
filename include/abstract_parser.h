#ifndef MLPLUS_ABSTRACT_PARSER_H
#define MLPLUS_ABSTRACT_PARSER_H
#include <vector>
#include <string>
namespace mlplus
{
class AttributeSpec;
class DataSet;
class AbstractParser
{
public:
    AbstractParser(const std::string& header):mHeaderFileName(header)
    {
    }
    virtual ~AbstractParser(){}
    virtual DataSet* readData(const std::string& filename) = 0;
protected:
    const std::string mHeaderFileName;
};
}
#endif
