#ifndef MAINCOMMON_H
#define MAINCOMMON_H

#include "MainTypeDef.h"
#include <assert.h>
//分岐先で何もしないときに使用
#define NO_STATEMENT
#define RBAbort(...) abort()

#ifndef DEV_UT
#define RBAssert(cond) if(!(cond)){RBAbort();}
#else
#define RBAssert(cond) if(!(cond)){ exit(0);}
#endif

#endif /* MAINCOMMON_H */

