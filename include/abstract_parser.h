#ifndef MLPLUS_ABSTRACT_PARSER_H
#define MLPLUS_ABSTRACT_PARSER_H
#include <vector>
namespace mlplus
{

class AbstractParser
{
public:
    /**
     * initializes the data file and the header file.
     */
    AbstractParser(const string& fileName, const string& headerFileName)
        :mDataFileName(fileName), mHeaderFileName(headerFileName), mpDataset(NULL)
    {
    }
    virtual void readData() = 0;
    DataSet*  getDataSet() 
    { 
        return mpDataset;
    }
protected:
    const string& mDataFileName;
    const string& mHeaderFileName;
    DataSet*  mpDataset;
}
