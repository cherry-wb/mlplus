#ifndef __ENCODE_DATA_H
#define __ENCODE_DATA_H

#include "io/input_data.h"

#include <vector>
#include <utility> // for pair

using namespace std;

namespace mlplus
{

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

/**
* Overloading of the InputData class to support the --encode option.
* \date 25/04/2007
*/
class EncodeData : public InputData
{
public:

    /**
    * The destructor. Must be declared (virtual) for the proper destruction of
    * the object.
    * \date 25/04/2007
    */
    virtual ~EncodeData() {}

    /**
    * Clear the _data field and set _numExamples to 0.
    * \date 25/04/2007
    */
    void resetData();

    /**
    * Push back an example to the _data field and increment _numExamples.
    * Be careful: weight initialization is not done in this function.
    * \date 25/04/2007
    */
    void addExample(Example example);

};

} // end of namespace mlplus

#endif // __ENCODE_DATA_H
