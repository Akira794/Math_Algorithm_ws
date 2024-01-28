#ifndef RB_COMMON_H
#define RB_COMMON_H

#include "rb_type.h"

#if 0
#define RB_DBG
#endif

#define NO_STATEMENT
#define RBUNUSED(x) ((void)(x))

#ifdef RB_DBG
#define RBCONST
#else
#define RBCONST const
#endif /* RB_DBG*/

#endif /* RB_COMMON_H */