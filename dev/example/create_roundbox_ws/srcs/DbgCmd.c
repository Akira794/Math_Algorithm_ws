#include "MainCommon.h"
#include "MainTypeDef.h"
#include "DbgCmd.h"
#include "Kbhit.h"

RBSTATIC DBGCMD_T f_CommandStatus;
RBSTATIC RB_Vec3f f_RPY[OBJECT_MAXID] = { 0.0f };

//正式版
RBSTATIC OBJECT_T f_ObjectData[OBJECT_MAXID] = { 0.0f };

typedef struct
{
	RB_Vec3f CenterPos;
	RB_Mat3f CenterRot;
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
	float NowPos_elem = f_ObjectData[id].CenterPos.e[elem];
	RB_Vec3fSetElem(&(f_ObjectData[id].CenterPos), elem, (NowPos_elem + step_pos));
}

RBSTATIC void UpdateCmdRot(uint32_t id, uint8_t elem, int32_t step_deg)
{
	float NowRPY_elem = f_RPY[id].e[elem];
	RB_Mat3f Now_Rot = f_ObjectData[id].CenterRot;
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
	RB_MulMatMat3f(&Target_Rot, &Now_Rot, &(f_ObjectData[id].CenterRot));

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
			f_ObjectData[id].TFMode ^= 1;

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

	printf("ID: %u\t", id);
	if(flag)
	{
		printf("Mode: Pos X[+:w, -:s], Y[+:d, -:a], Z[+:k, -:m]\n");
	}
	else
	{
		printf("Mode: Rot Roll[+:w, -:s], Pitch[+:d, -:a], Yaw[+:k, -:m]\n");
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

RBSTATIC void ConfigPose(float x, float y, float z, uint8_t type, float deg, POSEDATA_T *Pose)
{
	RB_Vec3fCreate(x,y,z, &(Pose->CenterPos));
	RB_Mat3f Target_Rot = f_RB_Mat3fident;
	RB_Vec3f axis;

	switch(type)
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
	RB_AxisRotateMat3f(&axis, Deg2Rad(deg), &Target_Rot);
	Pose->CenterRot = Target_Rot;
}

RBSTATIC void ConfigBoxObject(POSEDATA_T *Pose, RB_Vec3f *BoxSize, uint8_t CenterType)
{
	BOX_T box_obj = { 0 };

	f_ObjectData[f_id].CenterPos = Pose->CenterPos;
	f_ObjectData[f_id].CenterRot = Pose->CenterRot;

	box_obj.BoxSize = *BoxSize;
	box_obj.CenterType = CenterType;
	f_ObjectData[f_id].ShapeType = 0u;
	f_ObjectData[f_id].Box = box_obj;
	f_id++;
}

RBSTATIC void ConfigSphereObject(POSEDATA_T *Pose, float Radius)
{
	SSV_T sphere_obj = { 0 };

	f_ObjectData[f_id].CenterPos = Pose->CenterPos;
	f_ObjectData[f_id].CenterRot = Pose->CenterRot;
	f_ObjectData[f_id].ShapeType = 1u;
	sphere_obj.Radius = Radius;

	f_ObjectData[f_id].Sphere = sphere_obj;
	f_id++;
}

RBSTATIC void ConfigCapsuleObject(POSEDATA_T *Pose, float Radius, RB_Vec3f *EndPos)
{
	SSV_T capsule_obj = { 0 };

	f_ObjectData[f_id].CenterPos = Pose->CenterPos;
	f_ObjectData[f_id].CenterRot = Pose->CenterRot;
	f_ObjectData[f_id].ShapeType = 2u;
	capsule_obj.Radius = Radius;

	capsule_obj.EndPos = *EndPos;

	f_ObjectData[f_id].Capsule = capsule_obj;
	f_id++;
}

RBSTATIC void ConfigCylinderObject(POSEDATA_T *Pose, float Radius, RB_Vec3f *EndPos)
{
	SSV_T cylinder_obj = { 0 };

	f_ObjectData[f_id].CenterPos = Pose->CenterPos;
	f_ObjectData[f_id].CenterRot = Pose->CenterRot;
	f_ObjectData[f_id].ShapeType = 3u;
	cylinder_obj.Radius = Radius;

	cylinder_obj.EndPos = *EndPos;

	f_ObjectData[f_id].Cylinder = cylinder_obj;
	f_id++;
}

RBSTATIC void ConfigRoundBoxObject(POSEDATA_T *Pose, float Radius, float Height, RB_Vec3f *EndPos)
{
	SSV_T roundbox_obj = { 0 };

	f_ObjectData[f_id].CenterPos = Pose->CenterPos;
	f_ObjectData[f_id].CenterRot = Pose->CenterRot;
	f_ObjectData[f_id].ShapeType = 4u;


	RB_Vec3f axis[6u];
	RB_Vec3f axis_vertical[6u];

	RB_Vec3fCreate(1.0f, 0.0f, 0.0f, &axis[0u]);
	RB_Vec3fCreate(0.0f, 1.0f, 0.0f, &axis[1u]);
	RB_Vec3fCreate(0.0f, 0.0f, 1.0f, &axis[2u]);

	RB_Vec3fCreate(-1.0f, 0.0f, 0.0f, &axis[3u]);
	RB_Vec3fCreate(0.0f, -1.0f, 0.0f, &axis[4u]);
	RB_Vec3fCreate(0.0f, 0.0f, -1.0f, &axis[5u]);

	for(uint8_t i = 0u; i < 6u; i++)
	{
		RB_CalcVerticalVec3f(&axis[i], &axis_vertical[i]);
	}

	RB_Vec3fTermOut("axis_x_vertical", &axis_vertical[0u]);
	RB_Vec3fTermOut("axis_y_vertical", &axis_vertical[1u]);
	RB_Vec3fTermOut("axis_z_vertical", &axis_vertical[2u]);
	RB_Vec3fTermOut("m_axis_x_vertical", &axis_vertical[3u]);
	RB_Vec3fTermOut("m_axis_y_vertical", &axis_vertical[4u]);
	RB_Vec3fTermOut("m_axis_z_vertical", &axis_vertical[5u]);


	RB_Vec3f uwidth, uaxis, WidthPos, Rot_WidthPos;

	RB_Vec3fNormalize(EndPos, &uaxis);
	RB_CalcVerticalVec3f(EndPos, &uwidth);

	RB_Vec3fCreate(((Height)*(uwidth.e[0])), ((Height)*(uwidth.e[1])), ((Height)*(uwidth.e[2])), &WidthPos);

	roundbox_obj.Radius = Radius;
	roundbox_obj.WidthPos = WidthPos;
	roundbox_obj.EndPos = *EndPos;

	f_ObjectData[f_id].RoundBox = roundbox_obj;
	f_id++;
}

RBSTATIC void DbgCmdSetObjectParam(void)
{
	POSEDATA_T Pose = { 0 };

	RB_Vec3f BoxSize;
	RB_Vec3f Rel;

#if 0
	//シリンダー
	ConfigPose(600.0f, 400.0f, 300.0f, 0u, 0.0f, &Pose);
	RB_Vec3fCreate(400.0f, 00.0f, 0.0f, &Rel);
	ConfigCylinderObject(&Pose, 200.0f, &Rel);

	//カプセル
	ConfigPose(-800.0f, -400.0f, 700.0f, 0u, 0.0f, &Pose);
	RB_Vec3fCreate(-200.0f, 500.0f, 300.0f, &Rel);
	ConfigCapsuleObject(&Pose, 100.0f, &Rel);

#endif
	//ひし形
	ConfigPose(0.00f, 400.0f, 700.0f, 0u, 0.0f, &Pose);
	RB_Vec3fCreate(800.0f, 0.0f, 00.0f, &Rel);
							//Radius, Width
	ConfigRoundBoxObject(&Pose, 100.0f, 400.0f, &Rel);


#if 0
	//カプセル
	ConfigPose(0.0f, -800.0f, 700.0f, 0u, 0.0f, &Pose);
	RB_Vec3fCreate(400.0f, 0.0f, 0.0f, &Rel);
	ConfigCapsuleObject(&Pose, 100.0f, &Rel);

	//カプセル
	ConfigPose(0.0f, -300.0f, 700.0f, 0u, 0.0f, &Pose);
	RB_Vec3fCreate(0.0f, 400.0f, 0.0f, &Rel);
	ConfigCapsuleObject(&Pose, 100.0f, &Rel);

	//カプセル
	ConfigPose(0.0f, 600.0f, 700.0f, 0u, 0.0f, &Pose);
	RB_Vec3fCreate(0.0f, 0.0f, 400.0f, &Rel);
	ConfigCapsuleObject(&Pose, 100.0f, &Rel);

	//カプセル
	ConfigPose(-600.0f, 400.0f, 800.0f, 0u, 0.0f, &Pose);
	RB_Vec3fCreate(-400.0f, 0.0f, -400.0f, &Rel);
	ConfigCylinderObject(&Pose, 200.0f, &Rel);
#endif
#if 0
	//球体
	ConfigPose(0.0f, 0.0f, 500.0f, 0u, 0.0f, &Pose);
	ConfigSphereObject(&Pose, 200.0f);

	//カプセル
	ConfigPose(300.0f, 400.0f, 700.0f, 0u, 0.0f, &Pose);
	RB_Vec3fCreate(0.0f, 0.0f, -500.0f, &Rel);
	ConfigCapsuleObject(&Pose, 200.0f, &Rel);
#endif
#if 0
	ConfigPose(-300.0f, -200.0f, 0.0f, 0u, 0.0f, &Pose);
	RB_Vec3fCreate(300.0f, 100.0f, 200.0f, &BoxSize);
	ConfigBoxObject(&Pose, &BoxSize, 1u);

	ConfigPose(-500.0f, -500.0f, 0.0f, 0u, 0.0f, &Pose);
	RB_Vec3fCreate(50.0f, 200.0f, 300.0f, &BoxSize);
	ConfigBoxObject(&Pose, &BoxSize, 2u);

	ConfigPose(800.0f, -600.0f, 0.0f, 0u, 0.0f, &Pose);
	RB_Vec3fCreate(100.0f, 200.0f, 400.0f, &BoxSize);
	ConfigBoxObject(&Pose, &BoxSize, 3u);

	ConfigPose(-500.0f, 0.0f, 300.0f, 0u, 0.0f, &Pose);
	RB_Vec3fCreate(100.0f, 300.0f, 100.0f, &BoxSize);
	ConfigBoxObject(&Pose, &BoxSize, 4u);

	ConfigPose(0.0f, -900.0f, 1000.0f, 0u, 0.0f, &Pose);
	RB_Vec3fCreate(1000.0f, 100.0f, 1000.0f, &BoxSize);
	ConfigBoxObject(&Pose, &BoxSize, 5u);

	ConfigPose(300.0f, -200.0f, 100.0f, 0u, 0.0f, &Pose);
	RB_Vec3fCreate(100.0f, 100.0f, 100.0f, &BoxSize);
	ConfigBoxObject(&Pose, &BoxSize, 0u);
#endif

	for(uint32_t id = 0; id < (uint32_t)OBJECT_MAXID; id++)
	{
		f_SavePose[id].S_Pos = f_ObjectData[id].CenterPos;
		f_SavePose[id].S_Rot = f_ObjectData[id].CenterRot;
		f_SavePose[id].S_RPY = f_RPY[id];
	}
}

void DbgCmd_Init(void)
{
	KB_open();

	for(uint32_t i = 0; i < (uint32_t)OBJECT_MAXID; i++)
	{
		f_ObjectData[i].CenterRot = f_RB_Mat3fident;
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
