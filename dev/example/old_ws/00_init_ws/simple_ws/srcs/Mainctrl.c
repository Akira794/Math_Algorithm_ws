#include "Mainctrl.h"

#include "MainCommon.h"
#include "MainTypeDef.h"
#include "DbgCmd.h"
#include <unistd.h>

typedef struct{
	bool endflag;
	//
	//
	//
}MainCtrl_T;

RBSTATIC MainCtrl_T f_mctrl;


RBSTATIC void SimLoop(bool pause);

//============================================================================
RBSTATIC void SimLoop(bool pause)
{
	if(pause)
	{
		DbgCmd_Cycle();
	}

	//=======draw===============

}

bool Mainctrl_IsLoopOut(void)
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

RBSTATIC void Struct_Init(void)
{
	f_mctrl.endflag = false;
}

//================================================
void Mainctrl_Init(void)
{
	DbgCmd_Init();
}

void Mainctrl_PreStartProc(void)
{
	DbgCmd_PreStartProc();
}

void Mainctrl_Loop(void)
{
	while(Mainctrl_IsLoopOut())
	{
		SimLoop(true);
		sleep(0.01);
	}
}

void Mainctrl_Destroy(void)
{
	DbgCmd_Destroy();
	DbgCmd_Info();
}
