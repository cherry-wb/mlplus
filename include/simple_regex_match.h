#ifndef MLPLUS_SIMPLE_REGEX_MATCH_H
#define MLPLUS_SIMPLE_REGEX_MATCH_H
int match(char* regex, char* text);
int matchhere(char* regex, char* text);
int matchstar(int c,char* regex, char* text);
int matchstar(bool (fun*)(char),char* regex, char* text);
#endif
