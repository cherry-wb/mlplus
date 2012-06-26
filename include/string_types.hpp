#ifndef MLPLUS_STRING_TYPES_H
#define MLPLUS_STRING_TYPES_H
#include <functional>
#include <string>
namespace mlplus
{
struct IsSpace: public std::unary_function<char, bool>
{
    inline bool operator()(char ch) const
    {
        return std::isspace(ch);
    }
};
struct AnyOf: public std::unary_function<char, bool>
{
    AnyOf(const std::string& s):str(s){}
    inline bool operator()(char ch) const
    {
        return str.find(ch) != std::string::npos;
    }
    const std::string str;
};
inline bool blankString(const std::string& str) 
{
    return str.find_first_not_of(" \t\n\v") == std::string::npos;
}

inline bool isInteger(const std::string& str) 
{
    if (!str.empty())
    {
        unsigned i = 0;
        if ('+' == str[0] || '-' == str[0])
        {
            i = 1;
        }
        for (; i < str.size(); ++i)
        {
            if (str[i] < '0' || str[i] > '9') return false;
        }
        return true;
    }
    return false;
}
inline bool isNumeric(const std::string& str) 
{
    if (!str.empty())
    {
        unsigned i = 0;
        unsigned c = 0;
        if ('+' == str[0] || '-' == str[0])
        {
            i = 1;
        }
        for (; i < str.size(); ++i)
        {
            switch (str[i])
            {
            case '0'...'9':
                break;
            case '.':
                if (++c > 1)
                {
                    return false;
                }
            default:
                return false;
            }
        }
        return true;
    }
    return false;
}
}
#endif
