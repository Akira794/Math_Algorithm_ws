#include "GnuPlot.h"
#include "MainCommon.h"
#include "MainTypeDef.h"
#include "DbgCmd.h"
#include "RB_Math.h"

RBSTATIC FILE *plt_3d;
RBSTATIC int32_t f_plot_width = 1000;
RBSTATIC RBCONST RB_Vec3f f_RB_Vec3fzero = { { 0.0f, 0.0f, 0.0f} };
RBSTATIC RBCONST RB_Mat3f f_RB_Mat3fident = { { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } } };

RBSTATIC void Splot_Test(void)
{

}

RBSTATIC void ColorConfig(void)
{
	fprintf(plt_3d,"set palette defined (\
		0.0 \"#999999\" , 0.2 \"#ffffff\", 0.2 \"#660000\", 0.4 \"#ff3333\",\
		0.4 \"#000000\", 0.6 \"#999999\", 0.6 \"#006600\", 0.8 \"#33cc33\",\
		0.8 \"#333300\", 1.0 \"#ffff99\") \n");
}

RBSTATIC void UnsetConfig(void)
{
	fprintf(plt_3d,"unset table \n");
	fprintf(plt_3d,"unset key \n");
	fprintf(plt_3d,"unset border \n"); 
	fprintf(plt_3d,"unset tics \n");
	fprintf(plt_3d,"unset colorbox \n");
}

RBSTATIC void SetConfig(void)
{
	fprintf(plt_3d,"set term qt enh size 800,800 \n");
	//fprintf(plt_3d,"set pm3d depthorder corners2color c1 nohidden3d \n");
	fprintf(plt_3d,"set pm3d depthorder\n");
	fprintf(plt_3d, "set size ratio -1\n");
	fprintf(plt_3d, "set view equal xyz\n");

	fprintf(plt_3d,"set xrange[-%d:%d] \n",f_plot_width, f_plot_width);
	fprintf(plt_3d,"set yrange[-%d:%d] \n",f_plot_width, f_plot_width);
	fprintf(plt_3d,"set zrange[0:%d] \n", (f_plot_width * 2));

	fprintf(plt_3d,"set ticslevel 0\n");/// z軸をxy平面に接続するコマンド
	fprintf(plt_3d,"set view 60,110 \n");
	fprintf(plt_3d,"set grid\n");

	//参照:https://gnuplot.sourceforge.net/demo/walls.html
	fprintf(plt_3d,"set wall x0 fs transparent solid 0.01 border -1 fc \"bisque\"\n");
	fprintf(plt_3d,"set wall z0 fs transparent solid 0.8 border -1 fc \"slategray\"\n");
	fprintf(plt_3d,"set wall y0 fs transparent solid 0.01 border -1 fc \"bisque\" \n");


	fprintf(plt_3d, "set walls\n");
}

RBSTATIC void CoordinateSys_Config(uint32_t id, RBCONST RB_Vec3f *v, float length, RBCONST RB_Mat3f *m)
{
	uint32_t arrow_num = (3 * (id + 1u)) - 2;

	fprintf(plt_3d,"set colorsequence classic\n");
	fprintf(plt_3d,"set arrow %u from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f front lw 2 lt 2 \n",\
	arrow_num, \
	v->e[0],v->e[1],v->e[2],\
//=====================================
	(v->e[0] + length * RB_Mat3fGetElem(m, 0u, 0u)),\
	(v->e[1] + length * RB_Mat3fGetElem(m, 1u, 0u)),\
	(v->e[2] + length * RB_Mat3fGetElem(m, 2u, 0u))\
	);//x

	arrow_num += 1u;
	fprintf(plt_3d,"set arrow %u from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f front lw 2 lt 3 \n",\
	arrow_num, \
	v->e[0],v->e[1],v->e[2],\
//=====================================
	(v->e[0] + length * RB_Mat3fGetElem(m, 0u, 1u)),\
	(v->e[1] + length * RB_Mat3fGetElem(m, 1u, 1u)),\
	(v->e[2] + length * RB_Mat3fGetElem(m, 2u, 1u))\
	);//y

	arrow_num += 1u;
	fprintf(plt_3d,"set arrow %u from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f front lw 2 lt 1 \n",\
	arrow_num, \
	v->e[0],v->e[1],v->e[2],\
//=====================================
	(v->e[0] + length * RB_Mat3fGetElem(m, 0u, 2u)),\
	(v->e[1] + length * RB_Mat3fGetElem(m, 1u, 2u)),\
	(v->e[2] + length * RB_Mat3fGetElem(m, 2u, 2u))\
	);//z

}

RBSTATIC void Create_Wall_x(float x)
{
	fprintf(plt_3d, "Wx=%.3f \n",x);

	fprintf(plt_3d, "set style fill transparent solid 0.2 \n");
	fprintf(plt_3d, "set obj 1 polygon from Wx,-1000,0 to Wx,1000,0 to Wx,1000,1000 to Wx,-1000,1000 to Wx,-1000,0 \
	depthorder fillcolor \"#33AAAA\" \n");	
}

RBSTATIC void Create_Wall_y(float y)
{
	fprintf(plt_3d, "Wy=%.3f\n",y);

	fprintf(plt_3d, "set style fill transparent solid 0.2 \n");
	fprintf(plt_3d, "set obj 2 polygon from -1000,Wy,0 to 1000,Wy,0 to 1000,Wy,1000 to -1000,Wy,1000 to -1000,Wy,0\
	depthorder fillcolor \"#33AAAA\" \n");	
}


RBSTATIC void Create_Wall_z(float z)
{
	fprintf(plt_3d, "Wz=%.3f\n",z);

	fprintf(plt_3d, "set style fill transparent solid 0.2 \n");
	fprintf(plt_3d, "set obj 3 polygon from 1000,1000,Wz to -1000,1000,Wz to -1000,-1000,Wz to 1000,-1000,Wz to 1000,1000,Wz\
	depthorder fillcolor \"#33AAAA\" \n");	
}

RBSTATIC void Create_Box(void)
{
	//fprintf(plt_3d, "set cbrange [0.9:1] \n");
	//fprintf(plt_3d, "set style line 1 lc rgb \"#b90046\" lt 1 lw 0.5 \n");
	//fprintf(plt_3d, "set pm3d depthorder hidden3d \n");
	//fprintf(plt_3d, "set pm3d implicit \n");
	//fprintf(plt_3d, "unset hidden3d \n");

	fprintf(plt_3d, "array X[23] = [%.3f, %.3f, %.3f, %.3f, %.3f, "", \
									%.3f, %.3f, %.3f, %.3f, %.3f, "", \
									%.3f, %.3f, %.3f, %.3f, %.3f, "", \
									%.3f, %.3f, %.3f, %.3f, %.3f]\n", \
									0.0f, 0.0f, 0.0f, 0.0f, 0.0f, \
									100.0f, 100.0f, 100.0f, 100.0f, 100.0f, \
									0.0f, 100.0f, 100.0f, 0.0f, 0.0f, \
									0.0f, 100.0f, 100.0f, 0.0f, 0.0f);

	fprintf(plt_3d, "array Y[23] = [%.3f, %.3f, %.3f, %.3f, %.3f, "", \
									%.3f, %.3f, %.3f, %.3f, %.3f, "", \
									%.3f, %.3f, %.3f, %.3f, %.3f, "", \
									%.3f, %.3f, %.3f, %.3f, %.3f]\n", \
									0.0f, 0.0f, 100.0f, 100.0f, 0.0f, \
									0.0f, 0.0f, 100.0f, 100.0f, 0.0f, \
									0.0f, 0.0f, 100.0f, 100.0f, 0.0f, \
									0.0f, 0.0f, 100.0f, 100.0f, 0.0f);

	fprintf(plt_3d, "array Z[23] = [%.3f, %.3f, %.3f, %.3f, %.3f, "", \
									%.3f, %.3f, %.3f, %.3f, %.3f, "", \
									%.3f, %.3f, %.3f, %.3f, %.3f, "", \
									%.3f, %.3f, %.3f, %.3f, %.3f]\n", \
									500.0f, 600.0f, 600.0f, 500.0f, 500.0f, \
									500.0f, 600.0f, 600.0f, 500.0f, 500.0f, \
									500.0f, 500.0f, 500.0f, 500.0f, 500.0f, \
									600.0f, 600.0f, 600.0f, 600.0f, 600.0f);
}

RBSTATIC void Splot_Data(void)
{
//	fprintf(plt_3d, "array X[3] = [%.3f, %.3f, %.3f]\n", 100.0f, 300.0f, 500.0f);
//	fprintf(plt_3d, "array Y[3] = [%.3f, %.3f, %.3f]\n", 100.0f, 300.0f, 500.0f);
//	fprintf(plt_3d, "array Z[3] = [%.3f, %.3f, %.3f]\n", 300.0f, 100.0f, 300.0f);

	fprintf(plt_3d, "width = %d\n",f_plot_width);
	fprintf(plt_3d, "z_width = %d\n",(f_plot_width * 2));
	fprintf(plt_3d," splot [-width:width][-width:width][0:z_width] X using (X[$1]):(Y[$1]):(Z[$1]):(1) w l ls 1 \n");
}


RBSTATIC void Plot_WorldCoordinateSys(void)
{
	CoordinateSys_Config(0u, &f_RB_Vec3fzero, 100.0f, &f_RB_Mat3fident);	
}

RBSTATIC void Plot_CoordinateSys(void)
{
	static POSECMD_T CommandPose[OBJECT_MAXID];

	DbgCmd_GetPoseCmd(CommandPose);

	for(uint8_t id = 1u; id < (uint32_t)OBJECT_MAXID; id++)
	{
		CoordinateSys_Config(id, &(CommandPose[id].C_Pos), 300.0f, &(CommandPose[id].C_Rot));
	}
}

void GnuPlot_Init(void)
{
//描画立ち上げ
#ifdef WINDOWS
	plt_3d = _popen("gnuplot", "w"); /*Windows gnuplotパイプ通し */
#elif __GNUC__
	plt_3d = popen("gnuplot", "w"); /*Linux gnuplotパイプ通し */
#else
	NO_STATEMENT;
#endif

}

void GnuPlot_PreStartProc(void)
{
//1回の設定で済むものたち
	ColorConfig();
	UnsetConfig();
	SetConfig();
}

void GnuPlot_Cycle(void)
{
//コマンドで変化するものたち

	Plot_WorldCoordinateSys();
	Plot_CoordinateSys();

	//先頭
	Create_Wall_x(100);
	Create_Wall_y(500);
	Create_Wall_z(500);
	Create_Box();
	Splot_Data();
	Splot_Test();

	//fprintf(plt_3d, "e\n");
	fflush(plt_3d);
}

void GnuPlot_Destroy(void)
{
//後始末
	#ifdef WINDOWS
	_pclose(plt_3d);
	#elif __GNUC__
	pclose(plt_3d);
	#else
	NO_STATEMENT;
	#endif
}
