#ifndef MLPLUS_VARIANT_H
#define MLPLUS_VARIANT_H
#include <string>
#include <cassert>
#include <sstream>
#include "datetime.h"
namespace mlplus 
{
enum VariantType 
{
    VT_INTEGER,
    VT_DOUBLE,
    VT_BOOLEAN,
    VT_STRING,
    VT_DATETIME,
    VT_NULL,
};
class Variant
{
public:
    Variant();
    Variant(int64_t i);
    Variant(int i);
    Variant(double d);
    Variant(bool b);
    Variant(const char* s);
    Variant(const unsigned char* s);
    Variant(const DateTime& dateTime);
    Variant(const std::string& s, bool isEncodedStr = false);
    inline void setNullValue();
    inline void setIntegerValue(int64_t i); 
    inline void setDoubleValue(double d); 
    inline void setBooleanValue(bool b); 
    inline void setStringValue(const char* s, uint32_t len);
    inline void setDateTimeValue(uint64_t dateTimeTicks);
    inline bool isPOD() const;
    inline bool isNULL() const;
    int64_t asIntegerStrict() const;
    bool asBooleanStrict() const;
    double asDoubleStrict() const;
    int64_t asInteger() const;
    bool asBoolean() const;
    double asDouble() const;
    std::string asString() const;
    DateTime asDateTime() const;
    operator int64_t() const;
    operator bool() const;
    operator double() const;
    operator std::string() const;
    operator DateTime() const;
    VariantType getType() const;
    const std::string& toEncodedString() const;
    void fromEncodedString(const std::string& src);
    Variant addOne() const;
    int32_t compare(const Variant& variant) const;
private:
    void encodeString(const std::string& str)
    {
        uint32_t len = str.size();
        mEncodedString.resize(len + sizeof(uint8_t) + sizeof(uint32_t));
        char* data = &mEncodedString[0];
        *data = mType;
        data += sizeof(uint8_t);
        *(uint32_t*)data = len;
        data += sizeof(uint32_t);
        memcpy(data, str.c_str(), len);
    }
    std::string decodeString() const
    {
        const char* data = mEncodedString.data();
        data += sizeof(uint8_t);
        uint32_t len =  *(uint32_t*)data;
        data += sizeof(uint32_t);
        return std::string(data, len);
    }
    VariantType mType;
    std::string mEncodedString;
};
inline Variant::Variant()
{
    mType = VT_NULL;
    //Encode
    mEncodedString.resize(sizeof(uint8_t));
    mEncodedString[0] = (uint8_t)mType;
}
inline Variant::Variant(int64_t i)
{
    mType = VT_INTEGER;
    //Encode
    uint32_t len = sizeof(uint8_t) + sizeof(int64_t);
    mEncodedString.resize(len);
    char* data = &mEncodedString[0];
    *(uint8_t*)data = (uint8_t)mType;
    data += sizeof(uint8_t);
    *(int64_t*)data = i;
}
inline Variant::Variant(int i) 
{
    mType = VT_INTEGER;
    //Encode
    uint32_t len = sizeof(uint8_t) + sizeof(int64_t);
    mEncodedString.resize(len);
    char* data = &mEncodedString[0];
    *(uint8_t*)data = (uint8_t)mType;
    data += sizeof(uint8_t);
    *(int64_t*)data = i;
}
inline Variant::Variant(double d)  
{
    mType = VT_DOUBLE;
    //Encode
    uint32_t len = sizeof(uint8_t) + sizeof(double);
    mEncodedString.resize(len);
    char* data = &mEncodedString[0];
    *(uint8_t*)data = (uint8_t)mType;
    data += sizeof(uint8_t);
    *(double*)data = d;
}
inline Variant::Variant(bool b) 
{
    mType = VT_BOOLEAN;
    //Encode
    uint32_t len = sizeof(uint8_t) + sizeof(bool);
    mEncodedString.resize(len);
    char* data = &mEncodedString[0];
    *(uint8_t*)data = (uint8_t)mType;
    data += sizeof(bool);
    *(bool*)data = b;
}
inline Variant::Variant(const char* s) 
{
    mType = VT_STRING;
    //Encode
    uint32_t len = strlen(s);
    mEncodedString.resize(len + sizeof(uint8_t) + sizeof(uint32_t));
    char* data = &mEncodedString[0];
    *data = mType;
    data += sizeof(uint8_t);
    *(uint32_t*)data = len;
    data += sizeof(uint32_t);
    memcpy(data, s, len);
}
inline Variant::Variant(const unsigned char* s) 
{
    mType = VT_STRING;
    //Encode
    uint32_t len = strlen((const char*)s);
    mEncodedString.resize(len + sizeof(uint8_t) + sizeof(uint32_t));
    char* data = &mEncodedString[0];
    *data = mType;
    data += sizeof(uint8_t);
    *(uint32_t*)data = len;
    data += sizeof(uint32_t);
    memcpy(data, s, len);
}
inline Variant::Variant(const DateTime& dateTime) 
{
    mType = VT_DATETIME;
    //Encode
    uint32_t len = sizeof(uint8_t) + sizeof(int64_t);
    mEncodedString.resize(len);
    char* data = &mEncodedString[0];
    *(uint8_t*)data = (uint8_t)mType;
    data += sizeof(uint8_t);
    *(int64_t*)data = dateTime.getTicks();
}
inline Variant::Variant(const std::string& s, bool isEncodedStr)
{
    if (isEncodedStr == true)
    {
        mEncodedString = s;
        mType = (VariantType)(s[0]);
        return;
    }
    mType = VT_STRING;
    //Encode
    encodeString(s);
}

inline bool Variant::isNULL() const
{
    return mType == VT_NULL;
}
inline bool Variant::isPOD() const
{
    switch (mType)
    {
    case VT_INTEGER:
    case VT_DOUBLE:
    case VT_BOOLEAN:
        return true;
    case  VT_STRING:
    case  VT_DATETIME:
    case  VT_NULL:
        return false;
    }
    return false;
}
inline void Variant::setNullValue()
{
    mType = VT_NULL;
    //Encode
    mEncodedString.resize(sizeof(uint8_t));
    mEncodedString[0] = (uint8_t)mType;
}
inline void Variant::setIntegerValue(int64_t i)
{   
    mType = VT_INTEGER;
    //Encode
    uint32_t len = sizeof(uint8_t) + sizeof(int64_t);
    mEncodedString.resize(len);
    char* data = &mEncodedString[0];
    *(uint8_t*)data = (uint8_t)mType;
    data += sizeof(uint8_t);
    *(int64_t*)data = i;
}
inline void Variant::setDoubleValue(double d)
{   
    mType = VT_DOUBLE;
    //Encode
    uint32_t len = sizeof(uint8_t) + sizeof(double);
    mEncodedString.resize(len);
    char* data = &mEncodedString[0];
    *(uint8_t*)data = (uint8_t)mType;
    data += sizeof(uint8_t);
    *(double*)data = d;
}
inline void Variant::setBooleanValue(bool b)
{   
    mType = VT_BOOLEAN;
    //Encode
    uint32_t len = sizeof(uint8_t) + sizeof(bool);
    mEncodedString.resize(len);
    char* data = &mEncodedString[0];
    *(uint8_t*)data = (uint8_t)mType;
    data += sizeof(bool);
    *(bool*)data = b;
}
inline void Variant::setStringValue(const char* s, uint32_t len)
{
    mType = VT_STRING;
    //Encode
    mEncodedString.resize(len + sizeof(uint8_t) + sizeof(uint32_t));
    char* data = &mEncodedString[0];
    *data = mType;
    data += sizeof(uint8_t);
    *(uint32_t*)data = len;
    data += sizeof(uint32_t);
    memcpy(data, s, len);
}
inline void Variant::setDateTimeValue(uint64_t dateTimeTicks)
{   
    mType = VT_DATETIME;
    //Encode
    uint32_t len = sizeof(uint8_t) + sizeof(uint64_t);
    mEncodedString.resize(len);
    char* data = &mEncodedString[0];
    *(uint8_t*)data = (uint8_t)mType;
    data += sizeof(uint8_t);
    *(uint64_t*)data = dateTimeTicks;
}
inline Variant::operator int64_t() const
{
    return asInteger();
}
inline Variant::operator bool() const
{
    return asBoolean();
}
inline Variant::operator double() const
{
    return asDouble();
}
inline Variant::operator std::string() const
{
    return asString();
}
inline Variant::operator DateTime() const
{
    return asDateTime();
}
inline int64_t Variant::asIntegerStrict() const
{
    assert(mType == VT_INTEGER);
    //Decode
    const char* data = &mEncodedString[0] + sizeof(uint8_t);
    return *(int64_t*)data;
}
inline bool Variant::asBooleanStrict() const
{
    assert(mType == VT_BOOLEAN);
    //Decode
    const char* data = &mEncodedString[0] + sizeof(uint8_t);
    return *(bool*)data;
}
inline double Variant::asDoubleStrict() const
{
    assert(mType == VT_DOUBLE);
    //Decode
    const char* data = &mEncodedString[0] + sizeof(uint8_t);
    return *(double*)data;
}
inline std::string Variant::asString() const
{
    assert(mType == VT_STRING);
    //Decode
    return decodeString();
}
inline DateTime Variant::asDateTime() const
{
    assert(mType == VT_DATETIME);
    //Decode
    const char* data = &mEncodedString[0] + sizeof(uint8_t);
    return DateTime(*(uint64_t*)data);
}
inline VariantType Variant::getType() const
{
    return mType;
}
inline const std::string& Variant::toEncodedString() const
{
    return mEncodedString;
}
inline void Variant::fromEncodedString(const std::string& src)
{
    mEncodedString = src;
    mType = (VariantType)mEncodedString[0];
}
/** ATTENTION!!!
 *  This method will return a Variant with value added rather than change itself. e.g.
 *  Variant v("abc");
 *  Wrong ==> v.addOne()
 *  Right ==> v = v.addOne()
 */
inline Variant Variant::addOne() const
{
    switch (getType())
    {
    case VT_STRING:
        {
            std::string value = asString();
            value.push_back('\0');
            return Variant(value);
        }
    case VT_INTEGER:
        {
            int64_t value = asIntegerStrict();
            ++value;
            return Variant(value);
        }
    case VT_DOUBLE:
        {
            double value = asDoubleStrict();
            return Variant(value + 0.000001);
        }
    case VT_DATETIME:
        {
            uint64_t value = asDateTime().getTicks();
            ++value;
            return Variant(DateTime(value));
        }
    default:
        {
            assert(0);
        }
    }
}
inline int32_t Variant::compare(const Variant& variant) const
{
    const std::string& lStr = mEncodedString;
    const std::string& rStr = variant.toEncodedString();
    const char* pLValue = lStr.data();
    const char* pRValue = rStr.data();
    VariantType lType = (VariantType)pLValue[0];
    VariantType rType = (VariantType)pRValue[0];
    assert(lType == rType);
    pLValue += sizeof(uint8_t);
    pRValue += sizeof(uint8_t);
    int32_t result;
    switch (lType)
    {
    case VT_STRING:
        {
            int lPKeyLen = *(uint32_t*)pLValue;
            int rPKeyLen = *(uint32_t*)pRValue;
            pLValue += sizeof(uint32_t);
            pRValue += sizeof(uint32_t);
            result = memcmp(pLValue, pRValue, lPKeyLen < rPKeyLen ? lPKeyLen : rPKeyLen);
            if (result == 0)
                result = lPKeyLen == rPKeyLen ? 0 : (lPKeyLen < rPKeyLen ? -1 : 1);
            return result;
        }
    case VT_INTEGER:
    case VT_DOUBLE:
    case VT_BOOLEAN:
        {
            double lValue = asDouble();
            double rValue = asDouble();
            result = lValue > rValue ? 1 : (lValue < rValue ? -1 : 0);
            return result;
        }
    case VT_DATETIME:
        {
            uint64_t lValue = *(uint64_t*)pLValue;
            uint64_t rValue = *(uint64_t*)pRValue;
            result = lValue > rValue ? 1 : (lValue < rValue ? -1 : 0);
            return result;
        }
    default:
        return 0;
    }
}

inline int64_t Variant::asInteger() const
{
    int64_t result = 0;
    switch (getType())
    {
    case VT_INTEGER:
        result = asIntegerStrict();
        break;
    case VT_BOOLEAN:
        result = (int64_t)asBooleanStrict();
        break;
    case VT_DOUBLE:
        result = (int64_t)asDoubleStrict();
        break;
    case VT_NULL:
        result = 0;
    default:
        assert(0);
    }
    return result;
}
inline bool  Variant::asBoolean() const
{
    bool result = false;
    switch (getType())
    {
    case VT_INTEGER:
        result = asIntegerStrict() > 0;
        break;
    case VT_BOOLEAN:
        result = asBooleanStrict();
        break;
    case VT_DOUBLE:
        result = asDoubleStrict() > 0;
        break;
    case VT_NULL:
        result = 0;
    default:
        assert(0);
    }
    return result;
}
inline double  Variant::asDouble() const
{
    double result = 0.0;
    switch (getType())
    {
    case VT_INTEGER:
        result = asIntegerStrict();
        break;
    case VT_BOOLEAN:
        result = asBooleanStrict()?1:0;
        break;
    case VT_DOUBLE:
        result = asDoubleStrict();
        break;
    case VT_NULL:
        result = 0;
    default:
        assert(0);
    }
    return result;
}
inline std::ostream& operator<<(std::ostream& out, const Variant& variant) 
{
    switch (variant.getType())
    {
    case VT_STRING:
        out<<variant.asString();
        break;
    case VT_INTEGER:
        out<<variant.asIntegerStrict();
        break;
    case VT_BOOLEAN:
        out<<variant.asBooleanStrict();
        break;
    case VT_DOUBLE:
        out<<variant.asDoubleStrict();
        break;
    case VT_DATETIME:
        out<< (variant.asDateTime().print());
        break;
    case VT_NULL:
        out<<"NULL";
        break;
    default:
        assert(0);
    }
    return out;
}
inline std::string prettyPrintVarint(const Variant& variant)
{
    std::stringstream ss;
    ss<<variant;
    return ss.str();
}
}
#endif
