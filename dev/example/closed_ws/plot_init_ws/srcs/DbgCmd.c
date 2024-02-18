#include "MainCommon.h"
#include "MainTypeDef.h"
#include "DbgCmd.h"
#include "Kbhit.h"

RBSTATIC DBGCMD_T f_CommandStatus;
RBSTATIC POSECMD_T f_CommandPose[OBJECT_MAXID] = { 0 };
RBSTATIC RBCONST RB_Vec3f f_RB_Vec3fzero = { { 0.0f, 0.0f, 0.0f} };
RBSTATIC RBCONST RB_Mat3f f_RB_Mat3fident = { { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } } };

RBSTATIC uint8_t f_ThreadCmd;

RBSTATIC void ShowCmdStatus(void);
RBSTATIC void UpdateCmdPos(uint32_t id, uint8_t elem, int32_t step_pos);
RBSTATIC void UpdateCmdRot(uint32_t id, uint8_t elem, int32_t step_deg);
RBSTATIC void UpdateCmdPose(uint32_t id, uint8_t elem, int32_t flag, int32_t step_pos, int32_t step_deg);
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
	float NowPos_elem = f_CommandPose[id].C_Pos.e[elem];
	RB_Vec3fSetElem(&(f_CommandPose[id].C_Pos), elem, (NowPos_elem + step_pos));
}

RBSTATIC void UpdateCmdRot(uint32_t id, uint8_t elem, int32_t step_deg)
{
	float NowRPY_elem = f_CommandPose[id].C_RPY.e[elem];
	RB_Mat3f Now_Rot = f_CommandPose[id].C_Rot;
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
	f_CommandPose[id].C_RPY.e[elem] = NowRPY_elem + step_deg;
	RB_MulMatMat3f(&Target_Rot, &Now_Rot, &(f_CommandPose[id].C_Rot));

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

RBSTATIC void KeyCmdSwitch(char cmd)
{
	static int32_t flag = 1;
	float pos_step = 10.0f;
	float deg_step = 10.0f;
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

		case 'x':
			f_CommandPose[id].C_Pos = f_RB_Vec3fzero;
			f_CommandPose[id].C_RPY = f_RB_Vec3fzero;
			f_CommandPose[id].C_Rot = f_RB_Mat3fident; 
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
	RB_Vec3fTermOut("Cmd C_Pos", &(f_CommandPose[id].C_Pos));
	RB_Vec3fTermOut("Cmd C_RPY", &(f_CommandPose[id].C_RPY));
	//RB_Mat3fTermOut("Cmd C_Rot", &(f_CommandPose[id].C_Rot));
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
		f_CommandPose[i].C_Rot = f_RB_Mat3fident;
	}

}

void DbgCmd_PreStartProc(void)
{
	NO_STATEMENT;
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

void DbgCmd_GetPoseCmd(POSECMD_T *CmdPose)
{
	memcpy((void*)CmdPose, (void*)&f_CommandPose, sizeof(POSECMD_T) * (uint32_t)OBJECT_MAXID);
}
