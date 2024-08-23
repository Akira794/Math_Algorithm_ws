#include "GnuPlot.h"
#include "MainCommon.h"
#include "MainTypeDef.h"
#include "DbgCmd.h"
#include "RB_Math.h"

RBSTATIC FILE *plt_3d;
RBSTATIC int32_t f_plot_width = 1000;
RBSTATIC RBCONST RB_Vec3f f_RB_Vec3fzero = { { 0.0f, 0.0f, 0.0f} };
RBSTATIC RBCONST RB_Mat3f f_RB_Mat3fident = { { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } } };

RBSTATIC void DrawGround(float z);
RBSTATIC void GenerateSphereDat(void);

RBSTATIC void GenerateCylinder_AxisXDat(void);
RBSTATIC void GenerateCylinder_AxisYDat(void);
RBSTATIC void GenerateCylinder_AxisZDat(void);
RBSTATIC void GenerateCylinderDat(void);

RBSTATIC void DrawBox(uint32_t id, OBJECT_T *Object);
RBSTATIC void DrawObjectSize(uint32_t id, OBJECT_T *Object);

//RBSTATIC void DrawBox(uint32_t id, DATA_T *Object);
//RBSTATIC void DrawObjectSize(uint32_t id, DATA_T *Object);
RBSTATIC void DrawObject(void);

RBSTATIC void UnsetConfig(void);
RBSTATIC void SetConfig(void);
RBSTATIC void SplotData(void);

RBSTATIC void CoordinateSys_Config(uint32_t id, RBCONST RB_Vec3f *v, float length, RBCONST RB_Mat3f *m);
RBSTATIC void DrawWorldCoordinateSys(void);
RBSTATIC void DrawCoordinateSys(void);

RBSTATIC void DrawGround(float z)
{
	//描画する
	float objectsolid_val = 0.5;
	RB_Vec3f SurfaceArray[4u] = { 0.0f };

	RB_Vec3fCreate( -f_plot_width, -f_plot_width, z, &(SurfaceArray[0u]));//A0
	RB_Vec3fCreate( -f_plot_width,  f_plot_width, z, &(SurfaceArray[1u]));//B1
	RB_Vec3fCreate(  f_plot_width,  f_plot_width, z, &(SurfaceArray[2u]));//C2
	RB_Vec3fCreate(  f_plot_width, -f_plot_width, z, &(SurfaceArray[3u]));//D3

	fprintf(plt_3d, "set style fill transparent solid %f \n", objectsolid_val);
	fprintf(plt_3d, "set obj 1 polygon from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
		depthorder fillcolor \"bisque\" \n", \
		RB_Vec3fGetElem(&SurfaceArray[0u], 0u),RB_Vec3fGetElem(&SurfaceArray[0u], 1u),RB_Vec3fGetElem(&SurfaceArray[0u], 2u), \
		RB_Vec3fGetElem(&SurfaceArray[1u], 0u),RB_Vec3fGetElem(&SurfaceArray[1u], 1u),RB_Vec3fGetElem(&SurfaceArray[1u], 2u), \
		RB_Vec3fGetElem(&SurfaceArray[2u], 0u),RB_Vec3fGetElem(&SurfaceArray[2u], 1u),RB_Vec3fGetElem(&SurfaceArray[2u], 2u), \
		RB_Vec3fGetElem(&SurfaceArray[3u], 0u),RB_Vec3fGetElem(&SurfaceArray[3u], 1u),RB_Vec3fGetElem(&SurfaceArray[3u], 2u), \
		RB_Vec3fGetElem(&SurfaceArray[0u], 0u),RB_Vec3fGetElem(&SurfaceArray[0u], 1u),RB_Vec3fGetElem(&SurfaceArray[0u], 2u)  \
	);
}

RBSTATIC void GenerateSphereDat(void)
{
	fprintf(plt_3d, "F_sphere_x(u,v)=sin(u)*cos(v)\n");
	fprintf(plt_3d, "F_sphere_y(u,v)=sin(u)*sin(v)\n");
	fprintf(plt_3d, "F_sphere_z(u,v)=cos(u)\n");

	fprintf(plt_3d, "set xrange [0:pi] \n");
	fprintf(plt_3d, "set yrange [-pi:pi] \n");

	fprintf(plt_3d, "set samples 12 \n");
	fprintf(plt_3d, "set isosamples 24 \n");

	fprintf(plt_3d, "sphere=\"sphere.dat\" \n");
	fprintf(plt_3d, "set table sphere \n");
	fprintf(plt_3d, "splot 0 \n");
	fprintf(plt_3d, "unset xrange \n");
	fprintf(plt_3d, "unset yrange \n");
}

RBSTATIC void GenerateCylinder_AxisXDat(void)
{
	fprintf(plt_3d, "F_cylinderX_x(v)=v\n");
	fprintf(plt_3d, "F_cylinderX_y(u)=cos(u)\n");
	fprintf(plt_3d, "F_cylinderX_z(u)=sin(u)\n");

	fprintf(plt_3d, "set xrange [-pi:pi] \n");
	fprintf(plt_3d, "set yrange [0:1] \n");

	fprintf(plt_3d, "set samples 24 \n");
	fprintf(plt_3d, "set isosamples 2 \n");

	fprintf(plt_3d, "cylinderX=\"cylinderX.dat\" \n");
	fprintf(plt_3d, "set table cylinderX \n");
	fprintf(plt_3d, "splot 0 \n");
	fprintf(plt_3d, "unset xrange \n");
	fprintf(plt_3d, "unset yrange \n");
}

RBSTATIC void GenerateCylinder_AxisYDat(void)
{
	fprintf(plt_3d, "F_cylinderY_x(u)=cos(u)\n");
	fprintf(plt_3d, "F_cylinderY_y(v)=v\n");
	fprintf(plt_3d, "F_cylinderY_z(u)=-sin(u)\n");

	fprintf(plt_3d, "set xrange [-pi:pi] \n");
	fprintf(plt_3d, "set yrange [0:1] \n");

	fprintf(plt_3d, "set samples 24 \n");
	fprintf(plt_3d, "set isosamples 2 \n");

	fprintf(plt_3d, "cylinderY=\"cylinderY.dat\" \n");
	fprintf(plt_3d, "set table cylinderY \n");
	fprintf(plt_3d, "splot 0 \n");
	fprintf(plt_3d, "unset xrange \n");
	fprintf(plt_3d, "unset yrange \n");
}

RBSTATIC void GenerateCylinder_AxisZDat(void)
{
	fprintf(plt_3d, "F_cylinderZ_x(u)=cos(u)\n");
	fprintf(plt_3d, "F_cylinderZ_y(u)=sin(u)\n");
	fprintf(plt_3d, "F_cylinderZ_z(v)=v\n");

	fprintf(plt_3d, "set xrange [-pi:pi] \n");
	fprintf(plt_3d, "set yrange [0:1] \n");

	fprintf(plt_3d, "set samples 24 \n");
	fprintf(plt_3d, "set isosamples 2 \n");

	fprintf(plt_3d, "cylinderZ=\"cylinderZ.dat\" \n");
	fprintf(plt_3d, "set table cylinderZ \n");
	fprintf(plt_3d, "splot 0 \n");
	fprintf(plt_3d, "unset xrange \n");
	fprintf(plt_3d, "unset yrange \n");
}


RBSTATIC void GenerateCylinderDat(void)
{
	GenerateCylinder_AxisXDat();
	GenerateCylinder_AxisYDat();
	GenerateCylinder_AxisZDat();
}

RBSTATIC void DrawBox(uint32_t id, OBJECT_T *Object)
{
	BOX_T *Box_obj = &Object->Box;
	uint32_t object_num = (6 * (id + 1u)) - 5;
	RB_Vec3f BoxInitArray[8u] = { 0.0f };
	RB_Vec3f BoxVertex[8u] = { 0.0f };
	RB_Vec3f width3f;

	float lx = RB_Vec3fGetElem(&(Box_obj->BoxSize), 0u);
	float ly = RB_Vec3fGetElem(&(Box_obj->BoxSize), 1u);
	float lz = RB_Vec3fGetElem(&(Box_obj->BoxSize), 2u);

switch(Box_obj->CenterType)
{
	case 1:
		RB_Vec3fCreate( 0.0f, 1.0f, 0.0f, &width3f);//X軸でy方向に幅
		break;

	case 2:
		RB_Vec3fCreate( 1.0f, 0.0f, 0.0f, &width3f);//Y軸でx方向に幅
		break;

	case 3:
		RB_Vec3fCreate( 1.0f, 1.0f, 0.0f, &width3f);//Z軸でxy方向に幅(半径)
		break;

	case 4:
		RB_Vec3fCreate( 1.0f, 0.0f, 1.0f, &width3f);//Y軸でxz方向に幅(半径)
		break;

	case 5:
		RB_Vec3fCreate( 0.0f, 1.0f, 1.0f, &width3f);//X軸でyz方向に幅(半径)
		break;

	default:
		RB_Vec3fCreate( 1.0f, 1.0f, 1.0f, &width3f);//重心からxyz方向に幅(半径、サイズ)
		break;
}

float wh_x = RB_Vec3fGetElem(&width3f, 0u) * (-lx);
float wh_y = RB_Vec3fGetElem(&width3f, 1u) * (-ly);
float wh_z = RB_Vec3fGetElem(&width3f, 2u) * (-lz);

	RB_Vec3fCreate( wh_x, wh_y, wh_z, &(BoxInitArray[0u]));//A0
	RB_Vec3fCreate( wh_x,   ly, wh_z, &(BoxInitArray[1u]));//B1
	RB_Vec3fCreate(   lx,   ly, wh_z, &(BoxInitArray[2u]));//C2
	RB_Vec3fCreate(   lx, wh_y, wh_z, &(BoxInitArray[3u]));//D3
	RB_Vec3fCreate( wh_x, wh_y,   lz, &(BoxInitArray[4u]));//E4
	RB_Vec3fCreate( wh_x,   ly,   lz, &(BoxInitArray[5u]));//F5
	RB_Vec3fCreate(   lx,   ly,   lz, &(BoxInitArray[6u]));//G6
	RB_Vec3fCreate(   lx, wh_y,   lz, &(BoxInitArray[7u]));//H7

	//CenterRotを反映
	for(uint8_t i = 0u; i < 8u; i++)
	{
		RB_Vec3f RotVec;
		RB_MulMatVec3f(&Object->CenterRot, &BoxInitArray[i], &RotVec);
		RB_Vec3fAdd(&Object->CenterPos, &RotVec, &BoxVertex[i]);
	}

	//描画する
	float objectsolid_val = 0.3;

	fprintf(plt_3d, "set style fill transparent solid %f \n", objectsolid_val);
	fprintf(plt_3d, "set obj %u polygon from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
		depthorder fillcolor \"#0918e6\" \n", \
		object_num, \
		RB_Vec3fGetElem(&BoxVertex[0u], 0u),RB_Vec3fGetElem(&BoxVertex[0u], 1u),RB_Vec3fGetElem(&BoxVertex[0u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[1u], 0u),RB_Vec3fGetElem(&BoxVertex[1u], 1u),RB_Vec3fGetElem(&BoxVertex[1u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[2u], 0u),RB_Vec3fGetElem(&BoxVertex[2u], 1u),RB_Vec3fGetElem(&BoxVertex[2u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[3u], 0u),RB_Vec3fGetElem(&BoxVertex[3u], 1u),RB_Vec3fGetElem(&BoxVertex[3u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[0u], 0u),RB_Vec3fGetElem(&BoxVertex[0u], 1u),RB_Vec3fGetElem(&BoxVertex[0u], 2u)  \
	);
	object_num++;

	fprintf(plt_3d, "set obj %u polygon from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
		depthorder fillcolor \"#33AAAA\" \n", \
		object_num, \
		RB_Vec3fGetElem(&BoxVertex[3u], 0u),RB_Vec3fGetElem(&BoxVertex[3u], 1u),RB_Vec3fGetElem(&BoxVertex[3u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[2u], 0u),RB_Vec3fGetElem(&BoxVertex[2u], 1u),RB_Vec3fGetElem(&BoxVertex[2u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[6u], 0u),RB_Vec3fGetElem(&BoxVertex[6u], 1u),RB_Vec3fGetElem(&BoxVertex[6u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[7u], 0u),RB_Vec3fGetElem(&BoxVertex[7u], 1u),RB_Vec3fGetElem(&BoxVertex[7u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[3u], 0u),RB_Vec3fGetElem(&BoxVertex[3u], 1u),RB_Vec3fGetElem(&BoxVertex[3u], 2u)  \
	);
	object_num++;

	fprintf(plt_3d, "set obj %u polygon from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
		depthorder fillcolor \"#33AAAA\" \n", \
		object_num, \
		RB_Vec3fGetElem(&BoxVertex[1u], 0u),RB_Vec3fGetElem(&BoxVertex[1u], 1u),RB_Vec3fGetElem(&BoxVertex[1u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[5u], 0u),RB_Vec3fGetElem(&BoxVertex[5u], 1u),RB_Vec3fGetElem(&BoxVertex[5u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[6u], 0u),RB_Vec3fGetElem(&BoxVertex[6u], 1u),RB_Vec3fGetElem(&BoxVertex[6u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[2u], 0u),RB_Vec3fGetElem(&BoxVertex[2u], 1u),RB_Vec3fGetElem(&BoxVertex[2u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[1u], 0u),RB_Vec3fGetElem(&BoxVertex[1u], 1u),RB_Vec3fGetElem(&BoxVertex[1u], 2u)  \
	);
	object_num++;

	fprintf(plt_3d, "set obj %u polygon from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
		depthorder fillcolor \"#33AAAA\" \n", \
		object_num, \
		RB_Vec3fGetElem(&BoxVertex[4u], 0u),RB_Vec3fGetElem(&BoxVertex[4u], 1u),RB_Vec3fGetElem(&BoxVertex[4u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[0u], 0u),RB_Vec3fGetElem(&BoxVertex[0u], 1u),RB_Vec3fGetElem(&BoxVertex[0u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[3u], 0u),RB_Vec3fGetElem(&BoxVertex[3u], 1u),RB_Vec3fGetElem(&BoxVertex[3u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[7u], 0u),RB_Vec3fGetElem(&BoxVertex[7u], 1u),RB_Vec3fGetElem(&BoxVertex[7u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[4u], 0u),RB_Vec3fGetElem(&BoxVertex[4u], 1u),RB_Vec3fGetElem(&BoxVertex[4u], 2u)  \
	);
	object_num++;

	fprintf(plt_3d, "set style fill transparent solid %f \n", objectsolid_val);
	fprintf(plt_3d, "set obj %u polygon from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
		depthorder fillcolor \"#33AAAA\" \n", \
		object_num, \
		RB_Vec3fGetElem(&BoxVertex[4u], 0u),RB_Vec3fGetElem(&BoxVertex[4u], 1u),RB_Vec3fGetElem(&BoxVertex[4u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[5u], 0u),RB_Vec3fGetElem(&BoxVertex[5u], 1u),RB_Vec3fGetElem(&BoxVertex[5u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[1u], 0u),RB_Vec3fGetElem(&BoxVertex[1u], 1u),RB_Vec3fGetElem(&BoxVertex[1u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[0u], 0u),RB_Vec3fGetElem(&BoxVertex[0u], 1u),RB_Vec3fGetElem(&BoxVertex[0u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[4u], 0u),RB_Vec3fGetElem(&BoxVertex[4u], 1u),RB_Vec3fGetElem(&BoxVertex[4u], 2u)  \
	);
	object_num++;

	fprintf(plt_3d, "set style fill transparent solid %f \n", objectsolid_val);
	fprintf(plt_3d, "set obj %u polygon from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
		depthorder fillcolor \"#33AAAA\" \n", \
		object_num, \
		RB_Vec3fGetElem(&BoxVertex[5u], 0u),RB_Vec3fGetElem(&BoxVertex[5u], 1u),RB_Vec3fGetElem(&BoxVertex[5u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[4u], 0u),RB_Vec3fGetElem(&BoxVertex[4u], 1u),RB_Vec3fGetElem(&BoxVertex[4u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[7u], 0u),RB_Vec3fGetElem(&BoxVertex[7u], 1u),RB_Vec3fGetElem(&BoxVertex[7u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[6u], 0u),RB_Vec3fGetElem(&BoxVertex[6u], 1u),RB_Vec3fGetElem(&BoxVertex[6u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[5u], 0u),RB_Vec3fGetElem(&BoxVertex[5u], 1u),RB_Vec3fGetElem(&BoxVertex[5u], 2u)  \
	);
}

RBSTATIC void DrawBoxObjectSize(uint32_t id, OBJECT_T *Object)
{
	uint32_t arrow_num = ( 3 * (id + 100u)) -2;

	RB_Vec3f *v = &Object->CenterPos;
	RB_Mat3f *m = &Object->CenterRot;
	BOX_T *Box_obj = &Object->Box;
	RB_Vec3f *l = &Box_obj->BoxSize;

	fprintf(plt_3d,"set colorsequence default\n");
	fprintf(plt_3d,"set arrow %u from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f front lw 2 lt rgbcolor \'spring-green\' \n",\
	arrow_num, \
	v->e[0],v->e[1],v->e[2],\
//=====================================
	(v->e[0u] + l->e[0u] * RB_Mat3fGetElem(m, 0u, 0u)),\
	(v->e[1u] + l->e[0u] * RB_Mat3fGetElem(m, 1u, 0u)),\
	(v->e[2u] + l->e[0u] * RB_Mat3fGetElem(m, 2u, 0u))\
	);//x
	arrow_num++;

	fprintf(plt_3d,"set arrow %u from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f front lw 2 lt rgbcolor \'cyan\'  \n",\
	arrow_num, \
	v->e[0],v->e[1],v->e[2],\
//=====================================
	(v->e[0u] + l->e[1u] * RB_Mat3fGetElem(m, 0u, 1u)),\
	(v->e[1u] + l->e[1u] * RB_Mat3fGetElem(m, 1u, 1u)),\
	(v->e[2u] + l->e[1u] * RB_Mat3fGetElem(m, 2u, 1u))\
	);//x
	arrow_num++;

	fprintf(plt_3d,"set arrow %u from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f front lw 2 lt rgbcolor \'orange\'  \n",\
	arrow_num, \
	v->e[0],v->e[1],v->e[2],\
//=====================================
	(v->e[0u] + l->e[2u] * RB_Mat3fGetElem(m, 0u, 2u)),\
	(v->e[1u] + l->e[2u] * RB_Mat3fGetElem(m, 1u, 2u)),\
	(v->e[2u] + l->e[2u] * RB_Mat3fGetElem(m, 2u, 2u))\
	);//x

	fprintf(plt_3d,"set colorsequence classic\n");
}

RBSTATIC void DrawAreaObject(void)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	for(uint32_t i = 1u; i < (uint32_t)OBJECT_MAXID; i++)
	{
		if((ObjectData[i].ShapeType == 0))
		{
			DrawBox(i, &ObjectData[i]);
		}
	}

}

RBSTATIC void DrawObjectArrow(void)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	for(uint32_t i = 1u; i < (uint32_t)OBJECT_MAXID; i++)
	{
		DrawBoxObjectSize(i, &ObjectData[i]);
	}

}

RBSTATIC void ColorConfig(void)
{
	fprintf(plt_3d,"set xyplane 0        # \'set ticslevel 0\' is obsolute \n");
	//fprintf(plt_3d,"set palette defined (0 \"dark-blue\", 1 \"light-blue\") \n");
	fprintf(plt_3d,"set palette defined (0 \"steelblue\", 1 \"steelblue\") \n");
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
	fprintf(plt_3d,"set pm3d nohidden3d depthorder\n");
	fprintf(plt_3d, "set size ratio -1\n");
	fprintf(plt_3d, "set view equal xyz\n");

	fprintf(plt_3d,"set xrange[-%d:%d] \n",f_plot_width, f_plot_width);
	fprintf(plt_3d,"set yrange[-%d:%d] \n",f_plot_width, f_plot_width);
	fprintf(plt_3d,"set zrange[0:%d] \n",(f_plot_width * 2));
	//fprintf(plt_3d,"set zrange[-%d:%d] \n",500, (f_plot_width * 2));

	fprintf(plt_3d,"set ticslevel 0\n");/// z軸をxy平面に接続するコマンド
	fprintf(plt_3d,"set view 60,110 \n");
	fprintf(plt_3d,"set nogrid\n");
	//fprintf(plt_3d,"set grid\n");

	//参考: https://ayapin-film.sakura.ne.jp/Gnuplot/Primer/Parametric/3dparam_sphere_pm3d.html
	fprintf(plt_3d, "set macro \n");
	
	fprintf(plt_3d, "FSx=\"F_sphere_x($1,$2)\" \n");
	fprintf(plt_3d, "FSy=\"F_sphere_y($1,$2)\" \n");
	fprintf(plt_3d, "FSz=\"F_sphere_z($1,$2)\" \n");

	fprintf(plt_3d, "FCX_x=\"F_cylinderX_x($2)\" \n");
	fprintf(plt_3d, "FCX_y=\"F_cylinderX_y($1)\" \n");
	fprintf(plt_3d, "FCX_z=\"F_cylinderX_z($1)\" \n");

	fprintf(plt_3d, "FCY_x=\"F_cylinderY_x($1)\" \n");
	fprintf(plt_3d, "FCY_y=\"F_cylinderY_y($2)\" \n");
	fprintf(plt_3d, "FCY_z=\"F_cylinderY_z($1)\" \n");

	fprintf(plt_3d, "FCZ_x=\"F_cylinderZ_x($1)\" \n");
	fprintf(plt_3d, "FCZ_y=\"F_cylinderZ_y($1)\" \n");
	fprintf(plt_3d, "FCZ_z=\"F_cylinderZ_z($2)\" \n");
}

RBSTATIC void CoordinateSys_Config(uint32_t id, RBCONST RB_Vec3f *v, float length, RBCONST RB_Mat3f *m)
{
	uint32_t arrow_num = (3 * (id + 1u)) - 2;

	fprintf(plt_3d,"set colorsequence classic\n");
	fprintf(plt_3d,"set arrow %u from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f front lw 2 lt 2 \n",\
	arrow_num, \
	v->e[0u],v->e[1u],v->e[2u],\
//=====================================
	(v->e[0u] + length * RB_Mat3fGetElem(m, 0u, 0u)),\
	(v->e[1u] + length * RB_Mat3fGetElem(m, 1u, 0u)),\
	(v->e[2u] + length * RB_Mat3fGetElem(m, 2u, 0u))\
	);//x

	arrow_num++;
	fprintf(plt_3d,"set arrow %u from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f front lw 2 lt 3 \n",\
	arrow_num, \
	v->e[0u],v->e[1u],v->e[2u],\
//=====================================
	(v->e[0u] + length * RB_Mat3fGetElem(m, 0u, 1u)),\
	(v->e[1u] + length * RB_Mat3fGetElem(m, 1u, 1u)),\
	(v->e[2u] + length * RB_Mat3fGetElem(m, 2u, 1u))\
	);//y

	arrow_num++;
	fprintf(plt_3d,"set arrow %u from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f front lw 2 lt 1 \n",\
	arrow_num, \
	v->e[0u],v->e[1u],v->e[2u],\
//=====================================
	(v->e[0u] + length * RB_Mat3fGetElem(m, 0u, 2u)),\
	(v->e[1u] + length * RB_Mat3fGetElem(m, 1u, 2u)),\
	(v->e[2u] + length * RB_Mat3fGetElem(m, 2u, 2u))\
	);//z

}

RBSTATIC void GenerateObjectSplot(uint8_t id)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

#if 0
//角度のためし
	uint8_t AxisType = 0u;
	RB_Vec3f rel;
	RB_Mat3f OffsetRot;
	RB_Vec3f RotAxis = f_RB_Vec3fzero;

	RB_Vec3fCreate(0.0f, 100.0f, 100.0f, &rel);
	float radian = 0.0f;
	switch(AxisType)
	{
		case 0:
			RB_Vec3fCreate(1.0f, 0.0f, 0.0f, &RotAxis);
			radian = CalcAngleBetweenVec3f(2u, &rel);
			break;

		case 1:
			RB_Vec3fCreate(0.0f, 1.0f, 0.0f, &RotAxis);
			radian = CalcAngleBetweenVec3f(0u, &rel);
			break;

		case 2:
			RB_Vec3fCreate(0.0f, 0.0f, 1.0f, &RotAxis);
			radian = CalcAngleBetweenVec3f(1u, &rel);
			break;

		default:
				NO_STATEMENT;
			break;

	}
	
	RB_AxisRotateMat3f(&RotAxis, radian, &OffsetRot);
	RB_Mat3f NowRot = ObjectData[id].CenterRot;
	RB_MulMatMat3f(&NowRot, &OffsetRot, &(ObjectData[id].CenterRot));
//==============
#endif

	//注意! gnuplot の配列は0からではなく1からスタート
	fprintf(plt_3d, "array M%u_[9] = [%.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f]\n", \
		id, \
		RB_Mat3fGetElem(&(ObjectData[id].CenterRot), 0u, 0u), \
		RB_Mat3fGetElem(&(ObjectData[id].CenterRot), 0u, 1u), \
		RB_Mat3fGetElem(&(ObjectData[id].CenterRot), 0u, 2u), \
		RB_Mat3fGetElem(&(ObjectData[id].CenterRot), 1u, 0u), \
		RB_Mat3fGetElem(&(ObjectData[id].CenterRot), 1u, 1u), \
		RB_Mat3fGetElem(&(ObjectData[id].CenterRot), 1u, 2u), \
		RB_Mat3fGetElem(&(ObjectData[id].CenterRot), 2u, 0u), \
		RB_Mat3fGetElem(&(ObjectData[id].CenterRot), 2u, 1u), \
		RB_Mat3fGetElem(&(ObjectData[id].CenterRot), 2u, 2u));

	fprintf(plt_3d, "array P%u_[3] = [%.3f, %.3f, %.3f]\n", \
		id, \
		RB_Vec3fGetElem(&(ObjectData[id].CenterPos), 0u), \
		RB_Vec3fGetElem(&(ObjectData[id].CenterPos), 1u), \
		RB_Vec3fGetElem(&(ObjectData[id].CenterPos), 2u));

	switch(ObjectData[id].ShapeType)
	{
		case 2:
				SSV_T *Capsule_obj = &(ObjectData[id].Capsule);
				uint8_t CapsuleAxisType = Capsule_obj->AxisType;
				RB_Vec3f CapsuleEndPos = Capsule_obj->EndPos;

				switch(CapsuleAxisType)
				{
					case 0u:
							fprintf(plt_3d, "array L%u_[3] = [%.3f, %.3f, %.3f]\n", \
								id, \
								CapsuleEndPos.e[0u], \
								Capsule_obj->Radius, \
								Capsule_obj->Radius);
						break;

					case 1u:
							fprintf(plt_3d, "array L%u_[3] = [%.3f, %.3f, %.3f]\n", \
								id, \
								Capsule_obj->Radius, \
								CapsuleEndPos.e[1u], \
								Capsule_obj->Radius);
						break;

					case 2u:
							fprintf(plt_3d, "array L%u_[3] = [%.3f, %.3f, %.3f]\n", \
								id, \
								Capsule_obj->Radius, \
								Capsule_obj->Radius, \
								CapsuleEndPos.e[2u]);
						break;

					default:
						break;
				}
			break;

		case 3:
				SSV_T *Cylinder_obj = &(ObjectData[id].Cylinder);
				uint8_t CylinderAxisType = Cylinder_obj->AxisType;
				RB_Vec3f CylinderEndPos = Cylinder_obj->EndPos;

				switch(CylinderAxisType)
				{
					case 0u:
							fprintf(plt_3d, "array L%u_[3] = [%.3f, %.3f, %.3f]\n", \
								id, \
								CylinderEndPos.e[0u], \
								Cylinder_obj->Radius, \
								Cylinder_obj->Radius);
						break;

					case 1u:
							fprintf(plt_3d, "array L%u_[3] = [%.3f, %.3f, %.3f]\n", \
								id, \
								Cylinder_obj->Radius, \
								CylinderEndPos.e[1u], \
								Cylinder_obj->Radius);
						break;

					case 2u:
							fprintf(plt_3d, "array L%u_[3] = [%.3f, %.3f, %.3f]\n", \
								id, \
								Cylinder_obj->Radius, \
								Cylinder_obj->Radius, \
								CylinderEndPos.e[2u]);
						break;

					default:
						break;
				}
			break;

		default:
				SSV_T *Sphere_obj = &(ObjectData[id].Sphere);
				fprintf(plt_3d, "array L%u_[3] = [%.3f, %.3f, %.3f]\n", \
					id, \
					Sphere_obj->Radius, \
					0.0f, \
					0.0f );
			break;
	}

}

RBSTATIC void SphereFprintf(void)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);
	uint32_t id = 6u;

	GenerateObjectSplot(id);
	fprintf(plt_3d," splot -10, \
			sphere using \
							( ( (M6_[1] * (@FSx * L6_[1])) + (M6_[2] * (@FSy * L6_[1])) + (M6_[3] * (@FSz * L6_[1])) ) + P6_[1]):\
							( ( (M6_[4] * (@FSx * L6_[1])) + (M6_[5] * (@FSy * L6_[1])) + (M6_[6] * (@FSz * L6_[1])) ) + P6_[2]):\
							( ( (M6_[7] * (@FSx * L6_[1])) + (M6_[8] * (@FSy * L6_[1])) + (M6_[9] * (@FSz * L6_[1])) ) + P6_[3]) with pm3d \
			\
	\n");	
}

RBSTATIC void CapsuleFprintf(void)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);
	uint32_t id = 6u;

	GenerateObjectSplot(id);

	SSV_T capsule_obj = { 0 };
	capsule_obj = ObjectData[id].Capsule;

	switch(capsule_obj.AxisType)
	{
		case 2:
			fprintf(plt_3d," splot -10, \
					\
					cylinderZ using \
									( ( (M6_[1] * (@FCZ_x * L6_[1])) + (M6_[2] * (@FCZ_y * L6_[2])) + (M6_[3] * (@FCZ_z * L6_[3])) ) + P6_[1]):\
									( ( (M6_[4] * (@FCZ_x * L6_[1])) + (M6_[5] * (@FCZ_y * L6_[2])) + (M6_[6] * (@FCZ_z * L6_[3])) ) + P6_[2]):\
									( ( (M6_[7] * (@FCZ_x * L6_[1])) + (M6_[8] * (@FCZ_y * L6_[2])) + (M6_[9] * (@FCZ_z * L6_[3])) ) + P6_[3]) with pm3d, \
					sphere using \
									( ( (M6_[1] * (@FSx * L6_[1])) + (M6_[2] * (@FSy * L6_[1])) + (M6_[3] * (@FSz * L6_[1])) ) + P6_[1]):\
									( ( (M6_[4] * (@FSx * L6_[1])) + (M6_[5] * (@FSy * L6_[1])) + (M6_[6] * (@FSz * L6_[1])) ) + P6_[2]):\
									( ( (M6_[7] * (@FSx * L6_[1])) + (M6_[8] * (@FSy * L6_[1])) + (M6_[9] * (@FSz * L6_[1])) ) + P6_[3]) with pm3d ,\
					sphere using \
									( ( (M6_[1] * (@FSx * L6_[1])) + (M6_[2] * (@FSy * L6_[1])) + (M6_[3] * (@FSz * L6_[1])) ) + ( (M6_[3] * L6_[3]) ) + P6_[1]):\
									( ( (M6_[4] * (@FSx * L6_[1])) + (M6_[5] * (@FSy * L6_[1])) + (M6_[6] * (@FSz * L6_[1])) ) + ( (M6_[6] * L6_[3]) ) + P6_[2]):\
									( ( (M6_[7] * (@FSx * L6_[1])) + (M6_[8] * (@FSy * L6_[1])) + (M6_[9] * (@FSz * L6_[1])) ) + ( (M6_[9] * L6_[3]) ) + P6_[3]) with pm3d \
									\
					\
					\n");
			break;

		case 1:
			fprintf(plt_3d," splot -10, \
					\
					cylinderY using \
									( ( (M6_[1] * (@FCY_x * L6_[1])) + (M6_[2] * (@FCY_y * L6_[2])) + (M6_[3] * (@FCY_z * L6_[3])) ) + P6_[1]):\
									( ( (M6_[4] * (@FCY_x * L6_[1])) + (M6_[5] * (@FCY_y * L6_[2])) + (M6_[6] * (@FCY_z * L6_[3])) ) + P6_[2]):\
									( ( (M6_[7] * (@FCY_x * L6_[1])) + (M6_[8] * (@FCY_y * L6_[2])) + (M6_[9] * (@FCY_z * L6_[3])) ) + P6_[3]) with pm3d, \
					sphere using \
									( ( (M6_[1] * (@FSx * L6_[1])) + (M6_[2] * (@FSy * L6_[1])) + (M6_[3] * (@FSz * L6_[1])) ) + P6_[1]):\
									( ( (M6_[4] * (@FSx * L6_[1])) + (M6_[5] * (@FSy * L6_[1])) + (M6_[6] * (@FSz * L6_[1])) ) + P6_[2]):\
									( ( (M6_[7] * (@FSx * L6_[1])) + (M6_[8] * (@FSy * L6_[1])) + (M6_[9] * (@FSz * L6_[1])) ) + P6_[3]) with pm3d ,\
					sphere using \
									( ( (M6_[1] * (@FSx * L6_[1])) + (M6_[2] * (@FSy * L6_[1])) + (M6_[3] * (@FSz * L6_[1])) ) + ( (M6_[2] * L6_[2]) ) + P6_[1]):\
									( ( (M6_[4] * (@FSx * L6_[1])) + (M6_[5] * (@FSy * L6_[1])) + (M6_[6] * (@FSz * L6_[1])) ) + ( (M6_[5] * L6_[2]) ) + P6_[2]):\
									( ( (M6_[7] * (@FSx * L6_[1])) + (M6_[8] * (@FSy * L6_[1])) + (M6_[9] * (@FSz * L6_[1])) ) + ( (M6_[8] * L6_[2]) ) + P6_[3]) with pm3d \
									\
					\
					\n");
			break;

		case 0:
			fprintf(plt_3d," splot -10, \
					\
					cylinderX using \
									( ( (M6_[1] * (@FCX_x * L6_[1])) + (M6_[2] * (@FCX_y * L6_[2])) + (M6_[3] * (@FCX_z * L6_[3])) ) + P6_[1]):\
									( ( (M6_[4] * (@FCX_x * L6_[1])) + (M6_[5] * (@FCX_y * L6_[2])) + (M6_[6] * (@FCX_z * L6_[3])) ) + P6_[2]):\
									( ( (M6_[7] * (@FCX_x * L6_[1])) + (M6_[8] * (@FCX_y * L6_[2])) + (M6_[9] * (@FCX_z * L6_[3])) ) + P6_[3]) with pm3d, \
					sphere using \
									( ( (M6_[1] * (@FSx * L6_[2])) + (M6_[2] * (@FSy * L6_[2])) + (M6_[3] * (@FSz * L6_[2])) ) + P6_[1]):\
									( ( (M6_[4] * (@FSx * L6_[2])) + (M6_[5] * (@FSy * L6_[2])) + (M6_[6] * (@FSz * L6_[2])) ) + P6_[2]):\
									( ( (M6_[7] * (@FSx * L6_[2])) + (M6_[8] * (@FSy * L6_[2])) + (M6_[9] * (@FSz * L6_[2])) ) + P6_[3]) with pm3d ,\
					sphere using \
									( ( (M6_[1] * (@FSx * L6_[2])) + (M6_[2] * (@FSy * L6_[2])) + (M6_[3] * (@FSz * L6_[2])) ) + ( (M6_[1] * L6_[1]) ) + P6_[1]):\
									( ( (M6_[4] * (@FSx * L6_[2])) + (M6_[5] * (@FSy * L6_[2])) + (M6_[6] * (@FSz * L6_[2])) ) + ( (M6_[4] * L6_[1]) ) + P6_[2]):\
									( ( (M6_[7] * (@FSx * L6_[2])) + (M6_[8] * (@FSy * L6_[2])) + (M6_[9] * (@FSz * L6_[2])) ) + ( (M6_[7] * L6_[1]) ) + P6_[3]) with pm3d \
									\
					\
					\n");
			break;

		default:	
			break;
	}
}

RBSTATIC void CylinderFprintf(void)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);
	uint32_t id = 6u;

	GenerateObjectSplot(id);

	SSV_T cylinder_obj = { 0 };
	cylinder_obj = ObjectData[id].Cylinder;

	switch(cylinder_obj.AxisType)
	{
		case 2:
			fprintf(plt_3d," splot -10, \
					\
					cylinderZ using \
									( ( (M6_[1] * (@FCZ_x * L6_[1])) + (M6_[2] * (@FCZ_y * L6_[2])) + (M6_[3] * (@FCZ_z * L6_[3])) ) + P6_[1]):\
									( ( (M6_[4] * (@FCZ_x * L6_[1])) + (M6_[5] * (@FCZ_y * L6_[2])) + (M6_[6] * (@FCZ_z * L6_[3])) ) + P6_[2]):\
									( ( (M6_[7] * (@FCZ_x * L6_[1])) + (M6_[8] * (@FCZ_y * L6_[2])) + (M6_[9] * (@FCZ_z * L6_[3])) ) + P6_[3]) with pm3d, \
					\
					cylinderZ using \
									( ( (M6_[1] * (@FCZ_x * L6_[1])) + (M6_[2] * (@FCZ_y * L6_[2])) + (M6_[3] * (@FCZ_z * L6_[3])) ) + P6_[1]):\
									( ( (M6_[4] * (@FCZ_x * L6_[1])) + (M6_[5] * (@FCZ_y * L6_[2])) + (M6_[6] * (@FCZ_z * L6_[3])) ) + P6_[2]):\
									( ( (M6_[7] * (@FCZ_x * L6_[1])) + (M6_[8] * (@FCZ_y * L6_[2])) + (M6_[9] * (@FCZ_z * L6_[3])) ) + P6_[3]) with polygons fc \"steelblue\"\n");
			break;

		case 1:
			fprintf(plt_3d," splot -10, \
					\
					cylinderY using \
									( ( (M6_[1] * (@FCY_x * L6_[1])) + (M6_[2] * (@FCY_y * L6_[2])) + (M6_[3] * (@FCY_z * L6_[3])) ) + P6_[1]):\
									( ( (M6_[4] * (@FCY_x * L6_[1])) + (M6_[5] * (@FCY_y * L6_[2])) + (M6_[6] * (@FCY_z * L6_[3])) ) + P6_[2]):\
									( ( (M6_[7] * (@FCY_x * L6_[1])) + (M6_[8] * (@FCY_y * L6_[2])) + (M6_[9] * (@FCY_z * L6_[3])) ) + P6_[3]) with pm3d, \
					\
					cylinderY using \
									( ( (M6_[1] * (@FCY_x * L6_[1])) + (M6_[2] * (@FCY_y * L6_[2])) + (M6_[3] * (@FCY_z * L6_[3])) ) + P6_[1]):\
									( ( (M6_[4] * (@FCY_x * L6_[1])) + (M6_[5] * (@FCY_y * L6_[2])) + (M6_[6] * (@FCY_z * L6_[3])) ) + P6_[2]):\
									( ( (M6_[7] * (@FCY_x * L6_[1])) + (M6_[8] * (@FCY_y * L6_[2])) + (M6_[9] * (@FCY_z * L6_[3])) ) + P6_[3]) with polygons fc \"steelblue\"\n");

		case 0:
		fprintf(plt_3d," splot -10, \
				cylinderX using \
								( ( (M6_[1] * (@FCX_x * L6_[1])) + (M6_[2] * (@FCX_y * L6_[2])) + (M6_[3] * (@FCX_z * L6_[3])) ) + P6_[1]):\
								( ( (M6_[4] * (@FCX_x * L6_[1])) + (M6_[5] * (@FCX_y * L6_[2])) + (M6_[6] * (@FCX_z * L6_[3])) ) + P6_[2]):\
								( ( (M6_[7] * (@FCX_x * L6_[1])) + (M6_[8] * (@FCX_y * L6_[2])) + (M6_[9] * (@FCX_z * L6_[3])) ) + P6_[3]) with pm3d, \
				\
				cylinderX using \
								( ( (M6_[1] * (@FCX_x * L6_[1])) + (M6_[2] * (@FCX_y * L6_[2])) + (M6_[3] * (@FCX_z * L6_[3])) ) + P6_[1]):\
								( ( (M6_[4] * (@FCX_x * L6_[1])) + (M6_[5] * (@FCX_y * L6_[2])) + (M6_[6] * (@FCX_z * L6_[3])) ) + P6_[2]):\
								( ( (M6_[7] * (@FCX_x * L6_[1])) + (M6_[8] * (@FCX_y * L6_[2])) + (M6_[9] * (@FCX_z * L6_[3])) ) + P6_[3]) with polygons fc \"steelblue\"\n");
		default:	
			break;
	}
}

RBSTATIC void SplotData(void)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	//SphereFprintf();
	//CylinderFprintf();
	CapsuleFprintf();
}

RBSTATIC void DrawWorldCoordinateSys(void)
{
	float axis_length = 100.0f;
	CoordinateSys_Config(0u, &f_RB_Vec3fzero, axis_length, &f_RB_Mat3fident);
}

RBSTATIC void DrawCoordinateSys(void)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	for(uint8_t id = 1u; id < (uint32_t)OBJECT_MAXID; id++)
	{
		float axis_length = (ObjectData[id].TFMode) ? 200.0f : 0.0f;
		CoordinateSys_Config(id, &(ObjectData[id].CenterPos), axis_length, &(ObjectData[id].CenterRot));

		uint32_t label_num = id + 100u;

		RB_Vec3f LabelPos;
		RB_Vec3f Offset;
		RB_Vec3fCreate(50.0f, 50.0f, 100.0f, &Offset);
		RB_Vec3fAdd(&(ObjectData[id].CenterPos), &Offset, &LabelPos);
		fprintf(plt_3d, "set label %u \'id:%u \' font \"Times,10\" at %.3f, %.3f, %.3f \n", \
		label_num, \
		id, \
		RB_Vec3fGetElem(&LabelPos,0u),\
		RB_Vec3fGetElem(&LabelPos,1u),\
		RB_Vec3fGetElem(&LabelPos,2u)\
		);
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
	GenerateSphereDat();
	GenerateCylinderDat();
	ColorConfig();
	UnsetConfig();
	SetConfig();
	DrawGround(0.0f);
}

void GnuPlot_Cycle(void)
{
//コマンドで変化するものたち
	DrawWorldCoordinateSys();
	DrawCoordinateSys();
	DrawAreaObject();
	DrawObjectArrow();
	SplotData();

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
