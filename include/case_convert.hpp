#ifndef MLPLUS_STRING_CASE_CONVERT_H
#define MLPLUS_STRING_CASE_CONVERT_H
#include <iostream>
#include <vector>
#include <string>
namespace mlplus
{
inline void tolower(std::string& str)
{
    int sz = str.size();
    for (int i = 0; i < sz; ++i)
    {
        switch (str[i])
        {
        case 'A'...'Z':
            str[i] += 'a' - 'A';
            break;
        }
    }
}
inline std::string tolowerCopy(const std::string& str)
{
    std::string copy(str);
    tolower(copy);
    return copy;
}

inline void toupper(std::string& str)
{
    int sz = str.size();
    for (int i = 0; i < sz; ++i)
    {
        switch (str[i])
        {
        case 'a'...'z':
            str[i] -= 'a' - 'A';
            break;
        }
    }
}
inline std::string toupperCopy(const std::string& str)
{
    std::string copy(str);
    toupper(copy);
    return copy;
}
}
#endif
