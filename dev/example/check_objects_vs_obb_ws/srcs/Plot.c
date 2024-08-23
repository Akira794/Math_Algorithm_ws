#include "MainCommon.h"
#include "MainTypeDef.h"
#include "DbgCmd.h"
#include "Plot.h"
#include "GnuPlot.h"

void Plot_Init(void)
{
//描画立ち上げ
	Dbg_Info("Plot_Init");
	GnuPlot_Init();
}

void Plot_PreStartProc(void)
{
//1回の設定で済むものたち
	Dbg_Info("Plot_PreStartProc");
	GnuPlot_PreStartProc();
}

void Plot_Cycle(void)
{
//コマンドで変化するものたち
	Dbg_Info("Plot_Cycle");
	GnuPlot_Cycle();
}

void Plot_Destroy(void)
{
	Dbg_Info("Plot_Destroy");
	GnuPlot_Destroy();
//後始末
}