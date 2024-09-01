#include "MainCommon.h"
#include "MainTypeDef.h"
#include "DbgCmd.h"
#include "Kbhit.h"

RBSTATIC DBGCMD_T f_CommandStatus;
RBSTATIC RB_Vec3f f_RPY[OBJECT_MAXID] = { 0.0f };

RBSTATIC OBJECT_T f_ObjectData[OBJECT_MAXID] = { 0.0f };

typedef struct
{
	RB_Vec3f CenterPos;
	RB_Mat3f CenterRot;
	RB_Vec3f CenterRPY;
}POSEDATA_T;

typedef struct
{
	RB_Vec3f S_Pos;
	RB_Mat3f S_Rot;
	RB_Vec3f S_RPY;
}S_POSEDATA_T;

RBSTATIC S_POSEDATA_T f_SavePose[OBJECT_MAXID] = { 0 };

RBSTATIC RBCONST RB_Vec3f f_RB_Vec3fzero = { { 0.0f, 0.0f, 0.0f} };
RBSTATIC RBCONST RB_Mat3f f_RB_Mat3fident = { { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } } };

RBSTATIC uint8_t f_ThreadCmd;
RBSTATIC uint32_t f_id = 0u;

RBSTATIC void ShowCmdStatus(void);
RBSTATIC void UpdateCmdPos(uint32_t id, uint8_t elem, float step_pos);
RBSTATIC void UpdateCmdRot(uint32_t id, uint8_t elem, float step_deg);
RBSTATIC void UpdateCmdPose(uint32_t id, uint8_t elem, int32_t flag, float step_pos, float step_deg);

RBSTATIC void KeyCmdSwitch(char cmd);
RBSTATIC void RequestLoopOut(void);
RBSTATIC void EventTrigger(void);

RBSTATIC void GenerateBoxAreaObject(RBCONST RB_Vec3f *CPos, float roll, float pitch, float yaw, RBCONST RB_Vec3f *BoxSize, bool AreaType, uint8_t id);
RBSTATIC void GenerateInitSphereObject(RBCONST RB_Vec3f *CPos, float Radius);
RBSTATIC void GenerateInitCapsuleObject(RBCONST RB_Vec3f *CPos, RBCONST RB_Vec3f *Rel, float Radius);
RBSTATIC void GenerateInitRoundRectAngleObject(RBCONST RB_Vec3f *CPos, RBCONST RB_Vec3f *Rel, float Radius, float Height);
RBSTATIC void DbgCmdSetObjectParam(void);

//====================================================

RBSTATIC RB_Vec3f f_DbgVec3f = { 0.0f };
RBSTATIC char f_str[128];

RBSTATIC SEGMENT_T f_SegmentArray[SEGMENT_MAXID] = { 0.0f };

void DbgCmd_SetVec3f(RBCONST char *str, RBCONST RB_Vec3f *v)
{
	uint32_t n = 0u;

	while (str[n] != '\0')
	{
		f_str[n] = str[n];
		n++;
	}

	f_DbgVec3f = *v;
}

RBSTATIC void ShowCmdStatus(void)
{
	printf("-----------------------------------\n");

	RB_Vec3fTermOut(f_str, &f_DbgVec3f);

}

RBSTATIC void UpdateCmdPos(uint32_t id, uint8_t elem, float step_pos)
{
	float NowPos_elem = f_ObjectData[id].CenterPos.e[elem];
	RB_Vec3fSetElem(&(f_ObjectData[id].CenterPos), elem, (NowPos_elem + step_pos));
}

RBSTATIC void UpdateCmdRot(uint32_t id, uint8_t elem, float step_deg)
{
	RB_Mat3f Target_Roll, Target_Pitch,  Target_Yaw;
	RB_Mat3f Target_YawPitch;
	RB_Vec3f axis, Ex, Ey, Ez;

	f_RPY[id].e[elem] += step_deg;

#if 1

	RB_Vec3fCreate(1.0f, 0.0f, 0.0f, &Ex);
	RB_Vec3fCreate(0.0f, 1.0f, 0.0f, &Ey);
	RB_Vec3fCreate(0.0f, 0.0f, 1.0f, &Ez);

	//degをradに変換、3次元行列に変換
	RB_AxisRotateMat3f(&Ex, Deg2Rad(f_RPY[id].e[0u]), &Target_Roll );
	RB_AxisRotateMat3f(&Ey, Deg2Rad(f_RPY[id].e[1u]), &Target_Pitch );
	RB_AxisRotateMat3f(&Ez, Deg2Rad(f_RPY[id].e[2u]), &Target_Yaw );

	RB_MulMatMat3f(&Target_Yaw, &Target_Pitch, &Target_YawPitch);
	RB_MulMatMat3f(&Target_YawPitch, &Target_Roll, &(f_ObjectData[id].CenterRot));
#endif

}

RBSTATIC void UpdateCmdPose(uint32_t id, uint8_t elem, int32_t flag, float step_pos, float step_deg)
{
	if(flag)
	{
		UpdateCmdPos(id, elem, step_pos);
	}
	else
	{
		UpdateCmdRot(id, elem, step_deg);
	}
}

RBSTATIC void KeyCmdSwitch(char cmd)
{
	static int32_t cmd_flag = 1;
	static int32_t area_flag = 1;

	static float pos_step = 10.0f;
	static float deg_step = 5.0f;
	static uint32_t id = 1u;

	switch(cmd)
	{
		case 'c':
			cmd_flag ^= 1;
			break;

		case 'j':
			area_flag ^= 1;
			break;

		case 'h':
			printf("\n[key map]===============================\n");
			printf("c: translation <-or-> rotation\n");
			printf("j: MonObject(1〜) <-or-> AreaObject(11〜)\n");
			printf("@: view CoordinateSys\n");
			printf("( pos ) o: 0.1mm, i: 1.0mm, u: 10mm\n");
			printf("(angle) o: 0.1deg, i: 1.0deg, u: 5.0deg\n");
			printf("p: SetPose, l: LoadPose\n");
			printf("x: Reset\n");
			printf("=======================================\n\n");
			break;

		case 'd':
			UpdateCmdPose(id, 0u, cmd_flag, pos_step, deg_step);

			break;

		case 'a':
			UpdateCmdPose(id, 0u, cmd_flag, -pos_step, -deg_step);

			break;

		case 'w':
			UpdateCmdPose(id, 1u, cmd_flag, pos_step, deg_step);

			break;

		case 's':
			UpdateCmdPose(id, 1u, cmd_flag, -pos_step, -deg_step);

			break;	

		case 'k':
			UpdateCmdPose(id, 2u, cmd_flag, pos_step, deg_step);

			break;

		case 'm':
			UpdateCmdPose(id, 2u, cmd_flag, -pos_step, -deg_step);

			break;

		case '@':
			f_ObjectData[id].TFMode ^= 1;
			break;

		case 'o':
			pos_step = 0.1f;
			deg_step = 0.1f;
			break;

		case 'i':
			pos_step = 1.0f;
			deg_step = 1.0f;
			break;

		case 'u':
			pos_step = 10.0f;
			deg_step = 5.0f;
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if(cmd >= '0' && cmd <= '9')
			{
				id = (uint32_t)( cmd - '0');
				if(!area_flag)
				{
					id += 10u;
				}
			}
		break;
		
		case 'p':
			printf("Set Pose[%u]\n", id);
			f_SavePose[id].S_Pos = f_ObjectData[id].CenterPos;
			f_SavePose[id].S_Rot = f_ObjectData[id].CenterRot;
			f_SavePose[id].S_RPY = f_RPY[id];
			break;

		case 'l':
			printf("Loading Pose[%u]\n", id);
			f_ObjectData[id].CenterPos = f_SavePose[id].S_Pos;
			f_ObjectData[id].CenterRot = f_SavePose[id].S_Rot;
			f_RPY[id] = f_SavePose[id].S_RPY;
			break;

		case 'x':
			f_ObjectData[id].CenterPos = f_RB_Vec3fzero;
			f_RPY[id] = f_RB_Vec3fzero;
			f_ObjectData[id].CenterRot = f_RB_Mat3fident; 
			break;

		default:
			RequestLoopOut();
			break;
	}
	printf("Input > %c\n", cmd);

	printf("pos_step: %.3f, deg_step: %.3f\n", pos_step, deg_step);

	printf("ID: %u\t", id);
	if(cmd_flag)
	{
		printf("Mode: Pos X[+:d, -:a], Y[+:w, -:s], Z[+:k, -:m]\n");
	}
	else
	{
		printf("Mode: Rot Roll[+:d, -:a], Pitch[+:w, -:s], Yaw[+:k, -:m]\n");
	}

	if(area_flag)
	{
		printf("Mode: Object \n");
	}
	else
	{
		printf("Mode: Work Area or Block Area \n");
	}

	if(f_ObjectData[id].TFMode)
	{
		printf("Show CoordinateSys");
	}

	RB_Vec3fTermOut("Cmd CenterPos", &(f_ObjectData[id].CenterPos));
	RB_Vec3fTermOut("Cmd Rot(RPY)", &(f_RPY[id]));
	//RB_Mat3fTermOut("Cmd CenterRot", &(f_ObjectData[id].CenterRot));
}

RBSTATIC void RequestLoopOut(void)
{
	f_CommandStatus.endflag = true;
}

RBSTATIC void EventTrigger(void)
{
	if(KB_hit())
	{
		f_CommandStatus.eventflag = true;
		KeyCmdSwitch(KB_getch());
	}
	else
	{
		f_CommandStatus.eventflag = false;
	}
}

//===================================================================================================//
//移行用 BoxAreaObject
RBSTATIC void GenerateBoxAreaObject(RBCONST RB_Vec3f *CPos, float roll, float pitch, float yaw, RBCONST RB_Vec3f *BoxSize, bool AreaType, uint8_t id)
{
	BOX_T box_obj = { 0 };
	f_ObjectData[id].CenterPos = *CPos;

	f_ObjectData[id].ShapeType = 0u;
	box_obj.AreaType = AreaType;

	box_obj.BoxSize = *BoxSize;
	box_obj.CenterType = 0u;
	f_ObjectData[id].BoxData = box_obj;

	RB_Mat3f Target_Roll, Target_Pitch,  Target_Yaw;
	RB_Mat3f Target_YawPitch;
	RB_Vec3f axis, Ex, Ey, Ez;

	RB_Vec3fCreate(1.0f, 0.0f, 0.0f, &Ex);
	RB_Vec3fCreate(0.0f, 1.0f, 0.0f, &Ey);
	RB_Vec3fCreate(0.0f, 0.0f, 1.0f, &Ez);

	//degをradに変換、3次元行列に変換
	RB_AxisRotateMat3f(&Ex, Deg2Rad(roll), &Target_Roll );
	RB_AxisRotateMat3f(&Ey, Deg2Rad(pitch), &Target_Pitch );
	RB_AxisRotateMat3f(&Ez, Deg2Rad(yaw), &Target_Yaw );

	RB_MulMatMat3f(&Target_Yaw, &Target_Pitch, &Target_YawPitch);
	RB_MulMatMat3f(&Target_YawPitch, &Target_Roll, &(f_ObjectData[id].CenterRot));

}

RBSTATIC void GenerateInitSphereObject(RBCONST RB_Vec3f *CPos, float Radius)
{
	SSV_T sphere_data = { 0 };

	f_ObjectData[f_id].CenterPos = *CPos;
	f_ObjectData[f_id].ShapeType = 1u;

	RB_Vec3fCreate(Radius, 0.0f, 0.0f, &(sphere_data.SSV_Size));

	f_ObjectData[f_id].SSVData = sphere_data;
	f_id++;
}

RBSTATIC void GenerateInitCapsuleObject(RBCONST RB_Vec3f *CPos, RBCONST RB_Vec3f *Rel, float Radius)
{
	SSV_T capsule_data = { 0 };

	f_ObjectData[f_id].CenterPos = *CPos;
	f_ObjectData[f_id].ShapeType = 2u;
	//f_ObjectData[f_id].ShapeType = 3u; 3uならシリンダーになる

	//Relから単位ベクトル, ベクトルの長さを取得する
	RB_Vec3f u_rel;
	RB_Vec3fNormalize(Rel, &u_rel);
	float Rel_Size = RB_Vec3fDot(Rel, &u_rel);

	capsule_data.Unit_Rel = u_rel;
	capsule_data.SSV_Size.e[0u] = Radius;
	capsule_data.SSV_Size.e[1u] = Rel_Size;

	f_ObjectData[f_id].SSVData = capsule_data;
	f_id++;
}

RBSTATIC void GenerateInitRoundRectAngleObject(RBCONST RB_Vec3f *CPos, RBCONST RB_Vec3f *Rel, float Radius, float Height)
{
	SSV_T roundrectangle_data = { 0 };

	f_ObjectData[f_id].CenterPos = *CPos;
	f_ObjectData[f_id].ShapeType = 3u;

	RB_Vec3f u_rel, u_vertical, u_height;

	RB_Vec3fNormalize(Rel, &u_rel);
	RB_CalcVerticalVec3f(Rel, &u_vertical);
	RB_VecRotateVec3f(Deg2Rad(-90.0f), &u_rel, &u_vertical, &u_height);

	roundrectangle_data.SSV_Size.e[0u] = Radius;
	roundrectangle_data.SSV_Size.e[1u] = RB_Vec3fDot(Rel, &u_rel);
	roundrectangle_data.SSV_Size.e[2u] = Height;

	roundrectangle_data.Unit_Rel = u_rel;
	roundrectangle_data.Unit_Height = u_height;

	f_ObjectData[f_id].SSVData = roundrectangle_data;
	f_id++;
}

RBSTATIC void DbgCmdSetObjectParam(void)
{
	POSEDATA_T Pose = { 0 };
	RB_Vec3f CPos;
	RB_Vec3f BoxSize;
	RB_Vec3f Rel;

	//球体を作成
	RB_Vec3fCreate(-500.0f, -200.0f, 400.0f, &CPos);
	GenerateInitSphereObject(&CPos, 100.0f);

	//カプセル作成
	RB_Vec3fCreate(-700.0f, -700.0f, 500.0f, &CPos);
	RB_Vec3fCreate(0.0f, 100.0f, 200.0f, &Rel);
	GenerateInitCapsuleObject(&CPos, &Rel, 100.0f);

	//丸い長方形作成
	RB_Vec3fCreate(100.0f, -100.0f, 100.0f, &CPos);
	RB_Vec3fCreate(0.0f, 800.0f, 0.0f, &Rel);
	GenerateInitRoundRectAngleObject(&CPos, &Rel, 50.0f, 800.0f);

//BoxAreaの初期設定
#if 1
	RB_Vec3fCreate(500.0f, 100.0f, 500.0f, &CPos);
	RB_Vec3fCreate(200.0f, 600.0f, 200.0f, &BoxSize);
	GenerateBoxAreaObject(&CPos, 0.0f, -60.0f, 60.0f, &BoxSize, 0u, 11u);
#else
//BoxAreaの初期設定
	RB_Vec3fCreate(300.0f, 0.0f, 400.0f, &CPos);
	RB_Vec3fCreate(600.0f, 400.0f, 400.0f, &BoxSize);
	GenerateBoxAreaObject(&CPos, 0.0f, 0.0f, 0.0f, &BoxSize, 1u, 12u);	

//BoxAreaの初期設定
	RB_Vec3fCreate(300.0f, 0.0f, 400.0f, &CPos);
	RB_Vec3fCreate(600.0f, 400.0f, 400.0f, &BoxSize);
	GenerateBoxAreaObject(&CPos, 0.0f, 0.0f, 0.0f, &BoxSize, 1u, 11u);
#endif


#if 0
//BoxAreaの初期設定
	RB_Vec3fCreate(300.0f, 0.0f, 400.0f, &CPos);
	RB_Vec3fCreate(600.0f, 400.0f, 300.0f, &BoxSize);
	GenerateBoxAreaObject(&CPos, 0.0f, 0.0f, 0.0f, &BoxSize, 1u, 12u);

//BoxAreaの初期設定
	RB_Vec3fCreate(-100.0f, -200.0f, 500.0f, &CPos);
	RB_Vec3fCreate(300.0f, 100.0f, 500.0f, &BoxSize);
	GenerateBoxAreaObject(&CPos, 0.0f, 45.0f, 0.0f, &BoxSize, 0u, 12u);

//BoxAreaの初期設定
	RB_Vec3fCreate(-600.0f, -200.0f, 500.0f, &CPos);
	RB_Vec3fCreate(200.0f, 200.0f, 200.0f, &BoxSize);
	GenerateBoxAreaObject(&CPos, 45.0f, 0.0f, 0.0f, &BoxSize, 0u, 13u);
#endif

//オブジェクト操作(並進・回転用)
	for(uint32_t id = 0; id < (uint32_t)OBJECT_MAXID; id++)
	{
		f_SavePose[id].S_Pos = f_ObjectData[id].CenterPos;
		f_SavePose[id].S_Rot = f_ObjectData[id].CenterRot;
		f_SavePose[id].S_RPY = f_RPY[id];
	}
}

uint32_t DbgCmd_GetMonObjectNum(void)
{
	uint32_t ret = 0u;

	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	for(uint32_t i = 1u; i < (uint32_t)OBJECT_MAXID; i++)
	{
		uint8_t type = ObjectData[i].ShapeType;
		if((type > 0u) && (type < 5u))
		{
			ret++;
		}
	}

	return ret;
}

uint32_t DbgCmd_GetAreaObjectNum(void)
{
	uint32_t ret = 0u;

	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	for(uint32_t i = 1u; i < (uint32_t)OBJECT_MAXID; i++)
	{
		uint32_t type = ObjectData[i].ShapeType;
		if((type == 0u))
		{
			ret++;
		}
	}

	return ret;
}


void DbgCmd_Init(void)
{
	KB_open();

	for(uint32_t i = 0; i < (uint32_t)OBJECT_MAXID; i++)
	{
		f_ObjectData[i].CenterRot = f_RB_Mat3fident;
		f_ObjectData[i].ShapeType = 32u;
		f_ObjectData[i].Overlap = false;
	}

	f_id = 1u;
}

void DbgCmd_PreStartProc(void)
{
	//DbgCmdTestParam();
	DbgCmdSetObjectParam();
}

void DbgCmd_Cycle(void)
{
	if(f_CommandStatus.eventflag)
	{
		ShowCmdStatus();
	}
	EventTrigger();
}

void DbgCmd_Destroy(void)
{
	KB_close();
}

//==========================================================================
void DbgCmd_SetOverlapStatus(uint32_t id, bool status)
{
	f_ObjectData[id].Overlap = status;
}

void DbgCmd_GetCmdStatus(DBGCMD_T *CmdSts)
{
	memcpy((void*)CmdSts, (void*)&f_CommandStatus, sizeof(DBGCMD_T));
}

void DbgCmd_Info(RBCONST char *str)
{
	printf("<INFO> %s\n", str);
}

void DbgCmd_GetPoseCmd(OBJECT_T *Object)
{
	memcpy((void*)Object, (void*)&f_ObjectData, sizeof(OBJECT_T) * (uint32_t)OBJECT_MAXID);
}

void DbgCmd_SetSegment(uint32_t id, RBCONST uint8_t colorid, RBCONST RB_Vec3f *start, RBCONST RB_Vec3f *end)
{
	RBAssert(id < (uint32_t)OBJECT_MAXID);

	f_SegmentArray[id].ColorId = colorid;

	RB_Vec3fCreate(start->e[0u], start->e[1u], start->e[2u], &(f_SegmentArray[id].StPos));
	RB_Vec3fCreate(end->e[0u], end->e[1u], end->e[2u], &(f_SegmentArray[id].EdPos));
}

void DbgCmd_GetSegment(SEGMENT_T *Segments)
{
	memcpy((void*)Segments, (void*)&f_SegmentArray, sizeof(SEGMENT_T) * (uint32_t)SEGMENT_MAXID);
}