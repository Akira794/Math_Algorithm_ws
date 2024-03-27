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

//Next: SphereStruct build
RBSTATIC void DrawSphere(uint32_t id);

//CylinderStruct build
RBSTATIC void CreateCylinderSide(uint32_t id, float objectsolid_val, uint32_t j, RB_Vec3f *CircleVtexBottom, RB_Vec3f *CircleVtexTop);
RBSTATIC void CreateCylinderSurface(uint32_t id, float objectsolid_val, RB_Vec3f *CircleVtex, char *str);
RBSTATIC void CreateCylinderStruct(uint32_t id, float objectsolid_val, float radius, RB_Vec3f *CentralAxis, RB_Vec3f *CentralPos);
RBSTATIC void DrawCylinder(uint32_t id);

RBSTATIC void GenerateSphereDat(void);

//BoxStruct build
RBSTATIC void DrawBox(uint32_t id);

RBSTATIC void DrawObjectSize(uint32_t id, OBJECT_T *Object);
RBSTATIC void DrawObject3d(void);

RBSTATIC void UnsetConfig(void);
RBSTATIC void SetConfig(void);
RBSTATIC void SplotData(void);

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
	//主軸に対する単位ベクトル
	RB_Vec3f elx, elz;

	//中心軸(z軸)の単位ベクトルを作成
	RB_Vec3fNormalize(CentralAxis, &elz);
	//半径軸(x軸)の単位ベクトルを作成
	RB_CalcVerticalVec3f(&elz, &elx);

	//球体を構成する頂点の配列を用意
	RB_Vec3f Vertex[13u][24u] = { 0.0f };
	uint32_t object_id = id;

	uint32_t arrow_num = 1000u;

	//球体の頂点を作成する
	//半径軸を回転させる
	for(uint32_t i = 0u; i < 13u; i++)
	{
		RB_Vec3f Y_Vertex, Rel_Vertex;
		RB_VecRotateVec3f(Deg2Rad(15.0f * (float)i), &elx, CentralAxis, &Y_Vertex);

		//中心軸を回転させる
		for(uint32_t j = 0u; j < 24u; j++)
		{
			//Y_Vertexをelz(中心軸)を回転軸として回転させたRel_Vertex(相対位置頂点)を求める
			RB_VecRotateVec3f(Deg2Rad(15.0f * (float)j), &elz, &Y_Vertex, &Rel_Vertex);
			
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

RBSTATIC void DrawSphere(uint32_t id)
{
	float objectsolid_val = 0.3f;

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
RBSTATIC void CreateCylinderSide(uint32_t id, float objectsolid_val, uint32_t j, RB_Vec3f *CircleVtexBottom, RB_Vec3f *CircleVtexTop)
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
RBSTATIC void CreateCylinderSurface(uint32_t id, float objectsolid_val, RB_Vec3f *CircleVtex, char *str)
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
	RB_Vec3f elx, elz;
	RB_Vec3f BottomX_Radius, TopX_Radius;

	RB_Vec3f VertexTop[24u] = { 0.0f };
	RB_Vec3f VertexBottom[24u] = { 0.0f };

	uint32_t object_id = id;

	RB_Vec3fNormalize(CentralAxis, &elz);
	RB_CalcVerticalVec3f(CentralAxis, &elx);

	//半径ベクトルを作成
	RB_Vec3fCreate(((radius)*(elx.e[0])), ((radius)*(elx.e[1])), ((radius)*(elx.e[2])), &BottomX_Radius);

	for(uint32_t i = 0u; i < 24u; i++)
	{
		RB_Vec3f BottomVertex, TopVertex;

		//底部の頂点を計算
		RB_VecRotateVec3f(Deg2Rad(15.0f * (float)i), &elz, &BottomX_Radius, &BottomVertex);

		//上部の頂点を計算
		RB_Vec3fAdd(CentralAxis, &BottomVertex, &TopVertex);

		//上部, 底部の位置をworld座標系に反映し格納
		RB_Vec3fAdd(CentralPos, &BottomVertex, &VertexBottom[i]);
		RB_Vec3fAdd(CentralPos, &TopVertex, &VertexTop[i]);
	}

	//円柱の底面・上面
	CreateCylinderSurface(object_id, objectsolid_val, VertexBottom, "#0918e6");
	object_id++;
	CreateCylinderSurface(object_id, objectsolid_val, VertexTop, "green");
	object_id++;

	//円柱側面
	for(uint32_t i = 0u; i < 24; i++)
	{
		CreateCylinderSide(object_id, objectsolid_val, i, VertexBottom, VertexTop);
		object_id++;
	}
}

RBSTATIC void DrawCylinder(uint32_t id)
{
	float objectsolid_val = 0.3f;
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

RBSTATIC void CreateCapsuleStruct(uint32_t id, float objectsolid_val, float radius, RB_Vec3f *CentralAxis, RB_Vec3f *CentralPos)
{

	RB_Vec3f Axis, TopRel, BottomRel, Z_Norm, X_Norm, RadiusNorm;

	RB_Vec3f CircleVtexTop[24u] = { 0.0f };
	RB_Vec3f CircleVtexBottom[24u] = { 0.0f };

	uint32_t object_id = id;

	uint32_t arrow_num = 1000u;

	RB_Vec3f ObjAxis;

	//位置のオフセットを反映
	RB_Vec3fAdd(CentralPos, CentralAxis, &ObjAxis);
	//DevPlotArrow(arrow_num, "red", pos, &ObjAxis );
	arrow_num++;

	//オブジェクトの軸の単位ベクトルを作成
	RB_Vec3fNormalize(CentralAxis, &Z_Norm);
	RB_CalcVerticalVec3f(&Z_Norm, &X_Norm);

//==========オブジェクトの半径ベクトルを作成
	RB_Vec3f ObjRadius;
	RB_CalcVerticalVec3f(CentralAxis, &RadiusNorm);
	RB_Vec3fCreate(((radius)*(RadiusNorm.e[0])), ((radius)*(RadiusNorm.e[1])), ((radius)*(RadiusNorm.e[2])), &BottomRel);

	//位置のオフセットを反映
	RB_Vec3fAdd(CentralPos, &BottomRel, &ObjRadius);

	//オブジェクトの半径を描画
	//DevPlotArrow(arrow_num, "blue", pos, &ObjRadius );
	arrow_num++;
//=========================
	RB_Vec3f devRel, devPos;

	//半径のベクトル(オフセット)を主軸を中心に-90deg回転
	RB_VecRotateVec3f(Deg2Rad(-90.0f), &Z_Norm, &BottomRel, &devRel);

	//副軸(x)を作成
	RB_Vec3fNormalize(&devRel, &X_Norm);

	//位置のオフセットを反映
	RB_Vec3fAdd(CentralPos, &devRel, &devPos);

	//オブジェクトの半径を描画
	//DevPlotArrow(arrow_num, "green", pos, &devPos );
	arrow_num++;


	//オブジェクトの軸にBottomRelを足してTopRelを作成
	RB_Vec3fAdd(CentralAxis, &BottomRel, &TopRel);

		for(uint32_t i = 0u; i < 24u; i++)
		{
			RB_Vec3f BottomOffset;
			RB_Vec3f TopOffset;

		//Bottom
			//オブジェクトの主軸(Z_Norm:単位ベクトル)を軸に15(deg)ずつBottomRelベクトルを回転
			RB_VecRotateVec3f(Deg2Rad(15.0f * (float)i), &Z_Norm, &BottomRel, &BottomOffset);

			//オフセットベクトル(Bottom)にオブジェクト軸の始点(pos[CenterPos])を加算
			RB_Vec3fAdd(CentralPos, &BottomOffset, &CircleVtexBottom[i]);
#if 0
			//始点: オブジェクト軸の始点(pos[CenterPos]), 終点: CircleVtexBottom[i]
			DevPlotArrow(arrow_num, "dark-green", pos, &CircleVtexBottom[i] );
			arrow_num++;
#endif

		//Top
			//オブジェクトの主軸(Z_Norm:単位ベクトル)を軸に15(deg)ずつTopRelベクトルを回転
			RB_VecRotateVec3f(Deg2Rad(15.0f * (float)i), &Z_Norm, &TopRel, &TopOffset);

			//オフセットベクトル(Top)にオブジェクト軸の始点(pos[CenterPos])を加算
			RB_Vec3fAdd(CentralPos, &TopOffset, &CircleVtexTop[i]);

			//始点: オブジェクト軸の終点(ObjAxis), 終点: CircleVtexTop[i]
			//DevPlotArrow(arrow_num, "dark-green", &ObjAxis, &CircleVtexTop[i] );
			//arrow_num++;
		}


	//円柱側面
	for(uint32_t i = 0u; i < 24; i++)
	{
		CreateCylinderSide(object_id, objectsolid_val, i, CircleVtexBottom, CircleVtexTop);
		object_id++;
	}

	//半球部分の描画
	RB_Vec3f CapsuleBtmVtex[7u][24u] = { 0.0f };
	RB_Vec3f CapsuleTopVtex[7u][24u] = { 0.0f };

	for(uint32_t i = 0u; i < 7u; i++)
	{
		RB_Vec3f BtmVertical, BtmOffset, BtmVertx;
		RB_Vec3f TpVertical, TpOffset, TpVertx;
		RB_VecRotateVec3f(Deg2Rad(-15.0f * (float)i), &X_Norm, &BottomRel, &BtmVertical);
		RB_VecRotateVec3f(Deg2Rad( 15.0f * (float)i), &X_Norm, &BottomRel, &TpVertical);

		for(uint32_t j = 0u; j < 24u; j++)
		{
			//底部の描画
			RB_VecRotateVec3f(Deg2Rad(15.0f * (float)j), &Z_Norm, &BtmVertical, &BtmOffset);
			RB_Vec3fAdd(CentralPos, &BtmOffset, &CapsuleBtmVtex[i][j]);
			//RB_Vec3fAdd(pos, &BtmOffset, &BtmVertx);
			//DevPlotArrow(arrow_num, "orange", pos, &BtmVertx );
			//arrow_num++;

			//上部の描画
			RB_VecRotateVec3f(Deg2Rad(15.0f * (float)j), &Z_Norm, &TpVertical, &TpOffset);
			RB_Vec3fAdd(&ObjAxis, &TpOffset, &CapsuleTopVtex[i][j]);
			//RB_Vec3fAdd(&ObjAxis, &TpOffset, &TpVertx);
			//DevPlotArrow(arrow_num, "orange", &ObjAxis, &TpVertx);
			//arrow_num++;
		}
	}

	for(uint32_t i = 0u; i < 6u; i++)
	{
		for(uint32_t j = 0u; j < 24u; j++)
		{
			uint32_t k = (j == 23u) ? 0u : (j + 1u);

			fprintf(plt_3d, "set style fill transparent solid %f \n", 0.3f);
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

			fprintf(plt_3d, "set style fill transparent solid %f \n", 0.3f);
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





RBSTATIC void DrawCapsule(uint32_t id)
{
	float objectsolid_val = 0.3f;
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);
	uint32_t object_id = f_ObjectStartId[id];
	
	///Sphereを作成
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

//=======================================================================================

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
	for(uint32_t i = 0u; i < 8u; i++)
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
		RB_Vec3f EndPos;
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

			default:
				SSV_obj = &Object->Sphere;
				float Sphere_Radius = SSV_obj->Radius;
				RB_Vec3fCreate(0.0f, 0.0f, Sphere_Radius, &EndPos);
				break;
		}

		float Radius = SSV_obj->Radius;

		RB_Vec3f RotVec, ArrowVec, Vertical, RadiusArrow;
		RB_MulMatVec3f(m, &EndPos, &RotVec);
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
		RB_CalcVerticalVec3f(&EndPos, &Vertical);
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
		arrow_num++;

		fprintf(plt_3d,"set colorsequence classic\n");
	}
}

RBSTATIC void DrawObject3d(void)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	for(uint32_t id = 1u; id < (uint32_t)OBJECT_MAXID; id++)
	{
		switch((ObjectData[id].ShapeType))
		{
			case 1u:
				DrawSphere(id);
				break;

			case 2u:
				DrawCapsule(id);
				break;

			case 3u:
				DrawCylinder(id);
				break;

			default:
				DrawBox(id);
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
	fprintf(plt_3d," splot -1 \n");
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
