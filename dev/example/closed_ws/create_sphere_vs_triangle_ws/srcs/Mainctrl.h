#ifndef MAINCTRL_H
#define MAINCTRL_H

#include "MainCommon.h"
#include "MainTypeDef.h"

void Mainctrl_Init(void);
void Mainctrl_PreStartProc(void);
void Mainctrl_EngineLoop(void);
void Mainctrl_OutputLoop(void);
void Mainctrl_Destroy(void);

#endif /* MAINCTRL_H */
