//http://www.rulequest.com/see5-unix.html#.names
#ifndef MLPLUS_NAMEDS_FILE_READER_H
#define MLPLUS_NAMEDS_FILE_READER_H
#include <stack>
#include <set>
#include <string>
#include <vector>
namespace mlplus
{
class Attribute;
class NamesFileReader
{
public:
    const static uint64_t NUMERIC = 0x01;
    const static uint64_t DATE = 0x02;
    const static uint64_t TIME = 0x04;
    const static uint64_t TIMESTAMP = 0x08;
    const static uint64_t BINARY = 0x0F;
    const static uint64_t NAMEDNOMINAL = 0x10;
    const static uint64_t COMPACTNOMINAL = 0x20;
    const static uint64_t LABEL = 0x40;
    const static uint64_t IGNORE = 0x80;
    const static uint64_t ORDERED = 0xF0;
    const static uint64_t TARGET = 0x0100;
    const static uint64_t IMPLICIT = 0x0200;
    struct AttributeDesc
    {
        AttributeDesc();
        uint64_t type;
        std::string name;
        std::string value;
    };
    static inline void setBit(uint64_t& target, const uint64_t bit);
    static inline void unsetBit(uint64_t& target, const uint64_t bit);
    static inline bool testBit(const uint64_t& target, const uint64_t bit);
    void read(const std::string& filename);
    void read(std::istream& istr);
    void parseLine(const std::string& line);
    Attribute* makeAttribute(int index) const;
    AttributeDesc getAttributeMeta(const std::string&) const;
    AttributeDesc getExplictAttrMeta(int i) const;
    int getIdFromName(const std::string&) const;
    inline const std::vector<AttributeDesc>& getImplicitAttributeMeta() const;
    inline const std::vector<AttributeDesc>& getExplicitAttributeMeta() const;
private:
    bool stuffTarget();
    void setAttributeIgnore();
    bool translateValue();
    std::set<std::string> mTargetNames;
    std::set<std::string> mIncludeAttr;
    std::set<std::string> mExcludeAttr;
    std::vector<AttributeDesc> mImplicitAttributeMeta;
    std::vector<AttributeDesc> mExplicitAttributeMeta;
};

inline const std::vector<NamesFileReader::AttributeDesc>&
    NamesFileReader::getImplicitAttributeMeta() const
{
    return mImplicitAttributeMeta;
}
inline const std::vector<NamesFileReader::AttributeDesc>& NamesFileReader::getExplicitAttributeMeta() const
{
    return mExplicitAttributeMeta;
}
inline void  NamesFileReader::setBit(uint64_t& target, const uint64_t bit)
{
    target |= bit;
}
inline void  NamesFileReader::unsetBit(uint64_t& target, const uint64_t bit)
{
    target &= ~bit;
}
inline bool  NamesFileReader::testBit(const uint64_t& target, const uint64_t bit)
{
    return target & bit;
}
}
#endif

