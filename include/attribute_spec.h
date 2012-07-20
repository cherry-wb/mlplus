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
    int mTargetIndicator; 
    Attribute* makeAttribute(const AttributeDesc& desc) const;
    Expression* makeExpression(const AttributeDesc& desc) const;
    Attribute* attributeAt(int index);
    inline const std::vector<Attribute*>& attributesVector() const;
    inline const std::vector<Expression*>& expressionVector() const; 
    int findIndex(const std::string& name) const;
    inline unsigned int implictAttributeCount() const;
    inline unsigned int explictAttributeCount() const;
    inline bool isTarget(Attribute*) const;
private:
    std::vector<Expression*> mImplictExpression;
    std::vector<Attribute*> mAttributes;
};
inline bool AttributeSpec::isTarget(Attribute* at) const
{
    if (at)
    {
        return at->getIndex() == mAttributes[mTargetIndicator]->getIndex();
    }
    return false;
}
inline unsigned int  AttributeSpec::implictAttributeCount() const
{
    return mImplictExpression.size();
}
inline unsigned int  AttributeSpec::explictAttributeCount() const
{
    return mAttributes.size() - implictAttributeCount();
}
inline const std::vector<Attribute*>& AttributeSpec::attributesVector() const
{
    return mAttributes;
}
inline const std::vector<Expression*>&  AttributeSpec::expressionVector() const
{
    return  mImplictExpression;
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
    mTargetIndicator = index;
}
inline int  AttributeSpec::getTarget() const
{
    return mTargetIndicator;
}

inline int AttributeSpec::classNo(char* className) const
{ 
    return mAttributes[mTargetIndicator]->indexOfValue(className);
}
}
#endif

