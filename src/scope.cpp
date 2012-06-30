#include "scope.h"
using namespace mlplus;

Scope::Scope():mParent(NULL)
{
}

const Variant& Scope::find(const std::string& key) const
{
    static const Variant t;
    std::map<std::string, Variant>::const_iterator it = mVariants.find(key);
    if (it != mVariants.end())
    {
        return it->second;
    }
    return t;
}
