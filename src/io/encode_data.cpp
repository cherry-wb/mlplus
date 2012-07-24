#include "encode_data.h"


//
namespace mlplus
{

// ------------------------------------------------------------------------

void EncodeData::resetData()
{
    _pData->clearRawData();
}


// ------------------------------------------------------------------------

void EncodeData::addExample(Example example)
{
    _pData->addExample(example);
}

} // end of namespace mlplus
