#ifndef DBGCMD_H
#define DBGCMD_H

#include "MainCommon.h"
#include "MainTypeDef.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
{
	bool eventflag;
	bool endflag;

}DBGCMD_T;

void DbgCmd_Init(void);
void DbgCmd_PreStartProc(void);
void DbgCmd_Cycle(void);
void DbgCmd_Destroy(void);
void DbgCmd_Info(void);
void DbgCmd_GetCmdStatus(DBGCMD_T *CmdSts);
#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* DBGCMD_H */
