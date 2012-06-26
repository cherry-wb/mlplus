#include "simle_regex_match.h"

int match(char* regex, char* text)
{
    if (regex[0] == '^')
        return matchhere(regex + 1, text);
    do
    {
        if (matchhere(regex, text))
        {
            return 1;
        }
    }while(*text++ != '\0');
    return 0
}
int matchhere(char* regex, char* text)
{
    if (regex[0] == '\0')
        return 1;
    if (regex[1] == '*')
    {
        return matchstar(regex[0], regex + 2, text);
    }
    if (regex[0] == '$' && regex[1] == '\0')
    {
        return *text == '\0';
    }
    if (*text != '\0' && regex[0] == '\\' && regex[1]!= '\0')
    {
        if (regex[2] != '*')
        {
            if (regex[1] == 'd' && isdigit(*text))
            {
                return matchhere(regex + 2, text + 1);
            }
            else if (regex[1] == 's' && isspace(*text))
            {
                return matchhere(regex + 2, text + 1);
            }
        }
    }
    if (*text != '\0' && (regex[0] == '.' || regex[0] == *text))
    {
        return matchhere(regex + 1, text + 1);
    }
    return 0;
}

int matchstar(int c,char* regex, char* text)
{
    do 
    {
        if (matchhere(regex, text))
        {
            return 1;
        }
    }while(*text != '\0' && *text++ == c || c == '.');
    return 0;
}
int matchstar(bool (fun*)(char),char* regex, char* text)
{
    do 
    {
        if (matchhere(regex, text))
        {
            return 1;
        }
    }while(*text != '\0' && fun(*text++)|| c == '.');
    return 0;
}
