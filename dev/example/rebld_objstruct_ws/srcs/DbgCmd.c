#include "MainCommon.h"
#include "MainTypeDef.h"
#include "DbgCmd.h"
#include "Kbhit.h"

RBSTATIC DBGCMD_T f_CommandStatus;
RBSTATIC OBJECT_T f_Data[OBJECT_MAXID] = { 0 };
RBSTATIC RB_Vec3f f_RPY[OBJECT_MAXID] = { 0.0f };

//=======@=========
typedef struct
{
	RB_Vec3f S_Pos;
	RB_Mat3f S_Rot;
	RB_Vec3f S_RPY;
}S_OBJECT_T;

RBSTATIC S_OBJECT_T f_SaveData[OBJECT_MAXID] = { 0 };

RBSTATIC RBCONST RB_Vec3f f_RB_Vec3fzero = { { 0.0f, 0.0f, 0.0f} };
RBSTATIC RBCONST RB_Mat3f f_RB_Mat3fident = { { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } } };

RBSTATIC uint8_t f_ThreadCmd;

RBSTATIC void ShowCmdStatus(void);
RBSTATIC void UpdateCmdPos(uint32_t id, uint8_t elem, int32_t step_pos);
RBSTATIC void UpdateCmdRot(uint32_t id, uint8_t elem, int32_t step_deg);
RBSTATIC void UpdateCmdPose(uint32_t id, uint8_t elem, int32_t flag, int32_t step_pos, int32_t step_deg);
RBSTATIC void GenerateObject(uint32_t id, uint32_t ShapeType, uint32_t WidthType, RB_Vec3f CenterPos, RB_Vec3f ObjectSize);




RBSTATIC void KeyCmdSwitch(char cmd);
RBSTATIC void RequestLoopOut(void);
RBSTATIC void EventTrigger(void);

//====================================================

RBSTATIC void ShowCmdStatus(void)
{
	printf("-----------------------------------\n");	
}

RBSTATIC void UpdateCmdPos(uint32_t id, uint8_t elem, int32_t step_pos)
{
	float NowPos_elem = f_Data[id].C_Pos.e[elem];
	RB_Vec3fSetElem(&(f_Data[id].C_Pos), elem, (NowPos_elem + step_pos));
}

RBSTATIC void UpdateCmdRot(uint32_t id, uint8_t elem, int32_t step_deg)
{
	float NowRPY_elem = f_RPY[id].e[elem];
	RB_Mat3f Now_Rot = f_Data[id].C_Rot;
	RB_Mat3f Target_Rot;
	RB_Vec3f axis;
	
	switch(elem)
	{
		case 0:
			RB_Vec3fCreate(1.0f, 0.0f, 0.0f, &axis);
			break;

		case 1:
			RB_Vec3fCreate(0.0f, 1.0f, 0.0f, &axis);
			break;

		case 2:
			RB_Vec3fCreate(0.0f, 0.0f, 1.0f, &axis);
			break;

		default:
			RB_Vec3fCreate(0.0f, 0.0f, 0.0f, &axis);
			break;
	}
	RB_AxisRotateMat3f(&axis, Deg2Rad(step_deg), &Target_Rot);
	//姿勢情報の更新
	f_RPY[id].e[elem] = NowRPY_elem + step_deg;
	RB_MulMatMat3f(&Target_Rot, &Now_Rot, &(f_Data[id].C_Rot));

}

RBSTATIC void UpdateCmdPose(uint32_t id, uint8_t elem, int32_t flag, int32_t step_pos, int32_t step_deg)
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

RBSTATIC void GenerateObject(uint32_t id, uint32_t ShapeType, uint32_t WidthType, RB_Vec3f CenterPos, RB_Vec3f ObjectSize)
{
	//暫定Box設定
	f_Data[id].ShapeType = ShapeType;
	f_Data[id].C_Pos = CenterPos;
	f_Data[id].C_AxisLength = ObjectSize;
	f_Data[id].WidthType = WidthType;
}

RBSTATIC void KeyCmdSwitch(char cmd)
{
	static int32_t flag = 1;
	float pos_step = 10.0f;
	float deg_step = 5.0f;
	static uint32_t id = 1u;

	switch(cmd)
	{
		case 'c':
			flag ^= 1;
			break;

		case 'h':
			break;

		case 'w':
			UpdateCmdPose(id, 0u, flag, pos_step, deg_step);

			break;

		case 's':
			UpdateCmdPose(id, 0u, flag, -pos_step, -deg_step);

			break;

		case 'd':
			UpdateCmdPose(id, 1u, flag, pos_step, deg_step);

			break;

		case 'a':
			UpdateCmdPose(id, 1u, flag, -pos_step, -deg_step);

			break;	

		case 'k':
			UpdateCmdPose(id, 2u, flag, pos_step, deg_step);

			break;

		case 'm':
			UpdateCmdPose(id, 2u, flag, -pos_step, -deg_step);

			break;

		case '@':
			f_Data[id].TFMode ^= 1;

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
			}
		break;
		
		case 'p':
			printf("Set Pose[%u]\n", id);
			f_SaveData[id].S_Pos = f_Data[id].C_Pos;
			f_SaveData[id].S_Rot = f_Data[id].C_Rot;
			f_SaveData[id].S_RPY = f_RPY[id];
			break;

		case 'l':
			printf("Loading Pose[%u]\n", id);
			f_Data[id].C_Pos = f_SaveData[id].S_Pos;
			f_Data[id].C_Rot = f_SaveData[id].S_Rot;
			f_RPY[id] = f_SaveData[id].S_RPY;
			break;

		case 'x':
			f_Data[id].C_Pos = f_RB_Vec3fzero;
			f_RPY[id] = f_RB_Vec3fzero;
			f_Data[id].C_Rot = f_RB_Mat3fident; 
			break;
		default:
			RequestLoopOut();
			break;
	}
	printf("Input > %c\n", cmd);

	printf("ID: %u\t", id);
	if(flag)
	{
		printf("Mode: Cmd C_Pos\n");
	}
	else
	{
		printf("Mode: Cmd C_Rot\n");
	}

	if(f_Data[id].TFMode)
	{
		printf("Show CoordinateSys");
	}

	RB_Vec3fTermOut("Cmd C_Pos", &(f_Data[id].C_Pos));
	RB_Vec3fTermOut("Cmd Rot(RPY)", &(f_RPY[id]));
	//RB_Mat3fTermOut("Cmd C_Rot", &(f_Data[id].C_Rot));
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

void DbgCmd_Init(void)
{
	KB_open();

	for(uint8_t i = 0; i < (uint32_t)OBJECT_MAXID; i++)
	{
		f_Data[i].C_Rot = f_RB_Mat3fident;
	}

}

void DbgCmd_PreStartProc(void)
{
	//暫定 Box設定
	typedef struct
	{
		uint8_t ShapeType;
		RB_Vec3f ObjectPos;
		RB_Vec3f ObjectSize;
		uint8_t WidthType;
	}BoxInit_T;

	BoxInit_T ObjectBox[OBJECT_MAXID] = { 0.0f };
#if 1
	RB_Vec3fCreate(300.0f, -200.0f, 100.0f, &(ObjectBox[1u].ObjectPos));
	RB_Vec3fCreate(100.0f, -100.0f, 100.0f, &(ObjectBox[1u].ObjectSize));

	RB_Vec3fCreate(-300.0f, -200.0f, 0.0f, &(ObjectBox[2u].ObjectPos));
	RB_Vec3fCreate(300.0f, 100.0f, 200.0f, &(ObjectBox[2u].ObjectSize));
	ObjectBox[2u].WidthType = 1u;

	RB_Vec3fCreate(-500.0f, -500.0f, 0.0f, &(ObjectBox[3u].ObjectPos));
	RB_Vec3fCreate(50.0f, 200.0f, 300.0f, &(ObjectBox[3u].ObjectSize));
	ObjectBox[3u].WidthType = 2u;

	RB_Vec3fCreate(800.0f, -600.0f, 0.0f, &(ObjectBox[4u].ObjectPos));
	RB_Vec3fCreate(100.0f, 200.0f, 400.0f, &(ObjectBox[4u].ObjectSize));
	ObjectBox[4u].WidthType = 3u;

	RB_Vec3fCreate(0.0f, -900.0f, 1000.0f, &(ObjectBox[5u].ObjectPos));
	RB_Vec3fCreate(1000.0f, 100.0f, 1000.0f, &(ObjectBox[5u].ObjectSize));
	ObjectBox[5u].WidthType = 5u;
#endif
	//暫定・カプセル
	RB_Vec3fCreate(-300.0f, -300.0f, 500.0f, &(ObjectBox[6u].ObjectPos));
	RB_Vec3fCreate(100.0f, 100.0f, 500.0f, &(ObjectBox[6u].ObjectSize));
	ObjectBox[6u].ShapeType = 3u;
	ObjectBox[6u].WidthType = 3u;

	RB_Vec3fCreate(300.0f, -500.0f, 500.0f, &(ObjectBox[7u].ObjectPos));
	RB_Vec3fCreate(100.0f, 500.0f, 100.0f, &(ObjectBox[7u].ObjectSize));
	ObjectBox[7u].ShapeType = 3u;
	ObjectBox[7u].WidthType = 4u;

	RB_Vec3fCreate(300.0f, 500.0f, 500.0f, &(ObjectBox[8u].ObjectPos));
	RB_Vec3fCreate(500.0f, 100.0f, 100.0f, &(ObjectBox[8u].ObjectSize));
	ObjectBox[8u].ShapeType = 3u;
	ObjectBox[8u].WidthType = 5u;
	//暫定 球体
	RB_Vec3fCreate(-300.0f, 400.0f, 250.0f, &(ObjectBox[9u].ObjectPos));
	RB_Vec3fCreate(250.0f, 250.0f, 250.0f, &(ObjectBox[9u].ObjectSize));
	ObjectBox[9u].ShapeType = 1u;

	for(uint32_t id = 1u; id < (uint32_t)OBJECT_MAXID; id++)
	{
		GenerateObject(id, ObjectBox[id].ShapeType, ObjectBox[id].WidthType, ObjectBox[id].ObjectPos, ObjectBox[id].ObjectSize);
		f_SaveData[id].S_Pos = f_Data[id].C_Pos;
		f_SaveData[id].S_Rot = f_Data[id].C_Rot;
		f_SaveData[id].S_RPY = f_RPY[id];
	}
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

void DbgCmd_GetCmdStatus(DBGCMD_T *CmdSts)
{
	memcpy((void*)CmdSts, (void*)&f_CommandStatus, sizeof(DBGCMD_T));
}

void DbgCmd_Info(RBCONST char *str)
{
	printf("<INFO> %s\n", str);
}

void DbgCmd_GetPoseCmd(OBJECT_T *CmdPose)
{
	memcpy((void*)CmdPose, (void*)&f_Data, sizeof(OBJECT_T) * (uint32_t)OBJECT_MAXID);
}
