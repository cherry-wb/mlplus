#ifndef MLPLUS_ARFF_SAVER_H 
#define MLPLUS_ARFF_SAVER_H 

#include <string>
#include <streambuf>

namespace mlplus
{

class ArffSaver
{
    std::streambuf* mStream;
    bool mbHeaderWritten;
    DataSet* mHeader;

    void writeHeader(Instances * header);
    void writeInstance(Instance * instance);

public:
    ArffSaver(streambuf * s);
    ~ArffSaver();

    void writeBatch(Instances * instances);
    void writeIncremental(Instance * instance);
};

} // namespace mlplus

#endif 
