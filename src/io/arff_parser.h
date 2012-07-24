#pragma warning( disable : 4786 )


#ifndef __ARFF_PARSER_H
#define __ARFF_PARSER_H

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
    /**
     * The constructor. It initializes the file names and the separators.
     * \date 30/07/2010
     */
    ArffParser(const string& headerFileName);
    virtual void readData(const std::string& filename);
    virtual int  getNumAttributes() const
    {
        return _numAttributes;
    }
protected:
    /**
     * Read the header. It reads the class labels, attribute names, attribute types and
     * the mappings of nominal features.
     * @param in The file stream.
     * @param \see AbstractParser::readData
     */
    void readHeader(ifstream& in);

    /**
     * Read the data. The arff can be sparse and dense as well.
     * \param in The file stream.
     * \param \see AbstractParser::readData
     * \data 30/07/2011
     */
    void readData(ifstream& in, vector<Example>& examples, NameMap& classMap,
                  vector<NameMap>& enumMaps,
                  const vector<RawData::eAttributeType>& attributeTypes);

    string readName(ifstream& in);

    void readDenseValues(ifstream& in, vector<FeatureReal>& values, vector<NameMap>& enumMaps,
                         const vector<RawData::eAttributeType>& attributeTypes);

    void readSparseValues(istringstream& ss, vector<FeatureReal>& values, vector<int>& idxs, map<int, int>& idxmap,
                          vector<NameMap>& enumMaps,
                          const vector<RawData::eAttributeType>& attributeTypes);

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
    void readExtendedLabels(istringstream& ss, vector<Label>& labels, NameMap& classMap);

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
    string         _headerFileName;

    locale         _denseLocale;
    locale         _sparseLocale;
    bool           _hasName;
};

// -----------------------------------------------------------------------------

inline string ArffParser::readName(ifstream& in)
{
    const locale& originalLocale = in.imbue(_denseLocale);
    string name;
    in >> name;
    in.imbue(originalLocale);
    return name;
}

// -----------------------------------------------------------------------------

} // end of namespace mlplus

#endif // __ARFF_PARSER_H
