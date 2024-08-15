#ifndef DBGCMD_H
#define DBGCMD_H

#include "MainCommon.h"
#include "MainTypeDef.h"
#include "RB_Math.h"

#define OBJECT_MAXID 256u
#define SEGMENT_MAXID 256u

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
{
	bool eventflag;
	bool endflag;

}DBGCMD_T;

//WidthType
//0:Center, 1:X軸でy方向に幅, 2:Y軸でx方向に幅, 3:Z軸半径(xy)
// 4:Y軸半径(xz), 5:X軸半径(yz)

typedef struct
{
	uint8_t CenterType;
	RB_Vec3f BoxSize;
}BOX_T;

//Sphere-swept volume (SSV)
//Sphere, Capsule, Cylinder, RoundRectAngle
typedef struct
{
	RB_Vec3f Unit_Rel;
	RB_Vec3f Unit_Width;
	RB_Vec3f SSV_Size; //0u: Radius, 1u: Rel. 2u: Width

}SSV_T;

typedef struct
{
	uint32_t Id;//Max 20
	RB_Vec3f CenterPos;
	RB_Mat3f CenterRot;
	uint8_t ShapeType;//0:Box, 1:Sphere, 2:Capsule, 3:Cylinder, 4:RoundRectAngle
	bool TFMode;
	bool Overlap;
	BOX_T Box;
	SSV_T Sphere;
	SSV_T Capsule;
	SSV_T Cylinder;
	SSV_T RoundRectAngle;
}OBJECT_T;

typedef struct
{
	uint8_t ColorId;
	RB_Vec3f StPos;
	RB_Vec3f EdPos;
}SEGMENT_T;

void DbgCmd_Init(void);
void DbgCmd_PreStartProc(void);
void DbgCmd_Cycle(void);
void DbgCmd_Destroy(void);

void DbgCmd_SetOverlapStatus(uint32_t id, bool status);

void DbgCmd_GetCmdStatus(DBGCMD_T *CmdSts);
void DbgCmd_Info(RBCONST char *str);
void DbgCmd_SetVec3f(RBCONST char *str, RBCONST RB_Vec3f *v);

void DbgCmd_SetSegment(uint32_t id, RBCONST uint8_t colorid, RBCONST RB_Vec3f *start, RBCONST RB_Vec3f *end);
void DbgCmd_GetSegment(SEGMENT_T *Segments);

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
