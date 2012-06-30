#ifndef MLPLUS_SCOPE_H
#define MLPLUS_SCOPE_H
#include <inttypes.h>
#include <map>
#include "variant.h"
namespace mlplus
{
class Scope
{ 
public:
    Scope();
    template<typename T>
    inline void add(const std::string& key, const T& value);
    const Variant& find(const std::string& key) const;
private:
    std::map<std::string, Variant> mVariants;
    Scope* mParent;
};
template<typename T> 
inline void Scope::add(const std::string& key, const T& value)
{
    Variant t(value);
    mVariants[key] = t;
}
}
#endif
