#pragma warning( disable : 4786 )
#ifndef MLPLUS_ARFF_PARSER_H
#define MLPLUS_ARFF_PARSER_H
#include <fstream>
#include <sstream>
#include "abstract_parser.h"
#include "input_data.h"

using namespace std;

namespace mlplus
{

class ArffParser : public AbstractParser
{
public:
    ArffParser(const std::string& headerFileName);
    /*override*/virtual DataSet* readData(const std::string& filename);
protected:
    void readHeader(ifstream& in);

    void readData(ifstream& in, vector<Example>& examples, NameMap& classMap,
                  vector<NameMap>& enumMaps,
                  const vector<RawData::eAttributeType>& attributeTypes);

    string readName(istream& in);

    void readDenseValues(istream& in);

    void readSparseValues(istream& ss);

    /**
     * Read labels declared in the standard arff format:
     * each class (label) is set to +1 if it's there, otherwise is -1
     * i.e.:
     * \verbatim
     @ATTRIBUTE sepallength  NUMERIC
     @ATTRIBUTE sepalwidth   NUMERIC
     @ATTRIBUTE petallength  NUMERIC
     @ATTRIBUTE petalwidth   NUMERIC
     @ATTRIBUTE class        {Iris-setosa, Iris-versicolor, Iris-virginica}
     @DATA
     4,  2,  4,  2, Iris-setosa
     25, 23,  1,  0, Iris-versicolor, Iris-virginica
     0,  1, 10, 12, Iris-virginica
     \endverbatim
     * In this case the labels the the example will respectively be:
     \verbatim
     +1, -1, -1
     -1, +1, +1
     -1, -1, +1
     \endverbatim
     */
    void readSimpleLabels(istringstream& ss, vector<Label>& labels, NameMap& classMap);

    /**
     * Read sparse labels declared in a non-standard arff variant:
     * each class (label) is declared with a value that can be positive, or negative
     * left out labels are automatically considered zero (abstention!).
     * i.e.:
     * \verbatim
     @ATTRIBUTE sepallength  NUMERIC
     @ATTRIBUTE sepalwidth   NUMERIC
     @ATTRIBUTE petallength  NUMERIC
     @ATTRIBUTE petalwidth   NUMERIC
     @ATTRIBUTE class        {Iris-setosa, Iris-versicolor, Iris-virginica}
     @DATA
     4,  2,  4,  2, {Iris-setosa -2}
     25, 23,  1,  0, {Iris-versicolor 1, Iris-virginica -1}
     0,  1, 10, 12, {Iris-setosa +2, Iris-versicolor -1, Iris-virginica -3}
     \endverbatim
     * The sign is used to set the value of y[l], and the magnitude to initialize the weights.
     * In this case the labels the the example will respectively be:
     \verbatim
     -1, 0, 0
     0, +1, -1
     +1, -1, -1
     \endverbatim
     * \remark Internally this type of label is stored as sparse. This will have
     * a small hit in terms of memory, but nothing in terms of performance.
     */
    void readExtendedLabels(istream& ss, vector<Label>& labels, NameMap& classMap);

    enum eTokenType
    {
        TT_EOF,
        TT_COMMENT,
        TT_RELATION,
        TT_ATTRIBUTE,
        TT_DATA,
        TT_UNKNOWN
    };

    eTokenType getNextTokenType(ifstream& in);

    int            _numAttributes;

    locale         _denseLocale;
    locale         _sparseLocale;
    bool           _hasName;
};

// -----------------------------------------------------------------------------

inline string ArffParser::readName(std::ifstream& in)
{
    const locale& originalLocale = in.imbue(_denseLocale);
    string name;
    in >> name;
    in.imbue(originalLocale);
    return name;
}


} // end of namespace mlplus

#endif // 
