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

//SphereStruct
RBSTATIC void CreateSphereStruct(uint32_t id, float objectsolid_val, RB_Vec3f *CentralAxis, RB_Vec3f *CentralPos);
RBSTATIC void DrawSphere(uint32_t id, float objectsolid_val);

//CylinderStruct build
RBSTATIC void CreateCylinderPolygon(uint32_t id, float objectsolid_val, uint32_t j, RB_Vec3f *CircleVtexBottom, RB_Vec3f *CircleVtexTop);
RBSTATIC void CreateCirclePolygon(uint32_t id, float objectsolid_val, RB_Vec3f *CircleVtex, char *str);
RBSTATIC void CreateCylinderStruct(uint32_t id, float objectsolid_val, float radius, RB_Vec3f *CentralAxis, RB_Vec3f *CentralPos);
RBSTATIC void DrawCylinder(uint32_t id, float objectsolid_val);

//CapsuleStruct build
RBSTATIC void CreateHemiSphereStruct(uint32_t id, float objectsolid_val, RB_Vec3f *CentralAxis, RB_Vec3f *RadiusVec, RB_Vec3f *CentralPos);
RBSTATIC void CreateCapsuleStruct(uint32_t id, float objectsolid_val, float radius, RB_Vec3f *CentralAxis, RB_Vec3f *CentralPos);
RBSTATIC void DrawCapsule(uint32_t id, float objectsolid_val);

//RoundBoxStruct build
RBSTATIC void CreateQuarterSphereStruct(uint32_t id, float objectsolid_val, RB_Vec3f *CentralAxis, RB_Vec3f *RadiusVec, RB_Vec3f *CentralPos);
RBSTATIC void CreateSemiCylinderStruct(uint32_t id, float objectsolid_val, RB_Vec3f *CentralRadius, RB_Vec3f *CentralAxis, RB_Vec3f *CentralPos);
RBSTATIC void CreateSideRectAngle(uint32_t id, float objectsolid_val, RB_Vec3f *RectAngle);
RBSTATIC void CreateRoundBoxStruct(uint32_t id, float objectsolid_val,  RB_Vec3f *CentralRadius, RB_Vec3f *CentralAxis, RB_Vec3f *CentralWidth, RB_Vec3f *CentralPos);
RBSTATIC void DrawRoundBox(uint32_t id, float objectsolid_val);

//BoxStruct build
RBSTATIC void DrawBox(uint32_t id, float objectsolid_val);

//ObjectSizeArrow
RBSTATIC void DrawObjectSizeArrow(uint32_t id, OBJECT_T *Object);
RBSTATIC void DrawObject3d(void);
RBSTATIC void DrawObjectArrow(void);

RBSTATIC void ColorConfig(void);
RBSTATIC void UnsetConfig(void);
RBSTATIC void SetConfig(void);

RBSTATIC void CoordinateSys_Config(uint32_t id, RBCONST RB_Vec3f *v, float length, RBCONST RB_Mat3f *m);
RBSTATIC void DrawWorldCoordinateSys(void);
RBSTATIC void DrawCoordinateSys(void);

RBSTATIC void DevPlotArrow(uint32_t arrow_id, char *color, RB_Vec3f *startpos, RB_Vec3f *endpos )
{
	fprintf(plt_3d,"set colorsequence default\n");
	fprintf(plt_3d,"set arrow %u from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f front lw 2 lt rgbcolor \'%s\' \n",\
		arrow_id, \
		startpos->e[0],startpos->e[1],startpos->e[2],\
			(endpos->e[0]),\
			(endpos->e[1]),\
			(endpos->e[2]),\
			color
		);//x
}

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
				add_id = 288u;
				printf("Sphere:\t");
				break;

			case 2u:
				add_id = 312u;
				printf("Capsule:\t");
				break;

			case 3u:
				add_id = 26u;
				printf("Cylinder:\t");
				break;

			case 4u:
				add_id = 338u;
				printf("RoundBox:\t");
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

//地面をpolygonで作成
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

RBSTATIC void CreateSphereStruct(uint32_t id, float objectsolid_val, RB_Vec3f *CentralAxis, RB_Vec3f *CentralPos)
{
	//中心軸と半径に対する単位ベクトル
	RB_Vec3f radius_uvec, axis_uvec;

	//中心軸の単位ベクトルを作成
	RB_Vec3fNormalize(CentralAxis, &axis_uvec);
	//半径の単位ベクトルを作成
	RB_CalcVerticalVec3f(&axis_uvec, &radius_uvec);

	//球体を構成する頂点の配列を用意
	RB_Vec3f Vertex[13u][24u] = { 0.0f };
	uint32_t object_id = id;

	uint32_t arrow_num = 1000u;

	//球体の頂点を作成する
	//半径軸を回転させる
	for(uint32_t i = 0u; i < 13u; i++)
	{
		RB_Vec3f Rot_Vertex, Rel_Vertex;
		RB_VecRotateVec3f(Deg2Rad(15.0f * (float)i), &radius_uvec, CentralAxis, &Rot_Vertex);

		//中心軸を回転させる
		for(uint32_t j = 0u; j < 24u; j++)
		{
			//Rot_Vertexをaxis_uvec(中心軸)を回転軸として回転させたRel_Vertex(相対位置頂点)を求める
			RB_VecRotateVec3f(Deg2Rad(15.0f * (float)j), &axis_uvec, &Rot_Vertex, &Rel_Vertex);
			
			//球体の中心点を足してworld座標系の位置を格納
			RB_Vec3fAdd(CentralPos, &Rel_Vertex, &Vertex[i][j]);
		}
	}

	for(uint32_t i = 0u; i < 12u; i++)
	{
		for(uint32_t j = 0u; j < 24u; j++)
		{
			uint32_t k = (j == 23u) ? 0u : (j + 1u);
			fprintf(plt_3d, "set style fill transparent solid %f \n", 0.3f);
			fprintf(plt_3d, "set obj %u polygon from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
				depthorder fillcolor \"steelblue\" \n", \
				object_id, \
				RB_Vec3fGetElem(&Vertex[i][j], 0u),RB_Vec3fGetElem(&Vertex[i][j], 1u),RB_Vec3fGetElem(&Vertex[i][j], 2u), \
				RB_Vec3fGetElem(&Vertex[i][k], 0u),RB_Vec3fGetElem(&Vertex[i][k], 1u),RB_Vec3fGetElem(&Vertex[i][k], 2u), \
				RB_Vec3fGetElem(&Vertex[i+1u][k], 0u),RB_Vec3fGetElem(&Vertex[i+1u][k], 1u),RB_Vec3fGetElem(&Vertex[i+1u][k], 2u), \
				RB_Vec3fGetElem(&Vertex[i+1u][j], 0u),RB_Vec3fGetElem(&Vertex[i+1u][j], 1u),RB_Vec3fGetElem(&Vertex[i+1u][j], 2u), \
				RB_Vec3fGetElem(&Vertex[i][j], 0u),RB_Vec3fGetElem(&Vertex[i][j], 1u),RB_Vec3fGetElem(&Vertex[i][j], 2u)  \
			);
			object_id++;
		}
	}
}

RBSTATIC void DrawSphere(uint32_t id, float objectsolid_val)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);
	uint32_t object_id = f_ObjectStartId[id];
	
	///Sphereを作成
	RB_Vec3f CenterPos = ObjectData[id].CenterPos;
	RB_Mat3f CenterRot = ObjectData[id].CenterRot;
	SSV_T Sphere = ObjectData[id].Sphere;
	float Radius = Sphere.Radius;

	RB_Vec3f CentralAxis, RotCentralAxis;
	RB_Vec3fCreate(0.0f, 0.0f, Radius, &CentralAxis);
	RB_MulMatVec3f(&CenterRot, &CentralAxis, &RotCentralAxis);

	CreateSphereStruct(object_id, objectsolid_val, &RotCentralAxis, &CenterPos);
}

//Cylinderの側面を長方形で描画
RBSTATIC void CreateCylinderPolygon(uint32_t id, float objectsolid_val, uint32_t j, RB_Vec3f *CircleVtexBottom, RB_Vec3f *CircleVtexTop)
{
	uint32_t k = (j == 23u) ? 0u : (j + 1u);

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

//Cylinderの底面と上面を円(正24角形)で描画
RBSTATIC void CreateCirclePolygon(uint32_t id, float objectsolid_val, RB_Vec3f *CircleVtex, char *str)
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

RBSTATIC void CreateCylinderStruct(uint32_t id, float objectsolid_val, float radius, RB_Vec3f *CentralAxis, RB_Vec3f *CentralPos)
{
	//主軸に対する単位ベクトル
	RB_Vec3f radius_uvec, axis_uvec;
	RB_Vec3f Bottom_RadiusVec, TopX_Radius;

	RB_Vec3f VertexTop[24u] = { 0.0f };
	RB_Vec3f VertexBottom[24u] = { 0.0f };

	uint32_t object_id = id;

	RB_Vec3fNormalize(CentralAxis, &axis_uvec);
	RB_CalcVerticalVec3f(CentralAxis, &radius_uvec);

	//半径ベクトルを作成
	RB_Vec3fCreate(((radius)*(radius_uvec.e[0])), ((radius)*(radius_uvec.e[1])), ((radius)*(radius_uvec.e[2])), &Bottom_RadiusVec);

	for(uint32_t i = 0u; i < 24u; i++)
	{
		RB_Vec3f BottomVertex, TopVertex;

		//底部の頂点を計算
		RB_VecRotateVec3f(Deg2Rad(15.0f * (float)i), &axis_uvec, &Bottom_RadiusVec, &BottomVertex);

		//上部の頂点を計算
		RB_Vec3fAdd(CentralAxis, &BottomVertex, &TopVertex);

		//上部, 底部の位置をworld座標系に反映し格納
		RB_Vec3fAdd(CentralPos, &BottomVertex, &VertexBottom[i]);
		RB_Vec3fAdd(CentralPos, &TopVertex, &VertexTop[i]);
	}

	//円柱の底面・上面
	CreateCirclePolygon(object_id, objectsolid_val, VertexBottom, "#0918e6");
	object_id++;
	CreateCirclePolygon(object_id, objectsolid_val, VertexTop, "green");
	object_id++;

	//円柱側面
	for(uint32_t i = 0u; i < 24; i++)
	{
		CreateCylinderPolygon(object_id, objectsolid_val, i, VertexBottom, VertexTop);
		object_id++;
	}
}

RBSTATIC void DrawCylinder(uint32_t id, float objectsolid_val)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);
	uint32_t object_id = f_ObjectStartId[id];

	///Cylinderを作成
	RB_Vec3f CenterPos = ObjectData[id].CenterPos;
	RB_Mat3f CenterRot = ObjectData[id].CenterRot;
	SSV_T Cylinder = ObjectData[id].Cylinder;
	float Radius = Cylinder.Radius;
	RB_Vec3f EndPos = Cylinder.EndPos;
	RB_Vec3f CentralAxis;

	//姿勢を中心軸に反映
	RB_MulMatVec3f(&CenterRot, &EndPos, &CentralAxis);
	CreateCylinderStruct(object_id, objectsolid_val, Radius, &CentralAxis, &CenterPos);
}

RBSTATIC void CreateHemiSphereStruct(uint32_t id, float objectsolid_val, RB_Vec3f *CentralAxis, RB_Vec3f *RadiusVec, RB_Vec3f *CentralPos)
{
	RB_Vec3f axis_uvec;
	uint32_t object_id = id;

	RB_Vec3f CapsuleBtmVtex[13u][24u] = { 0.0f };
	RB_Vec3f CapsuleTopVtex[13u][24u] = { 0.0f };

	RB_Vec3f Rot_Vertex;

	RB_Vec3fNormalize(CentralAxis, &axis_uvec);

	//半径のベクトル(オフセット)を主軸を中心に-90deg回転
	RB_VecRotateVec3f(Deg2Rad(-90.0f), &axis_uvec, RadiusVec, &Rot_Vertex);

	//Rot_Vertexを作成
	RB_Vec3f  Rot_uvec;
	RB_Vec3fNormalize(&Rot_Vertex, &Rot_uvec);

	for(uint32_t i = 0u; i < 7u; i++)
	{
		RB_Vec3f HemiBtmVertex, HemiTopVertex;
		RB_VecRotateVec3f(Deg2Rad(-15.0f * (float)i), &Rot_uvec, RadiusVec, &HemiBtmVertex);
		RB_VecRotateVec3f(Deg2Rad( 15.0f * (float)i), &Rot_uvec, RadiusVec, &HemiTopVertex);

		for(uint32_t j = 0u; j < 24u; j++)
		{
			RB_Vec3f BottomVertex, TopVertex;

			//底部の描画
			RB_VecRotateVec3f(Deg2Rad(15.0f * (float)j), &axis_uvec, &HemiBtmVertex, &BottomVertex);

			//上部の描画
			RB_Vec3f HemiTopPos;
			RB_Vec3fAdd(CentralPos, CentralAxis, &HemiTopPos);
			RB_VecRotateVec3f(Deg2Rad(15.0f * (float)j), &axis_uvec, &HemiTopVertex, &TopVertex);

			RB_Vec3fAdd(CentralPos, &BottomVertex, &CapsuleBtmVtex[i][j]);
			RB_Vec3fAdd(&HemiTopPos, &TopVertex, &CapsuleTopVtex[i][j]);
		}
	}

	for(uint32_t i = 0u; i < 6u; i++)
	{
		for(uint32_t j = 0u; j < 24u; j++)
		{

			uint32_t k = (j == 23u) ? 0u : (j + 1u);
			fprintf(plt_3d, "set style fill transparent solid %f \n", objectsolid_val);
			fprintf(plt_3d, "set obj %u polygon from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
				depthorder fillcolor \"steelblue\" \n", \
				object_id, \
				RB_Vec3fGetElem(&CapsuleBtmVtex[i][j], 0u),RB_Vec3fGetElem(&CapsuleBtmVtex[i][j], 1u),RB_Vec3fGetElem(&CapsuleBtmVtex[i][j], 2u), \
				RB_Vec3fGetElem(&CapsuleBtmVtex[i][k], 0u),RB_Vec3fGetElem(&CapsuleBtmVtex[i][k], 1u),RB_Vec3fGetElem(&CapsuleBtmVtex[i][k], 2u), \
				RB_Vec3fGetElem(&CapsuleBtmVtex[i+1u][k], 0u),RB_Vec3fGetElem(&CapsuleBtmVtex[i+1u][k], 1u),RB_Vec3fGetElem(&CapsuleBtmVtex[i+1u][k], 2u), \
				RB_Vec3fGetElem(&CapsuleBtmVtex[i+1u][j], 0u),RB_Vec3fGetElem(&CapsuleBtmVtex[i+1u][j], 1u),RB_Vec3fGetElem(&CapsuleBtmVtex[i+1u][j], 2u), \
				RB_Vec3fGetElem(&CapsuleBtmVtex[i][j], 0u),RB_Vec3fGetElem(&CapsuleBtmVtex[i][j], 1u),RB_Vec3fGetElem(&CapsuleBtmVtex[i][j], 2u)  \
			);
			object_id++;

			fprintf(plt_3d, "set style fill transparent solid %f \n", objectsolid_val);
			fprintf(plt_3d, "set obj %u polygon from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
				depthorder fillcolor \"steelblue\" \n", \
				object_id, \
				RB_Vec3fGetElem(&CapsuleTopVtex[i][j], 0u),RB_Vec3fGetElem(&CapsuleTopVtex[i][j], 1u),RB_Vec3fGetElem(&CapsuleTopVtex[i][j], 2u), \
				RB_Vec3fGetElem(&CapsuleTopVtex[i][k], 0u),RB_Vec3fGetElem(&CapsuleTopVtex[i][k], 1u),RB_Vec3fGetElem(&CapsuleTopVtex[i][k], 2u), \
				RB_Vec3fGetElem(&CapsuleTopVtex[i+1u][k], 0u),RB_Vec3fGetElem(&CapsuleTopVtex[i+1u][k], 1u),RB_Vec3fGetElem(&CapsuleTopVtex[i+1u][k], 2u), \
				RB_Vec3fGetElem(&CapsuleTopVtex[i+1u][j], 0u),RB_Vec3fGetElem(&CapsuleTopVtex[i+1u][j], 1u),RB_Vec3fGetElem(&CapsuleTopVtex[i+1u][j], 2u), \
				RB_Vec3fGetElem(&CapsuleTopVtex[i][j], 0u),RB_Vec3fGetElem(&CapsuleTopVtex[i][j], 1u),RB_Vec3fGetElem(&CapsuleTopVtex[i][j], 2u)  \
			);
			object_id++;
		}
	}
}

RBSTATIC void CreateCapsuleStruct(uint32_t id, float objectsolid_val, float radius, RB_Vec3f *CentralAxis, RB_Vec3f *CentralPos)
{
	RB_Vec3f radius_uvec, axis_uvec;
	RB_Vec3f Axis, Bottom_RadiusVec, Z_Norm, RadiusNorm;

	RB_Vec3f VertexTop[24u] = { 0.0f };
	RB_Vec3f VertexBottom[24u] = { 0.0f };

	uint32_t object_id = id;

	//オブジェクトの軸の単位ベクトルを作成
	RB_Vec3fNormalize(CentralAxis, &axis_uvec);
	RB_CalcVerticalVec3f(CentralAxis, &radius_uvec);

//==========オブジェクトの半径ベクトルを作成
	RB_Vec3fCreate(((radius)*(radius_uvec.e[0])), ((radius)*(radius_uvec.e[1])), ((radius)*(radius_uvec.e[2])), &Bottom_RadiusVec);


	//カプセル円柱側面の描画===================================================================================
		for(uint32_t i = 0u; i < 24u; i++)
		{
			RB_Vec3f BottomVertex, TopVertex;

		//Bottom
			//底部の頂点を計算
			RB_VecRotateVec3f(Deg2Rad(15.0f * (float)i), &axis_uvec, &Bottom_RadiusVec, &BottomVertex);

			//上部の頂点を計算
			RB_Vec3fAdd(CentralAxis, &BottomVertex, &TopVertex);

			//上部、底部の位置をworld座標系に反映し格納
			RB_Vec3fAdd(CentralPos, &BottomVertex, &VertexBottom[i]);
			RB_Vec3fAdd(CentralPos, &TopVertex, &VertexTop[i]);
		}

	for(uint32_t i = 0u; i < 24; i++)
	{
		CreateCylinderPolygon(object_id, objectsolid_val, i, VertexBottom, VertexTop);
		object_id++;
	}

	//カプセル上部,底部の半球を描画
	CreateHemiSphereStruct(object_id, objectsolid_val, CentralAxis, &Bottom_RadiusVec, CentralPos);
}

RBSTATIC void DrawCapsule(uint32_t id, float objectsolid_val)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);
	uint32_t object_id = f_ObjectStartId[id];
	
	///Capsuleを作成
	RB_Vec3f CenterPos = ObjectData[id].CenterPos;
	RB_Mat3f CenterRot = ObjectData[id].CenterRot;
	SSV_T Capsule = ObjectData[id].Capsule;
	float Radius = Capsule.Radius;
	RB_Vec3f EndPos = Capsule.EndPos;
	RB_Vec3f CentralAxis;

	//姿勢を中心軸に反映
	RB_MulMatVec3f(&CenterRot, &EndPos, &CentralAxis);
	CreateCapsuleStruct(object_id, objectsolid_val, Radius, &CentralAxis, &CenterPos);
}

//ひし形四隅のQuarter球体を作成
RBSTATIC void CreateQuarterSphereStruct(uint32_t id, float objectsolid_val, RB_Vec3f *CentralAxis, RB_Vec3f *RadiusVec, RB_Vec3f *CentralPos)
{
	RB_Vec3f axis_uvec;
	uint32_t object_id = id;

	RB_Vec3f CapsuleBtmVtex[7u][13u] = { 0.0f };
	RB_Vec3f CapsuleTopVtex[7u][13u] = { 0.0f };

	RB_Vec3f Rot_Vertex;

	RB_Vec3fNormalize(CentralAxis, &axis_uvec);

	//半径のベクトル(オフセット)を主軸を中心に-90deg回転
	RB_VecRotateVec3f(Deg2Rad(-90.0f), &axis_uvec, RadiusVec, &Rot_Vertex);

	//Rot_Vertexを作成
	RB_Vec3f  Rot_uvec;
	RB_Vec3fNormalize(&Rot_Vertex, &Rot_uvec);

	for(uint32_t i = 0u; i < 7u; i++)
	{
		RB_Vec3f HemiBtmVertex, HemiTopVertex;
		RB_VecRotateVec3f(Deg2Rad(-15.0f * (float)i), &Rot_uvec, RadiusVec, &HemiBtmVertex);
		RB_VecRotateVec3f(Deg2Rad( 15.0f * (float)i), &Rot_uvec, RadiusVec, &HemiTopVertex);

		for(uint32_t j = 0u; j < 13u; j++)
		{
			RB_Vec3f BottomVertex, TopVertex;

			//底部の描画
			RB_VecRotateVec3f(Deg2Rad(15.0f * (float)j), &axis_uvec, &HemiBtmVertex, &BottomVertex);

			//上部の描画
			RB_Vec3f HemiTopPos;
			RB_Vec3fAdd(CentralPos, CentralAxis, &HemiTopPos);
			RB_VecRotateVec3f(Deg2Rad(15.0f * (float)j), &axis_uvec, &HemiTopVertex, &TopVertex);

			RB_Vec3fAdd(CentralPos, &BottomVertex, &CapsuleBtmVtex[i][j]);
			RB_Vec3fAdd(&HemiTopPos, &TopVertex, &CapsuleTopVtex[i][j]);
		}
	}

	for(uint32_t i = 0u; i < 6u; i++)
	{
		for(uint32_t j = 0u; j < 12u; j++)
		{

			uint32_t k = j + 1u;
			fprintf(plt_3d, "set style fill transparent solid %f \n", objectsolid_val);
			fprintf(plt_3d, "set obj %u polygon from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
				depthorder fillcolor \"steelblue\" \n", \
				object_id, \
				RB_Vec3fGetElem(&CapsuleBtmVtex[i][j], 0u),RB_Vec3fGetElem(&CapsuleBtmVtex[i][j], 1u),RB_Vec3fGetElem(&CapsuleBtmVtex[i][j], 2u), \
				RB_Vec3fGetElem(&CapsuleBtmVtex[i][k], 0u),RB_Vec3fGetElem(&CapsuleBtmVtex[i][k], 1u),RB_Vec3fGetElem(&CapsuleBtmVtex[i][k], 2u), \
				RB_Vec3fGetElem(&CapsuleBtmVtex[i+1u][k], 0u),RB_Vec3fGetElem(&CapsuleBtmVtex[i+1u][k], 1u),RB_Vec3fGetElem(&CapsuleBtmVtex[i+1u][k], 2u), \
				RB_Vec3fGetElem(&CapsuleBtmVtex[i+1u][j], 0u),RB_Vec3fGetElem(&CapsuleBtmVtex[i+1u][j], 1u),RB_Vec3fGetElem(&CapsuleBtmVtex[i+1u][j], 2u), \
				RB_Vec3fGetElem(&CapsuleBtmVtex[i][j], 0u),RB_Vec3fGetElem(&CapsuleBtmVtex[i][j], 1u),RB_Vec3fGetElem(&CapsuleBtmVtex[i][j], 2u)  \
			);
			object_id++;

			fprintf(plt_3d, "set style fill transparent solid %f \n", objectsolid_val);
			fprintf(plt_3d, "set obj %u polygon from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
				depthorder fillcolor \"steelblue\" \n", \
				object_id, \
				RB_Vec3fGetElem(&CapsuleTopVtex[i][j], 0u),RB_Vec3fGetElem(&CapsuleTopVtex[i][j], 1u),RB_Vec3fGetElem(&CapsuleTopVtex[i][j], 2u), \
				RB_Vec3fGetElem(&CapsuleTopVtex[i][k], 0u),RB_Vec3fGetElem(&CapsuleTopVtex[i][k], 1u),RB_Vec3fGetElem(&CapsuleTopVtex[i][k], 2u), \
				RB_Vec3fGetElem(&CapsuleTopVtex[i+1u][k], 0u),RB_Vec3fGetElem(&CapsuleTopVtex[i+1u][k], 1u),RB_Vec3fGetElem(&CapsuleTopVtex[i+1u][k], 2u), \
				RB_Vec3fGetElem(&CapsuleTopVtex[i+1u][j], 0u),RB_Vec3fGetElem(&CapsuleTopVtex[i+1u][j], 1u),RB_Vec3fGetElem(&CapsuleTopVtex[i+1u][j], 2u), \
				RB_Vec3fGetElem(&CapsuleTopVtex[i][j], 0u),RB_Vec3fGetElem(&CapsuleTopVtex[i][j], 1u),RB_Vec3fGetElem(&CapsuleTopVtex[i][j], 2u)  \
			);
			object_id++;
		}
	}
}
//ひし形四辺の半円柱を作成
RBSTATIC void CreateSemiCylinderStruct(uint32_t id, float objectsolid_val, RB_Vec3f *CentralRadius, RB_Vec3f *CentralAxis, RB_Vec3f *CentralPos)
{
	//主軸に対する単位ベクトル
	RB_Vec3f radius_uvec, axis_uvec;
	RB_Vec3f TopX_Radius;

	RB_Vec3f VertexTop[24u] = { 0.0f };
	RB_Vec3f VertexBottom[24u] = { 0.0f };

	uint32_t object_id = id;

	RB_Vec3fNormalize(CentralAxis, &axis_uvec);

	for(uint32_t i = 0u; i < 24u; i++)
	{
		RB_Vec3f BottomVertex, TopVertex;

		//底部の頂点を計算
		RB_VecRotateVec3f(Deg2Rad(15.0f * (float)i), &axis_uvec, CentralRadius, &BottomVertex);

		//上部の頂点を計算
		RB_Vec3fAdd(CentralAxis, &BottomVertex, &TopVertex);

		//上部, 底部の位置をworld座標系に反映し格納
		RB_Vec3fAdd(CentralPos, &BottomVertex, &VertexBottom[i]);
		RB_Vec3fAdd(CentralPos, &TopVertex, &VertexTop[i]);
	}

	//円柱側面
	for(uint32_t i = 0u; i < 12; i++)
	{
		CreateCylinderPolygon(object_id, objectsolid_val, i, VertexBottom, VertexTop);
		object_id++;
	}
}

//ひし形の平面を作成
RBSTATIC void CreateSideRectAngle(uint32_t id, float objectsolid_val, RB_Vec3f *RectAngle)
{
	uint32_t object_id = id;
	fprintf(plt_3d, "set style fill transparent solid %f \n", objectsolid_val);
	fprintf(plt_3d, "set obj %u polygon from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f to %.3f,%.3f,%.3f \
		depthorder fillcolor \"steelblue\" \n", \
			object_id, \
			RB_Vec3fGetElem(&RectAngle[0u], 0u),RB_Vec3fGetElem(&RectAngle[0u], 1u),RB_Vec3fGetElem(&RectAngle[0u], 2u), \
			RB_Vec3fGetElem(&RectAngle[1u], 0u),RB_Vec3fGetElem(&RectAngle[1u], 1u),RB_Vec3fGetElem(&RectAngle[1u], 2u), \
			RB_Vec3fGetElem(&RectAngle[2u], 0u),RB_Vec3fGetElem(&RectAngle[2u], 1u),RB_Vec3fGetElem(&RectAngle[2u], 2u), \
			RB_Vec3fGetElem(&RectAngle[3u], 0u),RB_Vec3fGetElem(&RectAngle[3u], 1u),RB_Vec3fGetElem(&RectAngle[3u], 2u), \
			RB_Vec3fGetElem(&RectAngle[0u], 0u),RB_Vec3fGetElem(&RectAngle[0u], 1u),RB_Vec3fGetElem(&RectAngle[0u], 2u)  \
		);
}

//ひし形本体の描画
RBSTATIC void CreateRoundBoxStruct(uint32_t id, float objectsolid_val,  RB_Vec3f *CentralRadius, RB_Vec3f *CentralAxis, RB_Vec3f *CentralWidth, RB_Vec3f *CentralPos)
{
	uint32_t object_id = id;

	RB_Vec3f EndPosAxis, EndPosWidth, EndPosRadius;

//===============================================================
//CentralPos, EndPosAxis, EndPosDiagonal, EndPosWidth

	RB_Vec3f EndPosDiagonal, Diagonal;
	RB_Vec3fAdd(CentralAxis, CentralWidth, &Diagonal);
	RB_Vec3fAdd(CentralPos, &Diagonal, &EndPosDiagonal);

	RB_Vec3fAdd(CentralPos, CentralAxis, &EndPosAxis);

	RB_Vec3fAdd(CentralPos, CentralRadius, &EndPosRadius);

	RB_Vec3fAdd(CentralPos, CentralWidth, &EndPosWidth);

	RB_Vec3f RectAngle_B[4u];
	RB_Vec3f RectAngle_L[4u];
	RB_Vec3f RectAngle_R[4u];

	RectAngle_B[0u] = *CentralPos;
	RectAngle_B[1u] = EndPosAxis;
	RectAngle_B[2u] = EndPosDiagonal;
	RectAngle_B[3u] = EndPosWidth;

	RB_Vec3f Minus_CentralRadius;
	RB_Vec3fSub(&f_RB_Vec3fzero, CentralRadius, &Minus_CentralRadius);

	for(uint32_t i = 0u; i < 4u; i++)
	{
		RB_Vec3fAdd(&RectAngle_B[i], CentralRadius, &RectAngle_L[i]);
		RB_Vec3fAdd(&RectAngle_B[i], &Minus_CentralRadius, &RectAngle_R[i]);
	}


	CreateSideRectAngle(object_id, objectsolid_val, RectAngle_L);
	object_id++;
	CreateSideRectAngle(object_id, objectsolid_val, RectAngle_R);
	object_id++;

	CreateQuarterSphereStruct(object_id, objectsolid_val, CentralWidth, CentralRadius, CentralPos);
	object_id+= 144u;
	CreateSemiCylinderStruct(object_id, objectsolid_val, CentralRadius, CentralWidth, CentralPos);
	object_id+= 12u;
	CreateSemiCylinderStruct(object_id, objectsolid_val, CentralRadius, CentralAxis, &EndPosWidth);
	object_id+= 12u;

	RB_Vec3f u_width, RotRadius;
	RB_Vec3fNormalize(CentralWidth, &u_width);
	RB_VecRotateVec3f(Deg2Rad(180.0f), &u_width, CentralRadius, &RotRadius);

	CreateQuarterSphereStruct(object_id, objectsolid_val, CentralWidth, &RotRadius, &EndPosAxis);
	object_id+= 144u;
	CreateSemiCylinderStruct(object_id, objectsolid_val, &RotRadius, CentralWidth, &EndPosAxis);
	object_id+= 12u;
	CreateSemiCylinderStruct(object_id, objectsolid_val, &RotRadius, CentralAxis, CentralPos);
	object_id+= 12u;
}

RBSTATIC void DrawRoundBox(uint32_t id, float objectsolid_val)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	SSV_T RoundBox_obj = ObjectData[id].RoundBox;
	uint32_t object_id = f_ObjectStartId[id];

	RB_Vec3f CenterPos = ObjectData[id].CenterPos;
	RB_Mat3f CenterRot = ObjectData[id].CenterRot;

	SSV_T RoundBox = ObjectData[id].RoundBox;
	float Radius = RoundBox.Radius;
	RB_Vec3f EndPos = RoundBox.EndPos;
	RB_Vec3f WidthPos = RoundBox.WidthPos;

	RB_Vec3f CentralAxis, CentralWidth, CentralRadius;

	//姿勢を中心軸に反映
	RB_MulMatVec3f(&CenterRot, &EndPos, &CentralAxis);
	RB_MulMatVec3f(&CenterRot, &WidthPos, &CentralWidth);


	RB_Vec3f RadiusVertical, RadiusArrow, u_axis, u_radius;
	RB_Vec3fNormalize(&EndPos, &u_axis);
	RB_CalcVerticalVec3f(&EndPos, &u_radius);

	RB_Vec3fCreate(((Radius)*(u_radius.e[0])), ((Radius)*(u_radius.e[1])), ((Radius)*(u_radius.e[2])), &RadiusArrow);

	//半径のベクトル(オフセット)を主軸を中心に-180deg回転
	
	RB_VecRotateVec3f(Deg2Rad(-180.0f), &u_axis, &RadiusArrow, &RadiusVertical);

	RB_MulMatVec3f(&CenterRot, &RadiusVertical, &CentralRadius);

	CreateRoundBoxStruct(object_id, objectsolid_val, &CentralRadius, &CentralAxis, &CentralWidth, &CenterPos);
}

RBSTATIC void DrawBox(uint32_t id, float objectsolid_val)
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
	for(uint32_t i = 0u; i < 8u; i++)
	{
		RB_Vec3f RotVec;
		RB_MulMatVec3f(&ObjectData[id].CenterRot, &BoxInitArray[i], &RotVec);
		RB_Vec3fAdd(&ObjectData[id].CenterPos, &RotVec, &BoxVertex[i]);
	}

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
		RB_Vec3f EndPos;
		RB_Vec3f WidthPos;
		switch(ShapeType)
		{
			case 2u:
				SSV_obj = &Object->Capsule;
				EndPos = SSV_obj->EndPos;
				break;

			case 3u:
				SSV_obj = &Object->Cylinder;
				EndPos = SSV_obj->EndPos;
				break;

			case 4u:
				SSV_obj = &Object->RoundBox;
				EndPos = SSV_obj->EndPos;
				WidthPos = SSV_obj->WidthPos;
				break;

			default:
				SSV_obj = &Object->Sphere;
				float Sphere_Radius = SSV_obj->Radius;
				RB_Vec3fCreate(0.0f, 0.0f, Sphere_Radius, &EndPos);
				break;
		}

		float Radius = SSV_obj->Radius;

		RB_Vec3f Axis, EndPosAxis;
		RB_MulMatVec3f(m, &EndPos, &Axis);
		RB_Vec3fAdd(v, &Axis, &EndPosAxis);

		fprintf(plt_3d,"set colorsequence default\n");
		fprintf(plt_3d,"set arrow %u from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f front lw 2 lt rgbcolor \'orange\' \n",\
		arrow_num, \
		v->e[0],v->e[1],v->e[2],\
			(EndPosAxis.e[0]),\
			(EndPosAxis.e[1]),\
			(EndPosAxis.e[2])\
		);//x

		arrow_num++;

		RB_Vec3f RadiusArrow, RadiusVec, RadiusVertical, EndPosRadius, u_axis, u_radius;
		RB_Vec3fNormalize(&EndPos, &u_axis);
		RB_CalcVerticalVec3f(&EndPos, &u_radius);

		RB_Vec3fCreate(((Radius)*(u_radius.e[0])), ((Radius)*(u_radius.e[1])), ((Radius)*(u_radius.e[2])), &RadiusArrow);

		//半径のベクトル(オフセット)を主軸を中心に-180deg回転
		RB_VecRotateVec3f(Deg2Rad(-180.0f), &u_axis, &RadiusArrow, &RadiusVertical);

		RB_MulMatVec3f(m, &RadiusVertical, &RadiusVec);
		RB_Vec3fAdd(v, &RadiusVec, &EndPosRadius);

		fprintf(plt_3d,"set colorsequence default\n");
		fprintf(plt_3d,"set arrow %u from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f front lw 2 lt rgbcolor \'cyan\' \n",\
		arrow_num, \
		v->e[0],v->e[1],v->e[2],\
	//=====================================
			(EndPosRadius.e[0]),\
			(EndPosRadius.e[1]),\
			(EndPosRadius.e[2])\
		);//x
		arrow_num++;

		if(ShapeType == 4u)
		{
			RB_Vec3f Width, EndPosWidth;
			RB_MulMatVec3f(m, &WidthPos, &Width);
			RB_Vec3fAdd(v, &Width, &EndPosWidth);

			fprintf(plt_3d,"set colorsequence default\n");
			fprintf(plt_3d,"set arrow %u from %.3f,%.3f,%.3f to %.3f,%.3f,%.3f front lw 2 lt rgbcolor \'spring-green\' \n",\
			arrow_num, \
			v->e[0],v->e[1],v->e[2],\
		//=====================================
				(EndPosWidth.e[0]),\
				(EndPosWidth.e[1]),\
				(EndPosWidth.e[2])\
			);//x
		}
		arrow_num++;

		fprintf(plt_3d,"set colorsequence classic\n");
	}
}

RBSTATIC void DrawObject3d(void)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);
	float objectsolid_val = 0.2f;

	for(uint32_t id = 1u; id < (uint32_t)OBJECT_MAXID; id++)
	{
		switch((ObjectData[id].ShapeType))
		{
			case 1u:
				DrawSphere(id, objectsolid_val);
				break;

			case 2u:
				DrawCapsule(id, objectsolid_val);
				break;

			case 3u:
				DrawCylinder(id, objectsolid_val);
				break;

			case 4u:
				DrawRoundBox(id, objectsolid_val);
				break;

			default:
				DrawBox(id, objectsolid_val);
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

RBSTATIC void DrawWorldCoordinateSys(void)
{
	float axis_length = 100.0f;
	CoordinateSys_Config(0u, &f_RB_Vec3fzero, axis_length, &f_RB_Mat3fident);
}

RBSTATIC void DrawCoordinateSys(void)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	for(uint32_t id = 1u; id < (uint32_t)OBJECT_MAXID; id++)
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

RBSTATIC void SplotData(void)
{
	fprintf(plt_3d," splot -1 \n");
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
