#include "Mainctrl.h"
#include <pthread.h>
#include <stdlib.h>
#include <err.h>
#include <unistd.h>

void Test(void);

void *MainEngine(void *arg)
{
	Mainctrl_Init();
	Mainctrl_PreStartProc();
	Mainctrl_EngineLoop();
	Mainctrl_Destroy();
}

void *OutPut(void *arg)
{
	Mainctrl_OutputLoop();
}

void main(void)
{
#ifdef DEV_MATH
	Test();
#else
	pthread_t main_engine_thread;
	pthread_t output_thread;

	pthread_create(&main_engine_thread, NULL, MainEngine, NULL);	
	pthread_create(&output_thread, NULL, OutPut, NULL);

	pthread_join(main_engine_thread, NULL);	
	pthread_join(output_thread, NULL);
#endif	
}

void Test(void)
{
	#include "RB_Math.h"
	UT_RB_Math();
}
