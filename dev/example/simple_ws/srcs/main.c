#include "Mainctrl.h"
#include <pthread.h>
#include <stdlib.h>
#include <err.h>
#include <unistd.h>

void Test(void);

void main(void)
{
	Mainctrl_Init();
	Mainctrl_PreStartProc();
	Mainctrl_Loop();
	Mainctrl_Destroy();
}

void Test(void)
{
	NO_STATEMENT;
}
