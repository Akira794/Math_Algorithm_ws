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
RBSTATIC void GenerateCylinderDat(void);

RBSTATIC void DrawBox(uint32_t id, OBJECT_T *Object);
RBSTATIC void DrawObjectSize(uint32_t id, OBJECT_T *Object);
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

RBSTATIC void GenerateCylinderDat(void)
{
	fprintf(plt_3d, "F_cylinder_x(u)=cos(u)\n");
	fprintf(plt_3d, "F_cylinder_y(u)=sin(u)\n");
	fprintf(plt_3d, "F_cylinder_z(v)=v\n");

	fprintf(plt_3d, "set xrange [-pi:pi] \n");
	fprintf(plt_3d, "set yrange [0:1] \n");

	fprintf(plt_3d, "set samples 24 \n");
	fprintf(plt_3d, "set isosamples 2 \n");

	fprintf(plt_3d, "cylinder=\"cylinder.dat\" \n");
	fprintf(plt_3d, "set table cylinder \n");
	fprintf(plt_3d, "splot 0 \n");
	fprintf(plt_3d, "unset xrange \n");
	fprintf(plt_3d, "unset yrange \n");
}

RBSTATIC void DrawBox(uint32_t id, OBJECT_T *Object)
{
	uint32_t object_num = (6 * (id + 1u)) - 5;
	RB_Vec3f BoxInitArray[8u] = { 0.0f };
	RB_Vec3f BoxVertex[8u] = { 0.0f };
	RB_Vec3f width3f;

	float lx = RB_Vec3fGetElem(&Object->C_AxisLength, 0u);
	float ly = RB_Vec3fGetElem(&Object->C_AxisLength, 1u);
	float lz = RB_Vec3fGetElem(&Object->C_AxisLength, 2u);

switch(Object->WidthType)
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

	//C_Rotを反映
	for(uint8_t i = 0u; i < 8u; i++)
	{
		RB_Vec3f RotVec;
		RB_MulMatVec3f(&Object->C_Rot, &BoxInitArray[i], &RotVec);
		RB_Vec3fAdd(&Object->C_Pos, &RotVec, &BoxVertex[i]);
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

RBSTATIC void DrawObjectSize(uint32_t id, OBJECT_T *Object)
{
	uint32_t arrow_num = ( 3 * (id + 100u)) -2;

	RB_Vec3f *v = &Object->C_Pos;
	RB_Vec3f *l = &Object->C_AxisLength;
	RB_Mat3f *m = &Object->C_Rot;

	fprintf(plt_3d,"set colorsequence default\n");
	fprintf(plt_3d,"set arrow %u from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f front lw 2 lt 2 \n",\
	arrow_num, \
	v->e[0],v->e[1],v->e[2],\
//=====================================
	(v->e[0u] + l->e[0u] * RB_Mat3fGetElem(m, 0u, 0u)),\
	(v->e[1u] + l->e[0u] * RB_Mat3fGetElem(m, 1u, 0u)),\
	(v->e[2u] + l->e[0u] * RB_Mat3fGetElem(m, 2u, 0u))\
	);//x
	arrow_num++;

	fprintf(plt_3d,"set arrow %u from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f front lw 2 lt 6 \n",\
	arrow_num, \
	v->e[0],v->e[1],v->e[2],\
//=====================================
	(v->e[0u] + l->e[1u] * RB_Mat3fGetElem(m, 0u, 1u)),\
	(v->e[1u] + l->e[1u] * RB_Mat3fGetElem(m, 1u, 1u)),\
	(v->e[2u] + l->e[1u] * RB_Mat3fGetElem(m, 2u, 1u))\
	);//x
	arrow_num++;

	fprintf(plt_3d,"set arrow %u from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f front lw 2 lt 4 \n",\
	arrow_num, \
	v->e[0],v->e[1],v->e[2],\
//=====================================
	(v->e[0u] + l->e[2u] * RB_Mat3fGetElem(m, 0u, 2u)),\
	(v->e[1u] + l->e[2u] * RB_Mat3fGetElem(m, 1u, 2u)),\
	(v->e[2u] + l->e[2u] * RB_Mat3fGetElem(m, 2u, 2u))\
	);//x

	fprintf(plt_3d,"set colorsequence classic\n");
}

RBSTATIC void DrawObject(void)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	for(uint32_t i = 1u; i < (uint32_t)OBJECT_MAXID; i++)
	{
		if((ObjectData[i].ShapeType == 0))
		{
			DrawBox(i, &ObjectData[i]);
		}
		DrawObjectSize(i, &ObjectData[i]);
	}
}

#if 1
RBSTATIC void ColorConfig(void)
{
	fprintf(plt_3d,"set xyplane 0        # \'set ticslevel 0\' is obsolute \n");
	//fprintf(plt_3d,"set palette defined (0 \"dark-blue\", 1 \"light-blue\") \n");
	fprintf(plt_3d,"set palette defined (0 \"steelblue\", 1 \"steelblue\") \n");
}
#endif
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

	//注意! gnuplot の配列は0からではなく1からスタート
	fprintf(plt_3d, "array M%u_[9] = [%.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f]\n", \
		id, \
		RB_Mat3fGetElem(&(ObjectData[id].C_Rot), 0u, 0u), \
		RB_Mat3fGetElem(&(ObjectData[id].C_Rot), 0u, 1u), \
		RB_Mat3fGetElem(&(ObjectData[id].C_Rot), 0u, 2u), \
		RB_Mat3fGetElem(&(ObjectData[id].C_Rot), 1u, 0u), \
		RB_Mat3fGetElem(&(ObjectData[id].C_Rot), 1u, 1u), \
		RB_Mat3fGetElem(&(ObjectData[id].C_Rot), 1u, 2u), \
		RB_Mat3fGetElem(&(ObjectData[id].C_Rot), 2u, 0u), \
		RB_Mat3fGetElem(&(ObjectData[id].C_Rot), 2u, 1u), \
		RB_Mat3fGetElem(&(ObjectData[id].C_Rot), 2u, 2u));

	fprintf(plt_3d, "array L%u_[3] = [%.3f, %.3f, %.3f]\n", \
		id, \
		RB_Vec3fGetElem(&(ObjectData[id].C_AxisLength), 0u), \
		RB_Vec3fGetElem(&(ObjectData[id].C_AxisLength), 1u), \
		RB_Vec3fGetElem(&(ObjectData[id].C_AxisLength), 2u));

	fprintf(plt_3d, "array P%u_[3] = [%.3f, %.3f, %.3f]\n", \
		id, \
		RB_Vec3fGetElem(&(ObjectData[id].C_Pos), 0u), \
		RB_Vec3fGetElem(&(ObjectData[id].C_Pos), 1u), \
		RB_Vec3fGetElem(&(ObjectData[id].C_Pos), 2u));
}

RBSTATIC void SplotData(void)
{

	//参考: https://ayapin-film.sakura.ne.jp/Gnuplot/Primer/Parametric/3dparam_sphere_pm3d.html
	fprintf(plt_3d, "set macro \n");
	fprintf(plt_3d, "FSx=\"F_sphere_x($1,$2)\" \n");
	fprintf(plt_3d, "FSy=\"F_sphere_y($1,$2)\" \n");
	fprintf(plt_3d, "FSz=\"F_sphere_z($1,$2)\" \n");

	fprintf(plt_3d, "FCx=\"F_cylinder_x($1)\" \n");
	fprintf(plt_3d, "FCy=\"F_cylinder_y($1)\" \n");
	fprintf(plt_3d, "FCz=\"F_cylinder_z($2)\" \n");

	GenerateObjectSplot(6u);
	GenerateObjectSplot(7u);
	GenerateObjectSplot(8u);
	GenerateObjectSplot(9u);

	fprintf(plt_3d," splot -1, \
			cylinder using \
							( ( (M6_[1] * (@FCx * L6_[1])) + (M6_[2] * (@FCy * L6_[1])) + (M6_[3] * (@FCz * L6_[3])) ) + P6_[1]):\
							( ( (M6_[4] * (@FCx * L6_[1])) + (M6_[5] * (@FCy * L6_[1])) + (M6_[6] * (@FCz * L6_[3])) ) + P6_[2]):\
							( ( (M6_[7] * (@FCx * L6_[1])) + (M6_[8] * (@FCy * L6_[1])) + (M6_[9] * (@FCz * L6_[3])) ) + P6_[3]) with pm3d, \
			\
			cylinder using \
							( ( (M6_[1] * (@FCx * L6_[1])) + (M6_[2] * (@FCy * L6_[1])) + (M6_[3] * (@FCz * L6_[3])) ) + P6_[1]):\
							( ( (M6_[4] * (@FCx * L6_[1])) + (M6_[5] * (@FCy * L6_[1])) + (M6_[6] * (@FCz * L6_[3])) ) + P6_[2]):\
							( ( (M6_[7] * (@FCx * L6_[1])) + (M6_[8] * (@FCy * L6_[1])) + (M6_[9] * (@FCz * L6_[3])) ) + P6_[3]) with polygons fc \"steelblue\", \
			\
			sphere using \
							( ( (M7_[1] * (@FSx * L7_[1])) + (M7_[2] * (@FSy * L7_[2])) + (M7_[3] * (@FSz * L7_[3])) ) + P7_[1]):\
							( ( (M7_[4] * (@FSx * L7_[1])) + (M7_[5] * (@FSy * L7_[2])) + (M7_[6] * (@FSz * L7_[3])) ) + P7_[2]):\
							( ( (M7_[7] * (@FSx * L7_[1])) + (M7_[8] * (@FSy * L7_[2])) + (M7_[9] * (@FSz * L7_[3])) ) + P7_[3]) with pm3d, \
			\
			sphere using \
							( ( (M8_[1] * @FSx) + (M8_[2] * @FSy) + (M8_[3] * @FSz) ) * L8_[1] + P8_[1]):\
							( ( (M8_[4] * @FSx) + (M8_[5] * @FSy) + (M8_[6] * @FSz) ) * L8_[1] + P8_[2]):\
							( ( (M8_[7] * @FSx) + (M8_[8] * @FSy) + (M8_[9] * @FSz) ) * L8_[1] + P8_[3]) with pm3d, \
			\
			cylinder using \
							( ( (M9_[1] * (@FCx * L9_[1])) + (M9_[2] * (@FCy * L9_[1])) + (M9_[3] * (@FCz * L9_[3])) ) + P9_[1]):\
							( ( (M9_[4] * (@FCx * L9_[1])) + (M9_[5] * (@FCy * L9_[1])) + (M9_[6] * (@FCz * L9_[3])) ) + P9_[2]):\
							( ( (M9_[7] * (@FCx * L9_[1])) + (M9_[8] * (@FCy * L9_[1])) + (M9_[9] * (@FCz * L9_[3])) ) + P9_[3]) with pm3d, \
			\
			cylinder using \
							( ( (M9_[1] * (@FCx * L9_[1])) + (M9_[2] * (@FCy * L9_[1])) + (M9_[3] * (@FCz * L9_[3])) ) + P9_[1]):\
							( ( (M9_[4] * (@FCx * L9_[1])) + (M9_[5] * (@FCy * L9_[1])) + (M9_[6] * (@FCz * L9_[3])) ) + P9_[2]):\
							( ( (M9_[7] * (@FCx * L9_[1])) + (M9_[8] * (@FCy * L9_[1])) + (M9_[9] * (@FCz * L9_[3])) ) + P9_[3]) with polygons fc \"steelblue\" \
			\
			\n");
}

RBSTATIC void DrawWorldCoordinateSys(void)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	float axis_length = (ObjectData[0u].TFMode) ? 100.0f : 0.0f;
	CoordinateSys_Config(0u, &f_RB_Vec3fzero, axis_length, &f_RB_Mat3fident);
}

RBSTATIC void DrawCoordinateSys(void)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	for(uint8_t id = 1u; id < (uint32_t)OBJECT_MAXID; id++)
	{
		float axis_length = (ObjectData[id].TFMode) ? 200.0f : 0.0f;
		CoordinateSys_Config(id, &(ObjectData[id].C_Pos), axis_length, &(ObjectData[id].C_Rot));


		uint32_t label_num = id + 100u;

		RB_Vec3f LabelPos;
		RB_Vec3f Offset;
		RB_Vec3fCreate(50.0f, 50.0f, 100.0f, &Offset);
		RB_Vec3fAdd(&(ObjectData[id].C_Pos), &Offset, &LabelPos);
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

	SplotData();
	DrawObject();
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
