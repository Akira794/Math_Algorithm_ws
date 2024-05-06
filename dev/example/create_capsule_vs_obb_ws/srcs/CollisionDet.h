#ifndef COLLISIONDET_H
#define COLLISIONDET_H

#include "MainCommon.h"
#include "MainTypeDef.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void CollisionDet_Init(void);
void CollisionDet_PreStartProc(void);
void CollisionDet_Cycle(void);
void CollisionDet_Destroy(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* COLLISIONDET_H */