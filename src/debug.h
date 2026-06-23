#ifndef _DEBUG_H
#define _DEBUG_H 1

#include <cstdio>

#define DEBUG true
#define DEBUG_MSG(s,...) (DEBUG ? fprintf(stdout, s, __VA_ARGS__) : 0)

#endif
