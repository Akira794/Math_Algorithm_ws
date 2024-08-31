#include "GnuPlot.h"
#include "MainCommon.h"
#include "MainTypeDef.h"
#include "DbgCmd.h"
#include "RB_Math.h"

RBSTATIC FILE *plt_3d;
RBSTATIC int32_t f_plot_width = 1000;
RBSTATIC RBCONST RB_Vec3f f_RB_Vec3fzero = { { 0.0f, 0.0f, 0.0f} };
RBSTATIC RBCONST RB_Mat3f f_RB_Mat3fident = { { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } } };
RBSTATIC uint32_t f_ObjectStartId[OBJECT_MAXID] = { 0u };

RBSTATIC void ReservationObjectId(void);
RBSTATIC void DrawGround(float z);

RBSTATIC void CreateSphere(void);
RBSTATIC void DrawSphere(void);

RBSTATIC void CreateCylinderSide(uint8_t id, float objectsolid_val, uint8_t j, RB_Vec3f *CircleVtexBottom, RB_Vec3f *CircleVtexTop);
RBSTATIC void CreateCylinderSurface(uint8_t id, float objectsolid_val, RB_Vec3f *CircleVtex, char *str);
RBSTATIC void CreateCylinderStruct(uint8_t id, RB_Vec3f *axis, RB_Vec3f *norm, RB_Vec3f *rel, RB_Vec3f *pos);
RBSTATIC void DrawCylinder(uint32_t id);

RBSTATIC void GenerateSphereDat(void);

RBSTATIC void DrawBox(uint32_t id);
RBSTATIC void DrawObjectSize(uint32_t id, OBJECT_T *Object);
RBSTATIC void DrawObject3d(void);

RBSTATIC void UnsetConfig(void);
RBSTATIC void SetConfig(void);
RBSTATIC void SplotData(void);

RBSTATIC void CoordinateSys_Config(uint32_t id, RBCONST RB_Vec3f *v, float length, RBCONST RB_Mat3f *m);
RBSTATIC void DrawWorldCoordinateSys(void);
RBSTATIC void DrawCoordinateSys(void);


RBSTATIC void ReservationObjectId(void)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);
	uint32_t id = 2u;
	uint32_t add_id = 0u;
	for(uint32_t i = 1u; i < (uint32_t)OBJECT_MAXID; i++)
	{
		uint8_t ShapeType = ObjectData[i].ShapeType;
		//0:Box, 1:Sphere, 2:Capsule, 3:Cylinder
		id += add_id;
		switch(ShapeType)
		{
			case 1u:
				add_id = 0u;
				printf("Sphere:\t");
				break;

			case 2u:
				add_id = 24u;
				printf("Capsule:\t");
				break;

			case 3u:
				add_id = 26u;
				printf("Cylinder:\t");
				break;

			default:
				add_id = 6u;
				printf("Box:\t");
				break;
		}
		f_ObjectStartId[i] = id;
	}
	printf("\n");
	for(uint32_t n = 1u; n < (uint32_t)OBJECT_MAXID; n++)
	{
			printf("Id:%u >> startId: %u\n", n, f_ObjectStartId[n]);
	}
}

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

RBSTATIC void CreateCylinderSide(uint8_t id, float objectsolid_val, uint8_t j, RB_Vec3f *CircleVtexBottom, RB_Vec3f *CircleVtexTop)
{
	uint8_t k = (j == 23u) ? 0u : (j + 1u);

	fprintf(plt_3d, "set style fill transparent solid %f \n", objectsolid_val);
	fprintf(plt_3d, "set obj %u polygon from \
		%.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
		depthorder fillcolor \"steelblue\" \n", \
		id,
		RB_Vec3fGetElem(&CircleVtexBottom[j], 0u),RB_Vec3fGetElem(&CircleVtexBottom[j], 1u),RB_Vec3fGetElem(&CircleVtexBottom[j], 2u), \
		RB_Vec3fGetElem(&CircleVtexBottom[k], 0u),RB_Vec3fGetElem(&CircleVtexBottom[k], 1u),RB_Vec3fGetElem(&CircleVtexBottom[k], 2u), \
		RB_Vec3fGetElem(&CircleVtexTop[k], 0u),RB_Vec3fGetElem(&CircleVtexTop[k], 1u),RB_Vec3fGetElem(&CircleVtexTop[k], 2u), \
		RB_Vec3fGetElem(&CircleVtexTop[j], 0u),RB_Vec3fGetElem(&CircleVtexTop[j], 1u),RB_Vec3fGetElem(&CircleVtexTop[j], 2u), \
		RB_Vec3fGetElem(&CircleVtexBottom[j], 0u),RB_Vec3fGetElem(&CircleVtexBottom[j], 1u),RB_Vec3fGetElem(&CircleVtexBottom[j], 2u) \
	);
}

RBSTATIC void CreateCylinderSurface(uint8_t id, float objectsolid_val, RB_Vec3f *CircleVtex, char *str)
{
	fprintf(plt_3d, "set style fill transparent solid %f \n", objectsolid_val);
	fprintf(plt_3d, "set obj %u polygon from \
		%.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
		to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
		to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
		to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
		to %.3f,%.3f,%.3f \
		depthorder fillcolor \"%s\" \n", \
		id,
		RB_Vec3fGetElem(&CircleVtex[0u], 0u),RB_Vec3fGetElem(&CircleVtex[0u], 1u),RB_Vec3fGetElem(&CircleVtex[0u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[1u], 0u),RB_Vec3fGetElem(&CircleVtex[1u], 1u),RB_Vec3fGetElem(&CircleVtex[1u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[2u], 0u),RB_Vec3fGetElem(&CircleVtex[2u], 1u),RB_Vec3fGetElem(&CircleVtex[2u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[3u], 0u),RB_Vec3fGetElem(&CircleVtex[3u], 1u),RB_Vec3fGetElem(&CircleVtex[3u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[4u], 0u),RB_Vec3fGetElem(&CircleVtex[4u], 1u),RB_Vec3fGetElem(&CircleVtex[4u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[5u], 0u),RB_Vec3fGetElem(&CircleVtex[5u], 1u),RB_Vec3fGetElem(&CircleVtex[5u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[6u], 0u),RB_Vec3fGetElem(&CircleVtex[6u], 1u),RB_Vec3fGetElem(&CircleVtex[6u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[7u], 0u),RB_Vec3fGetElem(&CircleVtex[7u], 1u),RB_Vec3fGetElem(&CircleVtex[7u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[8u], 0u),RB_Vec3fGetElem(&CircleVtex[8u], 1u),RB_Vec3fGetElem(&CircleVtex[8u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[9u], 0u),RB_Vec3fGetElem(&CircleVtex[9u], 1u),RB_Vec3fGetElem(&CircleVtex[9u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[10u], 0u),RB_Vec3fGetElem(&CircleVtex[10u], 1u),RB_Vec3fGetElem(&CircleVtex[10u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[11u], 0u),RB_Vec3fGetElem(&CircleVtex[11u], 1u),RB_Vec3fGetElem(&CircleVtex[11u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[12u], 0u),RB_Vec3fGetElem(&CircleVtex[12u], 1u),RB_Vec3fGetElem(&CircleVtex[12u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[13u], 0u),RB_Vec3fGetElem(&CircleVtex[13u], 1u),RB_Vec3fGetElem(&CircleVtex[13u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[14u], 0u),RB_Vec3fGetElem(&CircleVtex[14u], 1u),RB_Vec3fGetElem(&CircleVtex[14u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[15u], 0u),RB_Vec3fGetElem(&CircleVtex[15u], 1u),RB_Vec3fGetElem(&CircleVtex[15u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[16u], 0u),RB_Vec3fGetElem(&CircleVtex[16u], 1u),RB_Vec3fGetElem(&CircleVtex[16u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[17u], 0u),RB_Vec3fGetElem(&CircleVtex[17u], 1u),RB_Vec3fGetElem(&CircleVtex[17u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[18u], 0u),RB_Vec3fGetElem(&CircleVtex[18u], 1u),RB_Vec3fGetElem(&CircleVtex[18u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[19u], 0u),RB_Vec3fGetElem(&CircleVtex[19u], 1u),RB_Vec3fGetElem(&CircleVtex[19u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[20u], 0u),RB_Vec3fGetElem(&CircleVtex[20u], 1u),RB_Vec3fGetElem(&CircleVtex[20u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[21u], 0u),RB_Vec3fGetElem(&CircleVtex[21u], 1u),RB_Vec3fGetElem(&CircleVtex[21u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[22u], 0u),RB_Vec3fGetElem(&CircleVtex[22u], 1u),RB_Vec3fGetElem(&CircleVtex[22u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[23u], 0u),RB_Vec3fGetElem(&CircleVtex[23u], 1u),RB_Vec3fGetElem(&CircleVtex[23u], 2u), \
		RB_Vec3fGetElem(&CircleVtex[0u], 0u),RB_Vec3fGetElem(&CircleVtex[0u], 1u),RB_Vec3fGetElem(&CircleVtex[0u], 2u), \
		str \
	);
}

RBSTATIC void CreateCylinderStruct(uint8_t id, RB_Vec3f *axis, RB_Vec3f *norm, RB_Vec3f *rel, RB_Vec3f *pos)
{
	float objectsolid_val = 0.5;
	RB_Vec3f CircleVtexTop[24u] = { 0.0f };
	RB_Vec3f CircleVtexBottom[24u] = { 0.0f };
	RB_Vec3f EndCircle;

	RB_Vec3fAdd(axis, rel, &EndCircle);

	for(uint8_t i = 0u; i < 24u; i++)
	{
		RB_Vec3f BottomOffset;
		RB_Vec3f TopOffset;
		RB_VecRotateVec3f(Deg2Rad(15.0f * (float)i), norm, rel, &BottomOffset);
		RB_Vec3fAdd(pos, &BottomOffset, &CircleVtexBottom[i]);
		RB_VecRotateVec3f(Deg2Rad(15.0f * (float)i), norm, &EndCircle, &TopOffset);
		RB_Vec3fAdd(pos, &TopOffset, &CircleVtexTop[i]);
	}

	//円柱の底面・上面
	CreateCylinderSurface(id, objectsolid_val, CircleVtexBottom, "#0918e6");
	id++;
	CreateCylinderSurface(id, objectsolid_val, CircleVtexTop, "steelblue");

	//円柱側面
	for(uint8_t i = 0u; i < 24; i++)
	{
		id++;
		CreateCylinderSide(id, objectsolid_val, i, CircleVtexBottom, CircleVtexTop);
	}
}

RBSTATIC void DrawCylinder(uint32_t id)
{

	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);
	uint32_t object_id = f_ObjectStartId[id];

	float Radius;
	RB_Vec3f EndPos;

	///Cylinderを作成
	RB_Vec3f CenterPos = ObjectData[id].CenterPos;
	RB_Mat3f CenterRot = ObjectData[id].CenterRot;
	SSV_T Cylinder = ObjectData[id].Cylinder;
	Radius = Cylinder.Radius;
	EndPos = Cylinder.EndPos;

	RB_Vec3f Rel, Norm, Vertical, Axis;
	RB_MulMatVec3f(&CenterRot, &EndPos, &Axis);

	RB_CalcVerticalVec3f(&Axis, &Vertical);
	RB_Vec3fCreate(((Radius)*(Vertical.e[0])), ((Radius)*(Vertical.e[1])), ((Radius)*(Vertical.e[2])), &Rel);
	RB_Vec3fNormalize(&Axis, &Norm);

	///シリンダーの描画
	CreateCylinderStruct(object_id, &Axis, &Norm, &Rel, &CenterPos);
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

RBSTATIC void DrawBox(uint32_t id)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	BOX_T Box_obj = ObjectData[id].Box;
	uint32_t object_id = f_ObjectStartId[id];
	RB_Vec3f BoxInitArray[8u] = { 0.0f };
	RB_Vec3f BoxVertex[8u] = { 0.0f };
	RB_Vec3f width3f;

	float lx = RB_Vec3fGetElem(&(Box_obj.BoxSize), 0u);
	float ly = RB_Vec3fGetElem(&(Box_obj.BoxSize), 1u);
	float lz = RB_Vec3fGetElem(&(Box_obj.BoxSize), 2u);

	switch(Box_obj.CenterType)
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
		RB_MulMatVec3f(&ObjectData[id].CenterRot, &BoxInitArray[i], &RotVec);
		RB_Vec3fAdd(&ObjectData[id].CenterPos, &RotVec, &BoxVertex[i]);
	}

	//描画する
	float objectsolid_val = 0.3;

	fprintf(plt_3d, "set style fill transparent solid %f \n", objectsolid_val);
	fprintf(plt_3d, "set obj %u polygon from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
		depthorder fillcolor \"#0918e6\" \n", \
		object_id, \
		RB_Vec3fGetElem(&BoxVertex[0u], 0u),RB_Vec3fGetElem(&BoxVertex[0u], 1u),RB_Vec3fGetElem(&BoxVertex[0u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[1u], 0u),RB_Vec3fGetElem(&BoxVertex[1u], 1u),RB_Vec3fGetElem(&BoxVertex[1u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[2u], 0u),RB_Vec3fGetElem(&BoxVertex[2u], 1u),RB_Vec3fGetElem(&BoxVertex[2u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[3u], 0u),RB_Vec3fGetElem(&BoxVertex[3u], 1u),RB_Vec3fGetElem(&BoxVertex[3u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[0u], 0u),RB_Vec3fGetElem(&BoxVertex[0u], 1u),RB_Vec3fGetElem(&BoxVertex[0u], 2u)  \
	);
	object_id++;

	fprintf(plt_3d, "set obj %u polygon from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
		depthorder fillcolor \"#33AAAA\" \n", \
		object_id, \
		RB_Vec3fGetElem(&BoxVertex[3u], 0u),RB_Vec3fGetElem(&BoxVertex[3u], 1u),RB_Vec3fGetElem(&BoxVertex[3u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[2u], 0u),RB_Vec3fGetElem(&BoxVertex[2u], 1u),RB_Vec3fGetElem(&BoxVertex[2u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[6u], 0u),RB_Vec3fGetElem(&BoxVertex[6u], 1u),RB_Vec3fGetElem(&BoxVertex[6u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[7u], 0u),RB_Vec3fGetElem(&BoxVertex[7u], 1u),RB_Vec3fGetElem(&BoxVertex[7u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[3u], 0u),RB_Vec3fGetElem(&BoxVertex[3u], 1u),RB_Vec3fGetElem(&BoxVertex[3u], 2u)  \
	);
	object_id++;

	fprintf(plt_3d, "set obj %u polygon from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
		depthorder fillcolor \"#33AAAA\" \n", \
		object_id, \
		RB_Vec3fGetElem(&BoxVertex[1u], 0u),RB_Vec3fGetElem(&BoxVertex[1u], 1u),RB_Vec3fGetElem(&BoxVertex[1u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[5u], 0u),RB_Vec3fGetElem(&BoxVertex[5u], 1u),RB_Vec3fGetElem(&BoxVertex[5u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[6u], 0u),RB_Vec3fGetElem(&BoxVertex[6u], 1u),RB_Vec3fGetElem(&BoxVertex[6u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[2u], 0u),RB_Vec3fGetElem(&BoxVertex[2u], 1u),RB_Vec3fGetElem(&BoxVertex[2u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[1u], 0u),RB_Vec3fGetElem(&BoxVertex[1u], 1u),RB_Vec3fGetElem(&BoxVertex[1u], 2u)  \
	);
	object_id++;

	fprintf(plt_3d, "set obj %u polygon from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
		depthorder fillcolor \"#33AAAA\" \n", \
		object_id, \
		RB_Vec3fGetElem(&BoxVertex[4u], 0u),RB_Vec3fGetElem(&BoxVertex[4u], 1u),RB_Vec3fGetElem(&BoxVertex[4u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[0u], 0u),RB_Vec3fGetElem(&BoxVertex[0u], 1u),RB_Vec3fGetElem(&BoxVertex[0u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[3u], 0u),RB_Vec3fGetElem(&BoxVertex[3u], 1u),RB_Vec3fGetElem(&BoxVertex[3u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[7u], 0u),RB_Vec3fGetElem(&BoxVertex[7u], 1u),RB_Vec3fGetElem(&BoxVertex[7u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[4u], 0u),RB_Vec3fGetElem(&BoxVertex[4u], 1u),RB_Vec3fGetElem(&BoxVertex[4u], 2u)  \
	);
	object_id++;

	fprintf(plt_3d, "set style fill transparent solid %f \n", objectsolid_val);
	fprintf(plt_3d, "set obj %u polygon from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
		depthorder fillcolor \"#33AAAA\" \n", \
		object_id, \
		RB_Vec3fGetElem(&BoxVertex[4u], 0u),RB_Vec3fGetElem(&BoxVertex[4u], 1u),RB_Vec3fGetElem(&BoxVertex[4u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[5u], 0u),RB_Vec3fGetElem(&BoxVertex[5u], 1u),RB_Vec3fGetElem(&BoxVertex[5u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[1u], 0u),RB_Vec3fGetElem(&BoxVertex[1u], 1u),RB_Vec3fGetElem(&BoxVertex[1u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[0u], 0u),RB_Vec3fGetElem(&BoxVertex[0u], 1u),RB_Vec3fGetElem(&BoxVertex[0u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[4u], 0u),RB_Vec3fGetElem(&BoxVertex[4u], 1u),RB_Vec3fGetElem(&BoxVertex[4u], 2u)  \
	);
	object_id++;

	fprintf(plt_3d, "set style fill transparent solid %f \n", objectsolid_val);
	fprintf(plt_3d, "set obj %u polygon from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
		depthorder fillcolor \"#33AAAA\" \n", \
		object_id, \
		RB_Vec3fGetElem(&BoxVertex[5u], 0u),RB_Vec3fGetElem(&BoxVertex[5u], 1u),RB_Vec3fGetElem(&BoxVertex[5u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[4u], 0u),RB_Vec3fGetElem(&BoxVertex[4u], 1u),RB_Vec3fGetElem(&BoxVertex[4u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[7u], 0u),RB_Vec3fGetElem(&BoxVertex[7u], 1u),RB_Vec3fGetElem(&BoxVertex[7u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[6u], 0u),RB_Vec3fGetElem(&BoxVertex[6u], 1u),RB_Vec3fGetElem(&BoxVertex[6u], 2u), \
		RB_Vec3fGetElem(&BoxVertex[5u], 0u),RB_Vec3fGetElem(&BoxVertex[5u], 1u),RB_Vec3fGetElem(&BoxVertex[5u], 2u)  \
	);
}

RBSTATIC void DrawObjectSizeArrow(uint32_t id, OBJECT_T *Object)
{
	uint32_t arrow_num = ( 3u * (id + 100u)) -2u;

	RB_Vec3f *v = &Object->CenterPos;
	RB_Mat3f *m = &Object->CenterRot;

	uint8_t ShapeType = Object->ShapeType;

	if(ShapeType == 0u)
	{
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
		);
		arrow_num++;

		fprintf(plt_3d,"set arrow %u from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f front lw 2 lt rgbcolor \'cyan\'  \n",\
		arrow_num, \
		v->e[0],v->e[1],v->e[2],\
	//=====================================
		(v->e[0u] + l->e[1u] * RB_Mat3fGetElem(m, 0u, 1u)),\
		(v->e[1u] + l->e[1u] * RB_Mat3fGetElem(m, 1u, 1u)),\
		(v->e[2u] + l->e[1u] * RB_Mat3fGetElem(m, 2u, 1u))\
		);
		arrow_num++;

		fprintf(plt_3d,"set arrow %u from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f front lw 2 lt rgbcolor \'orange\'  \n",\
		arrow_num, \
		v->e[0],v->e[1],v->e[2],\
	//=====================================
		(v->e[0u] + l->e[2u] * RB_Mat3fGetElem(m, 0u, 2u)),\
		(v->e[1u] + l->e[2u] * RB_Mat3fGetElem(m, 1u, 2u)),\
		(v->e[2u] + l->e[2u] * RB_Mat3fGetElem(m, 2u, 2u))\
		);	
		fprintf(plt_3d,"set colorsequence classic\n");
	}
	else
	{
		SSV_T *SSV_obj;
		switch(ShapeType)
		{
			case 2u:
				SSV_obj = &Object->Capsule;
				break;

			case 3u:
				SSV_obj = &Object->Cylinder;
				break;

			default:
				SSV_obj = &Object->Sphere;
				break;
		}
		RB_Vec3f *EndPos = &SSV_obj->EndPos;
		float Radius = SSV_obj->Radius;

		RB_Vec3f RotVec, ArrowVec, Vertical, RadiusArrow;
		RB_MulMatVec3f(m, EndPos, &RotVec);
		RB_Vec3fAdd(v, &RotVec, &ArrowVec);

		fprintf(plt_3d,"set colorsequence default\n");
		fprintf(plt_3d,"set arrow %u from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f front lw 2 lt rgbcolor \'orange\' \n",\
		arrow_num, \
		v->e[0],v->e[1],v->e[2],\
			(ArrowVec.e[0]),\
			(ArrowVec.e[1]),\
			(ArrowVec.e[2])\
		);//x

		arrow_num++;
		RB_CalcVerticalVec3f(EndPos, &Vertical);
		RB_Vec3fCreate(((Radius)*(Vertical.e[0])), ((Radius)*(Vertical.e[1])), ((Radius)*(Vertical.e[2])), &RadiusArrow);
		RB_MulMatVec3f(m, &RadiusArrow, &RotVec);
		RB_Vec3fAdd(v, &RotVec, &ArrowVec);

		fprintf(plt_3d,"set colorsequence default\n");
		fprintf(plt_3d,"set arrow %u from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f front lw 2 lt rgbcolor \'cyan\' \n",\
		arrow_num, \
		v->e[0],v->e[1],v->e[2],\
	//=====================================
			(ArrowVec.e[0]),\
			(ArrowVec.e[1]),\
			(ArrowVec.e[2])\
		);//x
		arrow_num++;		
		fprintf(plt_3d,"set colorsequence classic\n");
	}
}

RBSTATIC void DrawObject3d(void)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	for(uint32_t i = 1u; i < (uint32_t)OBJECT_MAXID; i++)
	{
		switch((ObjectData[i].ShapeType))
		{
			case 1u:
				break;

			case 2u:
				break;

			case 3u:
				DrawCylinder(i);
				break;

			default:
				DrawBox(i);
				break;
		}
	}

}

RBSTATIC void DrawObjectArrow(void)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	for(uint32_t i = 1u; i < (uint32_t)OBJECT_MAXID; i++)
	{
		DrawObjectSizeArrow(i, &ObjectData[i]);
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

RBSTATIC void SplotData(void)
{
	fprintf(plt_3d," splot -10 \n");
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

//データを作成
	GenerateSphereDat();

//1回の設定で済むものたち
	DrawWorldCoordinateSys();
	ColorConfig();
	UnsetConfig();
	SetConfig();
	DrawGround(0.0f);
	ReservationObjectId();
}

void GnuPlot_Cycle(void)
{
//コマンドで変化するものたち
	DrawCoordinateSys();
	DrawObject3d();
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
