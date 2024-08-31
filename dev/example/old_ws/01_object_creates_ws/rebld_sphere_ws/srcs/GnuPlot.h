#ifndef GNUPLOT_H
#define GNUPLOT_H

#include "MainCommon.h"
#include "MainTypeDef.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void GnuPlot_Init(void);
void GnuPlot_PreStartProc(void);
void GnuPlot_Cycle(void);
void GnuPlot_Destroy(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* GNUPLOT_H */
