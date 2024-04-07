#include "MainCommon.h"
#include "MainTypeDef.h"
#include "DbgCmd.h"
#include "Kbhit.h"


RBSTATIC DBGCMD_T f_CommandStatus;
RBSTATIC uint8_t f_ThreadCmd;

RBSTATIC void EventTrigger(void);
RBSTATIC void KeyCmdSwitch(char cmd);
RBSTATIC void RequestLoopOut(void);
RBSTATIC void ShowInputCmd(char cmd);
RBSTATIC void ShowCmdStatus(void);

RBSTATIC void ShowCmdStatus(void)
{
	printf("-----------------------------------\n");	
}

RBSTATIC void KeyCmdSwitch(char cmd)
{
	static uint8_t thread_cmd = 0u;
	switch(cmd)
	{
		case 'h':
			break;

		case 'w':
			break;

		case 's':
			break;

		case 'a':
			break;

		case 'd':
			break;	

		case 'x':

		default:
			RequestLoopOut();
			break;
	}
	printf("Input > %c\n", cmd);
}

RBSTATIC void RequestLoopOut(void)
{
	f_CommandStatus.endflag = true;
}

void DbgCmd_Init(void)
{
	KB_open();
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

void DbgCmd_GetCmdStatus(DBGCMD_T *CmdSts)
{
	memcpy((void*)CmdSts, (void*)&f_CommandStatus, sizeof(DBGCMD_T));
}

void DbgCmd_Info(void)
{
	printf("<info> MainSim Exit Loop:\n");
}
