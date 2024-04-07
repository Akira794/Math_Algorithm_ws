#include "Mainctrl.h"

#include "MainCommon.h"
#include "MainTypeDef.h"
#include "DbgCmd.h"
#include "Plot.h"
#include <unistd.h>

typedef struct{
	bool endflag;
	float engine_dt;
	float output_dt;
}MainCtrl_T;

RBSTATIC MainCtrl_T f_mctrl;
RBSTATIC bool f_IsStartUp = false;

RBSTATIC bool SimLoop(bool pause);

//============================================================================
RBSTATIC bool SimLoop(bool pause)
{
	bool ret = false;
	if(pause)
	{
		DbgCmd_Cycle();
		ret = true;
	}
	//=======draw===============
	return ret;
}

RBSTATIC bool IsLoopOut(void)
{
	bool ret;

	DBGCMD_T CmdStatus;
	DbgCmd_GetCmdStatus(&CmdStatus);

	if(CmdStatus.endflag)
	{
		ret = false;
	}
	else
	{
		ret = true;
	}

	return ret;
}

//================================================
void Mainctrl_Init(void)
{
	DbgCmd_Init();
	f_mctrl.endflag = false;
	f_mctrl.engine_dt = 0.01f;
	f_mctrl.output_dt = 0.5f;
}

void Mainctrl_PreStartProc(void)
{
	DbgCmd_PreStartProc();
	Plot_Init();
	Plot_PreStartProc();
}

void Mainctrl_EngineLoop(void)
{
	while(IsLoopOut())
	{
		f_IsStartUp = SimLoop(true);
		sleep(f_mctrl.engine_dt);
	}
	Dbg_Info("Exit MainEngine Loop");
}

void Mainctrl_OutputLoop(void)
{
	static bool idle_flag = true;
	if(idle_flag)
	{
		while(!f_IsStartUp)
		{
			Dbg_Info("Output Ready ....");
		}
		idle_flag = false;
	}

	Dbg_Info("Output Start!");
	while(IsLoopOut())
	{
		Plot_Cycle();
		sleep(f_mctrl.output_dt);
	}
	Dbg_Info("Exit Output Loop");
}

void Mainctrl_Destroy(void)
{
	Dbg_Info("Exit MainEngine");	
	Plot_Destroy();
	DbgCmd_Destroy();
}
