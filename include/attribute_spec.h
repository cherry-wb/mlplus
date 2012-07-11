#ifndef MLPLUS_ATTRIBUTE_SPEC_H
#define MLPLUS_ATTRIBUTE_SPEC_H
#include <vector>
#include "names_file_reader.h"
#include "attribute.h"
namespace mlplus
{
class Attribute;
class Expression;
class AttributeSpec
{
public:
    AttributeSpec();
    virtual ~AttributeSpec();
    AttributeSpec(NamesFileReader& reader);
    inline void setTarget(int index);
    inline int  getTarget() const;
    inline int classNo(char* className) const;
    int numTarget() const;
    int numAttributes() const;
    int mTargetIndex; 
    Attribute* makeAttribute(const AttributeDesc& desc) const;
    Expression* makeExpression(const AttributeDesc& desc) const;
    Attribute* attributeAt(int index);
    const std::vector<Attribute*>& attributesVector() const;
    int findIndex(const std::string& name) const;
private:
    std::vector<Expression*> mImplictExpression;
    std::vector<Attribute*> mAttributes;
};
inline const std::vector<Attribute*>& AttributeSpec::attributesVector() const
{
    return mAttributes;
}
inline  int  AttributeSpec::numAttributes() const
{
    return mAttributes.size();
}
inline Attribute* AttributeSpec::attributeAt(int index)
{
    return mAttributes.at(index);
}
inline void AttributeSpec::setTarget(int index)
{
    mTargetIndex = index;
}
inline int  AttributeSpec::getTarget() const
{
    return mTargetIndex;
}

inline int AttributeSpec::classNo(char* className) const
{ 
    return mAttributes[mTargetIndex]->indexOfValue(className);
}
}
#endif

