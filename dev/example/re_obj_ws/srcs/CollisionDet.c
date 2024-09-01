#include "CollisionDet.h"
#include "MainCommon.h"
#include "MainTypeDef.h"
#include "DbgCmd.h"
#include "RB_Math.h"
#include <math.h>

RBSTATIC uint32_t f_SegmentId = 0u;
RBSTATIC RBCONST float f_eps = 1E-05f;

RBSTATIC float Clamp(float val, float min_val, float max_val)
{
	return RB_max(min_val, (RB_min(val, max_val)));
}

//カプセルの情報を取得
RBSTATIC void GetCapsuleData(uint32_t mon_id, RB_Vec3f *StPos, RB_Vec3f *EdPos, float *radius)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	uint8_t type = ObjectData[mon_id].ShapeType;
	RBAssert(type == 2u);

	SSV_T Capsule = ObjectData[mon_id].SSVData;

	*radius = RB_Vec3fGetElem( &(Capsule.SSV_Size), 0u);
	float Rel_Size = RB_Vec3fGetElem( &(Capsule.SSV_Size), 1u);	

	RB_Vec3f rel_CentralAxis;
	RB_Vec3f wcs_edpos;

	RB_Vec3f EndPos;	
	RB_Vec3f u_rel = Capsule.Unit_Rel;
	RB_Vec3fCreate(((Rel_Size)*(u_rel.e[0])), ((Rel_Size)*(u_rel.e[1])), ((Rel_Size)*(u_rel.e[2])), &EndPos);

	//姿勢を中心軸に反映
	RB_MulMatVec3f( &(ObjectData[mon_id].CenterRot), &EndPos, &rel_CentralAxis);

	//中心軸のEndPosをWCSに反映
	RB_Vec3fAdd(&(ObjectData[mon_id].CenterPos), &rel_CentralAxis, &wcs_edpos);

	*StPos = ObjectData[mon_id].CenterPos;
	*EdPos = wcs_edpos;

}

//線分と線分の最短距離, 最近接点取得計算関数
RBSTATIC float ClosestPt_SegmentSegment(RB_Vec3f *st1, RB_Vec3f *ed1, RB_Vec3f *st2, RB_Vec3f *ed2,
float *L1s, float *L2t, RB_Vec3f *L1c, RB_Vec3f *L2c)
{
	RB_Vec3f d1, d2, r;

	RB_Vec3fSub(ed1, st1, &d1);	//線分S1の方向ベクトル
	RB_Vec3fSub(ed2, st2, &d2);	//線分S2の方向ベクトル
	RB_Vec3fSub(st1, st2, &r);

	float a = RB_Vec3fDot(&d1, &d1); //線分S1の距離の平方
	float e = RB_Vec3fDot(&d2, &d2); //線分S2の距離の平方
	float f = RB_Vec3fDot(&d2, &r);

	float s = 0.0f, t = 0.0f;

	//片方あるいは両方の線分が点に縮退してるかチェック
	if( (a <= f_eps ) && (e <= f_eps))
	{
		//両方の線分が点に縮退
		s = t = 0.0f;

		RB_Vec3f rel;
		RB_Vec3fSub(st1, st2, &rel);
		*L1s = s;
		*L2t = t;
		L1c = st1;
		L2c = st2;
		//TODO 要returnロジックの修正
		return RB_Vec3fDot(&rel, &rel);
	}

	//最初の線分が点に縮退
	if( a <= f_eps)
	{
		s = 0.0f;
		t = f / e; // s = 0 => t = ( b * s + f) / e = f / e
		t = Clamp(t, 0.0f, 1.0f);
	}
	else
	{
		float c = RB_Vec3fDot(&d1, &r);

		if(e <= f_eps)
		{
			//2番目の線分が点に縮退
			t = 0.0f;
			s = Clamp( ((-c) / a), 0.0f, 1.0f ); //t = 0 => s = (b * t - c) / a = -c / a
		}

		//ここから一般的な縮退の場合を開始
		else
		{
			float b = RB_Vec3fDot(&d1, &d2);

			float tnom = b * s + f;

			if(tnom < 0.0f)
			{
				t = 0.0f;
				s = Clamp( ((-c) / a), 0.0f, 1.0f);
			}
			else if(tnom > e)
			{
				t = 1.0f;
				s = Clamp( ((b -c) / a), 0.0f, 1.0f);
			}
			else
			{
				t = tnom / e;
			}
		}
	}

	RB_Vec3f d1s, d2t;
	for(uint8_t i = 0u; i < 3u; i++)
	{
		d1s.e[i] = d1.e[i] * s;
		d2t.e[i] = d2.e[i] * t;
	}

	RB_Vec3f Sc1, Sc2;
	RB_Vec3fAdd(st1, &d1s, &Sc1);
	RB_Vec3fAdd(st2, &d2t, &Sc2);

	RB_Vec3f rel;
	RB_Vec3fSub(&Sc1, &Sc2, &rel);
	*L1s = s;
	*L2t = t;
	*L1c = Sc1;
	*L2c = Sc2;
	return RB_Vec3fDot(&rel, &rel);
}

RBSTATIC void GetBoxAreaUnitVec3f(uint32_t area_id, RB_Vec3f *UnitList)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	RB_Mat3f m = ObjectData[area_id].CenterRot;

	UnitList[0u].e[0u] = RB_Mat3fGetElem(&m, 0u, 0u);
	UnitList[0u].e[1u] = RB_Mat3fGetElem(&m, 1u, 0u);
	UnitList[0u].e[2u] = RB_Mat3fGetElem(&m, 2u, 0u);

	UnitList[1u].e[0u] = RB_Mat3fGetElem(&m, 0u, 1u);
	UnitList[1u].e[1u] = RB_Mat3fGetElem(&m, 1u, 1u);
	UnitList[1u].e[2u] = RB_Mat3fGetElem(&m, 2u, 1u);

	UnitList[2u].e[0u] = RB_Mat3fGetElem(&m, 0u, 2u);
	UnitList[2u].e[1u] = RB_Mat3fGetElem(&m, 1u, 2u);
	UnitList[2u].e[2u] = RB_Mat3fGetElem(&m, 2u, 2u);
}

#if 1
//BoxAreaのエッジ情報を取得
RBSTATIC void GetBoxAreaEdgeSegments(uint32_t area_id, SEGMENT_T *EdgeData)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);
	RBCONST BOX_T Box_obj = ObjectData[area_id].BoxData;
	RB_Vec3f OBB_BoxSize = Box_obj.BoxSize;

	float lx = RB_Vec3fGetElem(&OBB_BoxSize, 0u);
	float ly = RB_Vec3fGetElem(&OBB_BoxSize, 1u);
	float lz = RB_Vec3fGetElem(&OBB_BoxSize, 2u);

	RB_Vec3f BoxInitArray[8u];
	RB_Vec3fCreate( -lx, -ly, -lz, &(BoxInitArray[0u]));//P0
	RB_Vec3fCreate(  lx, -ly, -lz, &(BoxInitArray[1u]));//P1
	RB_Vec3fCreate(  lx,  ly, -lz, &(BoxInitArray[2u]));//P2
	RB_Vec3fCreate( -lx,  ly, -lz, &(BoxInitArray[3u]));//P3
	RB_Vec3fCreate( -lx, -ly,  lz, &(BoxInitArray[4u]));//P4
	RB_Vec3fCreate(  lx, -ly,  lz, &(BoxInitArray[5u]));//P5
	RB_Vec3fCreate(  lx,  ly,  lz, &(BoxInitArray[6u]));//P6
	RB_Vec3fCreate( -lx,  ly,  lz, &(BoxInitArray[7u]));//P7

	EdgeData[0u].StPos = BoxInitArray[0u];
	EdgeData[0u].EdPos = BoxInitArray[1u];
	EdgeData[1u].StPos = BoxInitArray[1u];
	EdgeData[1u].EdPos = BoxInitArray[2u];
	EdgeData[2u].StPos = BoxInitArray[2u];
	EdgeData[2u].EdPos = BoxInitArray[3u];
	EdgeData[3u].StPos = BoxInitArray[3u];
	EdgeData[3u].EdPos = BoxInitArray[0u];

	EdgeData[4u].StPos = BoxInitArray[4u];
	EdgeData[4u].EdPos = BoxInitArray[5u];
	EdgeData[5u].StPos = BoxInitArray[5u];
	EdgeData[5u].EdPos = BoxInitArray[6u];
	EdgeData[6u].StPos = BoxInitArray[6u];
	EdgeData[6u].EdPos = BoxInitArray[7u];
	EdgeData[7u].StPos = BoxInitArray[7u];
	EdgeData[7u].EdPos = BoxInitArray[4u];

	EdgeData[8u].StPos = BoxInitArray[0u];
	EdgeData[8u].EdPos = BoxInitArray[4u];
	EdgeData[9u].StPos = BoxInitArray[1u];
	EdgeData[9u].EdPos = BoxInitArray[5u];
	EdgeData[10u].StPos = BoxInitArray[2u];
	EdgeData[10U].EdPos = BoxInitArray[6u];
	EdgeData[11u].StPos = BoxInitArray[3u];
	EdgeData[11u].EdPos = BoxInitArray[7u];
}
#endif

//直方体のエッジを取得
RBSTATIC void GetBoxEdges(uint8_t area_id, RB_Vec3f *Edge_St, RB_Vec3f *Edge_Ed)
{

	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);
	RBCONST BOX_T Box_obj = ObjectData[area_id].BoxData;
	RB_Vec3f OBB_BoxSize = Box_obj.BoxSize;

	float lx = RB_Vec3fGetElem(&OBB_BoxSize, 0u);
	float ly = RB_Vec3fGetElem(&OBB_BoxSize, 1u);
	float lz = RB_Vec3fGetElem(&OBB_BoxSize, 2u);

	RB_Vec3f BoxInitArray[8u];
	RB_Vec3fCreate( -lx, -ly, -lz, &(BoxInitArray[0u]));//P0
	RB_Vec3fCreate(  lx, -ly, -lz, &(BoxInitArray[1u]));//P1
	RB_Vec3fCreate(  lx,  ly, -lz, &(BoxInitArray[2u]));//P2
	RB_Vec3fCreate( -lx,  ly, -lz, &(BoxInitArray[3u]));//P3
	RB_Vec3fCreate( -lx, -ly,  lz, &(BoxInitArray[4u]));//P4
	RB_Vec3fCreate(  lx, -ly,  lz, &(BoxInitArray[5u]));//P5
	RB_Vec3fCreate(  lx,  ly,  lz, &(BoxInitArray[6u]));//P6
	RB_Vec3fCreate( -lx,  ly,  lz, &(BoxInitArray[7u]));//P7

	//CenterRotを反映
	for(uint32_t i = 0u; i < 8u; i++)
	{
		RB_Vec3f RotVec;
		RB_MulMatVec3f(&ObjectData[area_id].CenterRot, &BoxInitArray[i], &RotVec);
		RB_Vec3fAdd(&ObjectData[area_id].CenterPos, &RotVec, &BoxInitArray[i]);
	}

	Edge_St[0u] = BoxInitArray[0u];
	Edge_Ed[0u] = BoxInitArray[1u];

	Edge_St[1u] = BoxInitArray[1u];
	Edge_Ed[1u] = BoxInitArray[2u];

	Edge_St[2u] = BoxInitArray[2u];
	Edge_Ed[2u] = BoxInitArray[3u];

	Edge_St[3u] = BoxInitArray[3u];
	Edge_Ed[3u] = BoxInitArray[0u];

	Edge_St[4u] = BoxInitArray[4u];
	Edge_Ed[4u] = BoxInitArray[5u];

	Edge_St[5u] = BoxInitArray[5u];
	Edge_Ed[5u] = BoxInitArray[6u];

	Edge_St[6u] = BoxInitArray[6u];
	Edge_Ed[6u] = BoxInitArray[7u];

	Edge_St[7u] = BoxInitArray[7u];
	Edge_Ed[7u] = BoxInitArray[4u];

	Edge_St[8u] = BoxInitArray[0u];
	Edge_Ed[8u] = BoxInitArray[4u];

	Edge_St[9u] = BoxInitArray[1u];
	Edge_Ed[9u] = BoxInitArray[5u];

	Edge_St[10u] = BoxInitArray[2u];
	Edge_Ed[10u] = BoxInitArray[6u];

	Edge_St[11u] = BoxInitArray[3u];
	Edge_Ed[11u] = BoxInitArray[7u];

}

#if 0
//Sphere vs RectAngle
RBSTATIC bool CollDetSphere_vs_RectAngle_Unit(RB_Vec3f *CPos, float Radius, RBCONST OBJECT_T *Object, uint8_t ColorId)
{
	bool ret = false;

	RB_Vec3f wcs_q;
	RB_Vec3f rel_q;

	//WCS(p)に対してOBB上(または内部)にあるWCS(p)の最近接点qを返す
	ClosestPtPointOBB( CPos, Object, &wcs_q);

#if 0
	//最近接点から球体中心までのベクトルを描画
	f_SegmentId++;
	DbgCmd_SetSegment(f_SegmentId, ColorId, &wcs_q, CPos);

#endif

	//最近接点wcs_qと相対距離ベクトルrel_qを計算
	RB_Vec3fSub(&wcs_q, CPos, &rel_q);

	//相対距離ベクトル rel_q を2乗し、大きさを求める
	//それが半径の2乗以下ならOBBに球体が接触していると判定
	if(RB_Vec3fDot(&rel_q,&rel_q) <= (Radius * Radius))
	{
		ret = true;
	}

	return ret;
}

//丸い長方形 vs OBB 当たり判定
//TODO 処理を関数の関数に分ける必要あり
RBSTATIC bool CollDetRoundRectAngle_vs_OBB(uint32_t rectangle_id, uint32_t area_id)
{
	bool ret = false;
	bool skip_f = true;

	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	SSV_T RoundRectAngle_obj = ObjectData[rectangle_id].SSVData;

	RB_Vec3f CenterPos = ObjectData[rectangle_id].CenterPos;
	RB_Mat3f CenterRot = ObjectData[rectangle_id].CenterRot;

	RB_Vec3f u[3u] = { 0.0f };
	RB_Vec3f now_u[3u] = { 0.0f };

	u[0u] = RoundRectAngle_obj.Unit_Rel;
	u[1u] = RoundRectAngle_obj.Unit_Height;
	RB_Vec3fCross(&u[0u], &u[1u], &u[2u]);

	//姿勢を単位ベクトルに反映
	for(uint8_t i = 0u; i < 3u; i++)
	{
		RB_MulMatVec3f(&CenterRot, &u[i], &now_u[i]);
	}

	//RectAngleのサイズを指定
	float Radius = RB_Vec3fGetElem( &(RoundRectAngle_obj.SSV_Size), 0u);

	RB_Vec3f length = { 0.0f };
	length.e[0u] = RB_Vec3fGetElem( &(RoundRectAngle_obj.SSV_Size), 1u);
	length.e[1u] = RB_Vec3fGetElem( &(RoundRectAngle_obj.SSV_Size), 2u);	

	RB_Vec3f RelOfs, WidthOfs, DiagonalOfs;

	RB_Vec3fCreate(((length.e[0u])*(now_u[0u].e[0u])), ((length.e[0u])*(now_u[0u].e[1u])), ((length.e[0u])*(now_u[0u].e[2u])), &RelOfs);
	RB_Vec3fCreate(((length.e[1u])*(now_u[1u].e[0u])), ((length.e[1u])*(now_u[1u].e[1u])), ((length.e[1u])*(now_u[1u].e[2u])), &WidthOfs);
	RB_Vec3fAdd(&RelOfs, &WidthOfs, &DiagonalOfs);

	for(uint8_t i = 0u; i < 3u; i++)
	{
		RelOfs.e[i] += CenterPos.e[i];
		WidthOfs.e[i] += CenterPos.e[i];
		DiagonalOfs.e[i] += CenterPos.e[i];
	}

//TODO RectAngleのエッジを作成する関数に分ける必要あり
	RB_Vec3f RectAngle_Edge_St[4u] = { 0.0f };
	RB_Vec3f RectAngle_Edge_Ed[4u] = { 0.0f };

	RectAngle_Edge_St[0u] = CenterPos;
	RectAngle_Edge_Ed[0u] = RelOfs;

	RectAngle_Edge_St[1u] = RelOfs;
	RectAngle_Edge_Ed[1u] = DiagonalOfs;

	RectAngle_Edge_St[2u] = DiagonalOfs;
	RectAngle_Edge_Ed[2u] = WidthOfs;

	RectAngle_Edge_St[3u] = WidthOfs;
	RectAngle_Edge_Ed[3u] = CenterPos;

#if 0
	//デバッグ用
	for(uint8_t i = 0u; i < 4u; i++)
	{
		f_SegmentId++;
		DbgCmd_SetSegment( f_SegmentId, 2u, &RectAngle_Edge_St[i], &RectAngle_Edge_Ed[i]);
	}
#endif

	//直方体のエッジ(12本)とカプセルの円柱部分(線分)との最近接点を計算する

	//Boxのサイズを取得
	RBCONST BOX_T Box_obj = ObjectData[area_id].BoxData;
	RB_Vec3f l = Box_obj.BoxSize;
	RB_Vec3f BCpos = ObjectData[area_id].CenterPos;

	//ポリゴンの頂点を指定
	uint8_t PolygonVertex = 4u;
	uint8_t ObjectEdgeNum = 3 * PolygonVertex;
	uint8_t ObjectVertexNum = 2 * PolygonVertex;

	RB_Vec3f EdgeSt[ObjectEdgeNum];
	RB_Vec3f EdgeEd[ObjectEdgeNum];

	GetBoxEdges(area_id, EdgeSt, EdgeEd);

//RectAngle用のObjectを作成
//TODO ここも関数化すると便利
	OBJECT_T RectAngle_obj;
	RB_Vec3f RectAngleSize;
	RB_Vec3fCreate((length.e[0u] * 0.5f), (length.e[1u] * 0.5f), 0.0f, &RectAngleSize);

	RB_Vec3f CenterRel, CenterWidth, CenterOfs;
	RB_Vec3fCreate(((RectAngleSize.e[0u])*(now_u[0u].e[0u])), 
					((RectAngleSize.e[0u])*(now_u[0u].e[1u])), 
					((RectAngleSize.e[0u])*(now_u[0u].e[2u])), &CenterRel);

	RB_Vec3fCreate(((RectAngleSize.e[1u])*(now_u[1u].e[0u])), 
					((RectAngleSize.e[1u])*(now_u[1u].e[1u])), 
					((RectAngleSize.e[1u])*(now_u[1u].e[2u])), &CenterWidth);

	RB_Vec3fAdd(&CenterRel, &CenterWidth, &CenterOfs);
	RB_Vec3fAdd(&CenterPos, &CenterOfs, &(RectAngle_obj.CenterPos));

	RB_Mat3fCreate(
		now_u[0u].e[0u], now_u[1u].e[0u], now_u[2u].e[0u],
		now_u[0u].e[1u], now_u[1u].e[1u], now_u[2u].e[1u],
		now_u[0u].e[2u], now_u[1u].e[2u], now_u[2u].e[2u],
		&(RectAngle_obj.CenterRot)
	);

	RectAngle_obj.BoxData.BoxSize = RectAngleSize;

	//直方体のエッジ(12本)に対する丸い長方形のエッジ上の最近接点を計算する
	if(skip_f)
	{
		uint8_t i = 0u;
		uint8_t colorId = 1u;
		bool flag = false;

		//エッジ毎との最近接点を求める
		//求めた最近接点を中心とした球体の領域を作り、OBBと当たり判定を行う
		while(i < ObjectEdgeNum)
		{
			uint8_t n = 0u;
			//4u: RectAngleのEdge
			while(n < 4u)
			{
				//OBB上のエッジとRectAngle上のエッジでそれぞれの最近接点(p1, p2)を計算
				RB_Vec3f p1, p2;
				GetMonIdClosestPt(&RectAngle_Edge_St[n], &RectAngle_Edge_Ed[n], &EdgeSt[i], &EdgeEd[i], &p1, &p2);

//====================================================================================
				//RectAngleをOBB と OBB上の球体 の当たり判定 
				if( (n == 0u) && (CollDetSphere_vs_RectAngle_Unit(&p2, Radius, &RectAngle_obj, 3u)))
				{
#if 0
					//デバッグ用
					f_SegmentId++;
					DbgCmd_SetSegment( f_SegmentId, 1u, &p2, &p1);
#endif

					flag = true;
					break;
				}
//====================================================================================

//====================================================================================
				//OBBとRectAngle上の球体と当たり判定
				if((CollDetSphere_vs_OBB_Unit(&p1, Radius, area_id, colorId)))
				{

#if 0
					//デバッグ用
					f_SegmentId++;
					DbgCmd_SetSegment( f_SegmentId, 4u, &p1, &p2);
#endif

					flag = true;
					break;
				}
//====================================================================================

				n++;
			}

			//衝突と判定したらwhileループを終了、抜け出す
			if(flag)
			{
				ret = true;
				break;
			}
			//次のエッジへ進む
			i++;
		}
	}

	return ret;
}
#endif

//=========================================================================================

RBSTATIC void ConvBoxAreaLocalPos(RBCONST RB_Vec3f *Rel, RBCONST RB_Vec3f *BoxUnit, RB_Vec3f *LocalPos)
{
	LocalPos->e[0u] = RB_Vec3fDot(Rel, &BoxUnit[0u]);
	LocalPos->e[1u] = RB_Vec3fDot(Rel, &BoxUnit[1u]);
	LocalPos->e[2u] = RB_Vec3fDot(Rel, &BoxUnit[2u]);
}

RBSTATIC bool IsOverlapSphere_OBB(RBCONST RB_Vec3f *LCS_CPos, float Radius, RBCONST RB_Vec3f *BoxSize)
{
	bool ret = false;

	RB_Vec3f LCS_ClosestPt;
	for(uint8_t i = 0u; i < 3u; i++)
	{
		//ボックス上の球体への最近接点を計算
		LCS_ClosestPt.e[i] = Clamp(LCS_CPos->e[i], (-(BoxSize->e[i])), (BoxSize->e[i]));
	}

	//LCS上の最近接点と球体中心までの相対距離を計算
	RB_Vec3f rel_q;
	RB_Vec3fSub(LCS_CPos, &LCS_ClosestPt, &rel_q);

	//相対距離ベクトル rel_q を2乗し、大きさを求める
	//それが半径の2乗以下ならOBBに球体が接触していると判定
	if(RB_Vec3fDot(&rel_q,&rel_q) <= (Radius * Radius))
	{
		ret = true;
	}
	return ret;
}

RBSTATIC bool IsOutSideSphere_OBB(RBCONST RB_Vec3f *LCS_CPos, float Radius, RBCONST RB_Vec3f *BoxSize)
{
	bool ret = false;

	//ボックス基準(ローカル座標系)の位置を計算する
	for(uint8_t i = 0u; i < 3u; i++)
	{
		float min = (-(BoxSize->e[i])) + Radius;
		float max = ( (BoxSize->e[i])) - Radius;

		//LCS_CPosがBoxの範囲外なら接触と判定
		if( (min >= (LCS_CPos->e[i]) ) || (max <= (LCS_CPos->e[i]) ))
		{
			ret = true;
			break;
		}
	}

	return ret;
}

RBSTATIC bool IsCollision_Sphere(uint32_t sphere_id, uint32_t area_id)
{
	bool ret = false;
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

//球体の情報を取得=====================================================================
	OBJECT_T SphereObject = ObjectData[sphere_id];

	SSV_T SphereData = SphereObject.SSVData;
	float Radius = RB_Vec3fGetElem( &(SphereData.SSV_Size), 0u);

//ボックスエリアの情報を取得===========================================================
	OBJECT_T BoxObject = ObjectData[area_id];
	RB_Vec3f BoxUnit[3u];
	GetBoxAreaUnitVec3f(area_id, BoxUnit);

	BOX_T BoxData = BoxObject.BoxData;
	RB_Vec3f BoxSize = BoxData.BoxSize;
	bool BoxAreaType = BoxData.AreaType;
//======================================================================================

	RB_Vec3f Rel;
	//Box中心からの距離を計算
	RB_Vec3fSub(&SphereObject.CenterPos, &BoxObject.CenterPos, &Rel);

	RB_Vec3f Local_CPos;
	ConvBoxAreaLocalPos(&Rel, BoxUnit, &Local_CPos);

//======BoxAreaのタイプで内包判定, 衝突判定を実施する=========//
	//WorkAreaの場合
	if(BoxAreaType)
	{
		ret = IsOutSideSphere_OBB(&Local_CPos, Radius, &BoxSize);
	}
	//BlockAreaの場合
	else
	{
		ret = IsOverlapSphere_OBB(&Local_CPos, Radius, &BoxSize);
	}

	return ret;
}

RBSTATIC bool IsOverlapCapsule_OBB(RB_Vec3f *LCS_StPos, RB_Vec3f *LCS_EdPos, float Radius, RBCONST RB_Vec3f *BoxSize, SEGMENT_T *BoxEdges)
{
	bool ret = false;

	//とりあえず全部使う
	uint8_t i = 0u;

	while(i < 12u)
	{
		float t, s, dSq;
		RB_Vec3f Cp_ClosestPt, Box_ClosestPt;

		//TODO 必要な回数を減らす
		//線分と線分の最近接点を計算する
		dSq = ClosestPt_SegmentSegment(LCS_StPos, LCS_EdPos, &(BoxEdges[i].StPos), &(BoxEdges[i].EdPos), &t, &s, &Cp_ClosestPt, &Box_ClosestPt);

		//求めた最近接点を中心とした球体の領域を作り、OBBと当たり判定を行う
		//当たりがあればその時点でエッジとの判定を終了
		if(IsOverlapSphere_OBB(&Cp_ClosestPt, Radius, BoxSize))
		{
			ret = true;
			break;
		}
		//次のエッジへ進む
		i++;	
	}

	return ret;
}

RBSTATIC bool IsCollision_Capsule(uint32_t capsule_id, uint32_t area_id)
{
	bool ret = false;

	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	//カプセルの最新位置情報を取得
	RB_Vec3f Cp_St, Cp_Ed;
	float Radius;
	GetCapsuleData(capsule_id, &Cp_St, &Cp_Ed, &Radius);

//ボックスエリアの情報を取得===========================================================
	OBJECT_T BoxObject = ObjectData[area_id];
	RB_Vec3f BoxUnit[3u];
	GetBoxAreaUnitVec3f(area_id, BoxUnit);

	BOX_T BoxData = BoxObject.BoxData;
	RB_Vec3f BoxSize = BoxData.BoxSize;
	bool BoxAreaType = BoxData.AreaType;
//======================================================================================

//======BoxAreaのタイプで内包判定, 衝突判定を実施する=========//

	RB_Vec3f Rel_StPos, Rel_EdPos;
	RB_Vec3f Local_StPos, Local_EdPos;
	bool retSt = false;
	bool retEd = false;

	//Box中心からの距離を計算
	RB_Vec3fSub(&Cp_St, &BoxObject.CenterPos, &Rel_StPos);
	RB_Vec3fSub(&Cp_Ed, &BoxObject.CenterPos, &Rel_EdPos);
	
	//Box基準の座標系位置に変換
	ConvBoxAreaLocalPos(&Rel_StPos, BoxUnit, &Local_StPos);
	ConvBoxAreaLocalPos(&Rel_EdPos, BoxUnit, &Local_EdPos);

	//WorkAreaの場合
	if(BoxAreaType)
	{
		retSt = IsOutSideSphere_OBB(&Local_StPos, Radius, &BoxSize);
		retEd = IsOutSideSphere_OBB(&Local_EdPos, Radius, &BoxSize);

		//始点 or 終点 がBoxAreaに接触
		if(retSt || retEd)
		{
			ret = true;
		}
	}
	//BlockAreaの場合
	else
	{
		//Boxのエッジを取得する
		SEGMENT_T BoxEdges[12u];
		GetBoxAreaEdgeSegments(area_id, BoxEdges);
		ret = IsOverlapCapsule_OBB(&Local_StPos, &Local_EdPos, Radius, &BoxSize, BoxEdges);
	}

	return ret;
}


RBSTATIC bool IsCollision_RoundRectAngle(uint32_t rectangle_id, uint32_t area_id)
{
	bool ret = false;

	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

//RoundRectAngleの最新位置情報を取得


//ボックスエリアの情報を取得===========================================================
	OBJECT_T BoxObject = ObjectData[area_id];
	RB_Vec3f BoxUnit[3u];
	GetBoxAreaUnitVec3f(area_id, BoxUnit);

	BOX_T BoxData = BoxObject.BoxData;
	RB_Vec3f BoxSize = BoxData.BoxSize;
	bool BoxAreaType = BoxData.AreaType;
//======================================================================================
#if 0
//======BoxAreaのタイプで内包判定, 衝突判定を実施する=========//
	RB_Vec3f Rel_StPos, Rel_EdPos;
	RB_Vec3f Local_StPos, Local_EdPos;
	bool retSt = false;
	bool retEd = false;

	//Box中心からの距離を計算
	RB_Vec3fSub(&Cp_St, &BoxObject.CenterPos, &Rel_StPos);
	RB_Vec3fSub(&Cp_Ed, &BoxObject.CenterPos, &Rel_EdPos);
	
	//Box基準の座標系位置に変換
	ConvBoxAreaLocalPos(&Rel_StPos, BoxUnit, &Local_StPos);
	ConvBoxAreaLocalPos(&Rel_EdPos, BoxUnit, &Local_EdPos);

	//WorkAreaの場合
	if(BoxAreaType)
	{
		retSt = IsOutSideSphere_OBB(&Local_StPos, Radius, &BoxSize);
		retEd = IsOutSideSphere_OBB(&Local_EdPos, Radius, &BoxSize);

		//始点 or 終点 がBoxAreaに接触
		if(retSt || retEd)
		{
			ret = true;
		}
	}
	//BlockAreaの場合
	else
	{
		//Boxのエッジを取得する
		SEGMENT_T BoxEdges[12u];
		GetBoxAreaEdgeSegments(area_id, BoxEdges);
		ret = IsOverlapCapsule_OBB(&Local_StPos, &Local_EdPos, Radius, &BoxSize, BoxEdges);
	}
#endif
	return ret;
}



void CollisionDet_Init(void)
{
	NO_STATEMENT;
}

void CollisionDet_PreStartProc(void)
{
	NO_STATEMENT;
}

RBSTATIC uint8_t GetObjectType(uint32_t mon_id)
{
	uint8_t ret = 0u;

	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	return ObjectData[mon_id].ShapeType;
}

void CollisionDet_Cycle(void)
{
	uint8_t ret[OBJECT_MAXID] = { 0u };
	bool result[OBJECT_MAXID] = { 1u };

	uint32_t area_objnum = 0u;
	uint32_t mon_objectnum = 0u;

	area_objnum = DbgCmd_GetAreaObjectNum();
	mon_objectnum = DbgCmd_GetMonObjectNum();

	uint32_t ObjectBaseNum = 11u;
	uint8_t i = 1u;

	if(area_objnum && mon_objectnum)
	{
		while( i < (mon_objectnum +1u) )
		{
			for(uint32_t j = 0u; j < area_objnum; j++)
			{
				switch(GetObjectType(i))
				{
					case 1u:
						//ret[j] += (uint8_t)CollDetSphere_vs_OBB(i, (j + ObjectBaseNum));
						ret[j] += (uint8_t)IsCollision_Sphere(i, (j + ObjectBaseNum));
						break;

					case 2u:
						//ret[j] += (uint8_t)CollDetCapsule_vs_OBB(i, (j + ObjectBaseNum));
						ret[j] += (uint8_t)IsCollision_Capsule(i, (j + ObjectBaseNum));
						break;

					case 3u:
						//ret[j] += (uint8_t)CollDetRoundRectAngle_vs_OBB(i, (j + ObjectBaseNum));
						ret[j] += (uint8_t)IsCollision_RoundRectAngle(i, (j + ObjectBaseNum));
						break;

					default:
						ret[j] += 1u;
						break;
				}
			}
			i++;
		}

//判定結果をAreaObjectの色を変更
		for(uint8_t n = 0u; n < area_objnum; n++)
		{
			result[n] = (ret[n] == 0u) ? false : true;
			DbgCmd_SetOverlapStatus((n + ObjectBaseNum), result[n]);
		}
	}

	f_SegmentId = 0u;
}

void CollisionDet_Destroy(void)
{
	NO_STATEMENT;
}