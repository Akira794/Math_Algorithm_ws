#ifndef PLOT_H
#define PLOT_H

#include "MainCommon.h"
#include "MainTypeDef.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void Plot_Init(void);
void Plot_PreStartProc(void);
void Plot_Cycle(void);
void Plot_Destroy(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* PLOT_H */