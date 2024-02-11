#ifndef MAINCTRL_H
#define MAINCTRL_H

#include "MainCommon.h"
#include "MainTypeDef.h"

void Mainctrl_Init(void);
void Mainctrl_PreStartProc(void);
void Mainctrl_Loop(void);
void Mainctrl_Destroy(void);
bool Mainctrl_IsLoopOut(void);

#endif /* MAINCTRL_H */
