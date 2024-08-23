#ifndef DBGCMD_H
#define DBGCMD_H

#include "MainCommon.h"
#include "MainTypeDef.h"
#include "RB_Math.h"

#define OBJECT_MAXID 10u

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
{
	bool eventflag;
	bool endflag;

}DBGCMD_T;

typedef struct
{
	uint32_t id;
	uint8_t ShapeType;//0:Box, 1:Sphere, 2:Cylinder, 3:Capsule
	bool TFMode;
	RB_Vec3f C_Pos;
	RB_Vec3f C_RPY;
	RB_Mat3f C_Rot;
	RB_Vec3f C_AxisLength;
	uint8_t WidthType; 
}OBJECT_T;
//WidthType
//0:Center, 1:X軸でy方向に幅, 2:Y軸でx方向に幅, 3:Z軸半径(xy)
// 4:Y軸半径(xz), 5:X軸半径(yz)

void DbgCmd_Init(void);
void DbgCmd_PreStartProc(void);
void DbgCmd_Cycle(void);
void DbgCmd_Destroy(void);
void DbgCmd_GetCmdStatus(DBGCMD_T *CmdSts);
void DbgCmd_Info(RBCONST char *str);
void DbgCmd_GetPoseCmd(OBJECT_T *CmdPose);
#ifdef DBG_INFO
#define Dbg_Info(str) DbgCmd_Info(str)
#else
#define Dbg_Info(...) NO_STATEMENT;
#endif

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* DBGCMD_H */
