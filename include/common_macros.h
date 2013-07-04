#ifndef MLPLUS_MACROS_H
#define MLPLUS_MACROS_H
#define MLPLUS_BEGIN_NS(s) namespace mlplus{namespace s{
#define MLPLUS_END_NS(s) } }
#define MLPLUS_USING(s) using namespace mlplus::s;
#define MLPLUS_ROOT_NS  using namespace mlplus;
#define MLPLUS_ALIAS_NS(x, y) namespace mlplus { namespace x = y; }
#include <tr1/memory>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <list>
#include <string>
#include <memory>
#include <cassert>
#ifndef NAN
#define NAN builtinnan("")
#endif
