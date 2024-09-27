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
			float denom = a * e - b * b; //常に非負

			//線分が並行でない場合、L1上のL2に対する最近接点を計算。
			//そして線分S1に対してクランプ。そうでない場合は任意s(ここでは0)を選択
			if(denom != 0.0f)
			{
				s = Clamp( (b * f - c * e) / denom, 0.0f, 1.0f);
			}
			else
			{
				s = 0.0f;
			}
			//L2上のS1(s)に対する最近接点を以下を用いて計算
			//t = Dot((P1 + D1 * s) - P2, D2) / Dot(D2, D2) = (b * s + f) / e
			t = ( b * s + f) / e;

			//tが[0, 1]の中にあれば終了。そうでなければtをクランプ、sをtの新しい値に対して以下を用いて再計算\
			//s = Dot((P2 + D2 * t) - P1, D1) / Dot(D1, D1) = (t + b - c) / a
			//そしてsを[0, 1]に対してクランプ
			if( t < 0.0f)
			{
				t = 0.0f;
				s = Clamp( ((-c) / a ), 0.0f, 1.0f);
			}
			else if( t > 1.0f)
			{
				t = 1.0f;
				s = Clamp( ((b - c) / a), 0.0f, 1.0f);
			}
			else
			{
				NO_STATEMENT;
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

//WCS(p)から見たOBBの最近接点を取得
//WCS(p)に対してOBB上(または内部)にあるWCS(p)の最近接点qを返す
RBSTATIC void ClosestPtPointOBB( RBCONST RB_Vec3f *p, OBJECT_T *Object, RB_Vec3f *q)
{
	RBCONST RB_Mat3f *m = &Object->CenterRot;
	RBCONST BOX_T *Box_obj = &Object->BoxData;
	RBCONST RB_Vec3f *l = &Box_obj->BoxSize;

	RB_Vec3f u[3u] = { 0.0f };

	//Boxの姿勢行列からBoxのローカル座標系の単位ベクトルを取得
	for(uint8_t i = 0u; i < 3u; i++)
	{
		//Areaの方向ベクトル(x)を求める
		u[i].e[0u] = RB_Mat3fGetElem(m, 0u, i);
		u[i].e[1u] = RB_Mat3fGetElem(m, 1u, i);
		u[i].e[2u] = RB_Mat3fGetElem(m, 2u, i);
	}

	RB_Vec3f d, local_dist;
	RB_Vec3fSub(p, &(Object->CenterPos), &d);

	float dist;

	//ボックスの中心における結果から開始、そこから段階的に進める
	*q = Object->CenterPos;

	//各OBBの軸に対して以下を実施
	for(uint8_t i = 0u; i < 3u; i++)
	{
		//ボックスの中心からdの軸に沿った距離を得る
		dist = RB_Vec3fDot(&d, &u[i]);

		//ボックスの範囲よりも距離が大きい場合、ボックスのdistに固定
		if(dist > l->e[i])
		{
			dist = (l->e[i]);
		}
		if(dist < -(l->e[i]))
		{
			dist = -(l->e[i]);
		}

		//WCS(q)を得るためにその距離だけ軸に沿って進める
		q->e[0u] += dist * u[i].e[0u];
		q->e[1u] += dist * u[i].e[1u];
		q->e[2u] += dist * u[i].e[2u];
	}

}

//CPosを中心とした球体とarea_idで指定したOBBとの当たり判定用 単体関数
RBSTATIC bool CollDetSphere_vs_OBB_Unit(RB_Vec3f *CPos, float Radius, OBJECT_T *Object, uint8_t ColorId)
{
	bool ret = false;

	RB_Vec3f wcs_q;
	RB_Vec3f rel_q;

	//WCS(p)に対してOBB上(または内部)にあるWCS(p)の最近接点qを返す
	ClosestPtPointOBB( CPos, Object, &wcs_q);

#if 0
	//Debug用
	//最近接点から球体中心までのベクトルを描画
	f_SegmentId++;
	DbgCmd_SetSegment(f_SegmentId, ColorId, &wcs_q, CPos);

	//DbgCmd_SetVec3f("nearest neighbor point: ", &wcs_q);
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
	RB_Vec3f CapsulePos[2u] = { 0.0f };
	float Radius;
	GetCapsuleData(capsule_id, &CapsulePos[0u], &CapsulePos[1u], &Radius);

//ボックスエリアの情報を取得===========================================================
	OBJECT_T BoxObject = ObjectData[area_id];
	RB_Vec3f BoxUnit[3u];
	GetBoxAreaUnitVec3f(area_id, BoxUnit);

	BOX_T BoxData = BoxObject.BoxData;
	RB_Vec3f BoxSize = BoxData.BoxSize;
	bool BoxAreaType = BoxData.AreaType;
//======================================================================================

//======BoxAreaのタイプで内包判定, 衝突判定を実施する=========//
	RB_Vec3f Local_Pos[2u] = { 0.0f };
	for(uint8_t i = 0u; i < 2u; i++)
	{
		RB_Vec3f Rel;
		//Box中心からの距離を計算
		RB_Vec3fSub(&CapsulePos[i], &BoxObject.CenterPos, &Rel);
		//Box基準の座標系位置に変換
		ConvBoxAreaLocalPos(&Rel, BoxUnit, &Local_Pos[i]);
	}

	//WorkAreaの場合
	if(BoxAreaType)
	{
		for(uint8_t i = 0u; i < 2u; i++)
		{
			if(IsOutSideSphere_OBB(&Local_Pos[i], Radius, &BoxSize))
			{
				ret = true;
				break;
			}
		}
	}
	//BlockAreaの場合
	else
	{
		//Boxのエッジを取得する
		SEGMENT_T BoxEdges[12u];
		GetBoxAreaEdgeSegments(area_id, BoxEdges);
		ret = IsOverlapCapsule_OBB(&Local_Pos[0u], &Local_Pos[1u], Radius, &BoxSize, BoxEdges);
	}

	return ret;
}

RBSTATIC void GetRoundRectAngleData(uint32_t rectangle_id, SEGMENT_T *EdgeData, RB_Vec3f *now_u, RB_Vec3f *RectAngleSize)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	OBJECT_T RectAngleObject = ObjectData[rectangle_id];

	uint8_t type = RectAngleObject.ShapeType;
	RBAssert(type == 3u);

	SSV_T rectangle_data = RectAngleObject.SSVData;

	RB_Vec3f CenterPos = RectAngleObject.CenterPos;
	RB_Mat3f CenterRot = RectAngleObject.CenterRot;

	RB_Vec3f u[3u] = { 0.0f };
	//RB_Vec3f now_u[3u] = { 0.0f };

	u[0u] = rectangle_data.Unit_Rel;
	u[1u] = rectangle_data.Unit_Height;
	RB_Vec3fCross(&u[0u], &u[1u], &u[2u]);

	//姿勢を単位ベクトルに反映
	for(uint8_t i = 0u; i < 3u; i++)
	{
		RB_MulMatVec3f(&CenterRot, &u[i], &now_u[i]);
	}

	//RectAngleのサイズを指定
	float Radius = RB_Vec3fGetElem( &(rectangle_data.SSV_Size), 0u);
	float Rel_Size = RB_Vec3fGetElem( &(rectangle_data.SSV_Size), 1u);
	float Height_Size = RB_Vec3fGetElem( &(rectangle_data.SSV_Size), 2u);

	RectAngleSize->e[0u] = Radius;
	RectAngleSize->e[1u] = Rel_Size;
	RectAngleSize->e[2u] = Height_Size;

	RB_Vec3f RelOfs, HeightOfs, DiagonalOfs;

	RB_Vec3fCreate(((Rel_Size)*(now_u[0u].e[0u])), ((Rel_Size)*(now_u[0u].e[1u])), ((Rel_Size)*(now_u[0u].e[2u])), &RelOfs);
	RB_Vec3fCreate(((Height_Size)*(now_u[1u].e[0u])), ((Height_Size)*(now_u[1u].e[1u])), ((Height_Size)*(now_u[1u].e[2u])), &HeightOfs);
	RB_Vec3fAdd(&RelOfs, &HeightOfs, &DiagonalOfs);

	RB_Vec3f Vertexs[4u] = { 0.0f }; 
	Vertexs[0u] = CenterPos;
	RB_Vec3fAdd(&CenterPos, &RelOfs,&Vertexs[1u]);
	RB_Vec3fAdd(&CenterPos, &DiagonalOfs,&Vertexs[2u]);
	RB_Vec3fAdd(&CenterPos, &HeightOfs,&Vertexs[3u]);

	EdgeData[0u].StPos = CenterPos;
	EdgeData[0u].EdPos = Vertexs[1u];
	EdgeData[1u].StPos = Vertexs[1u];
	EdgeData[1u].EdPos = Vertexs[2u];
	EdgeData[2u].StPos = Vertexs[2u];
	EdgeData[2u].EdPos = Vertexs[3u];
	EdgeData[3u].StPos = Vertexs[3u];
	EdgeData[3u].EdPos = Vertexs[0u];

#if 0
	//デバッグ用
	for(uint8_t i = 0u; i < 4u; i++)
	{
		f_SegmentId++;
		DbgCmd_SetSegment( f_SegmentId, 2u, &EdgeData[i].StPos, &EdgeData[i].EdPos);
	}
#endif
}

#if 0
RBSTATIC void GenerateRectAngleBoxObject(RBCONST RB_Vec3f *CPos, RBCONST RB_Vec3f *now_u, RBCONST RB_Vec3f *SSVSize, RB_Vec3f *UpdatePos, RB_Vec3f *UnitList, RB_Vec3f *RectAngleSize)
{
//RectAngle用のObjectを作成
	OBJECT_T RectAngle_obj;
	RB_Vec3fCreate(( (SSVSize->e[1u]) * 0.5f), ( (SSVSize->e[2u]) * 0.5f), 0.0f, RectAngleSize);

	RB_Vec3f CenterRel, CenterHeight, CenterOfs;
	RB_Vec3fCreate(((RectAngleSize->e[0u])*( (now_u[0u].e[0u]) )), 
					((RectAngleSize->e[0u])*( (now_u[0u].e[1u]) )), 
					((RectAngleSize->e[0u])*( (now_u[0u].e[2u]) )), &CenterRel);

	RB_Vec3fCreate(((RectAngleSize->e[1u])*( (now_u[1u].e[0u]) )), 
					((RectAngleSize->e[1u])*( (now_u[1u].e[1u]) )), 
					((RectAngleSize->e[1u])*( (now_u[1u].e[2u]) )), &CenterHeight);

	RB_Vec3fAdd(&CenterRel, &CenterHeight, &CenterOfs);
	RB_Vec3fAdd(CPos, &CenterOfs, UpdatePos);

	UnitList[0u].e[0u] = now_u[0u].e[0u];
	UnitList[0u].e[1u] = now_u[0u].e[1u];
	UnitList[0u].e[2u] = now_u[0u].e[2u];

	UnitList[1u].e[0u] = now_u[1u].e[0u];
	UnitList[1u].e[1u] = now_u[1u].e[1u];
	UnitList[1u].e[2u] = now_u[1u].e[2u];

	UnitList[2u].e[0u] = now_u[2u].e[0u];
	UnitList[2u].e[1u] = now_u[2u].e[1u];
	UnitList[2u].e[2u] = now_u[2u].e[2u];
}
#endif

RBSTATIC void GenerateRectAngleObject(RBCONST RB_Vec3f *CPos, RBCONST RB_Vec3f *now_u, RBCONST RB_Vec3f *RoundRectAngleSize, OBJECT_T *RectAngleObject)
{
//RectAngle用のObjectを作成
	OBJECT_T RectAngle_obj = { 0.0f };
	BOX_T RectAngle;

	RB_Vec3f RectAngleSize;
	RB_Vec3fCreate(( (RoundRectAngleSize->e[1u]) * 0.5f), ( (RoundRectAngleSize->e[2u]) * 0.5f), 0.0f, &RectAngleSize);
	RectAngle.BoxSize = RectAngleSize;
	RectAngle_obj.BoxData = RectAngle;

	RB_Vec3f CenterRel, CenterHeight, CenterOfs;
	RB_Vec3fCreate(((RectAngleSize.e[0u])*( (now_u[0u].e[0u]) )), 
					((RectAngleSize.e[0u])*( (now_u[0u].e[1u]) )), 
					((RectAngleSize.e[0u])*( (now_u[0u].e[2u]) )), &CenterRel);

	RB_Vec3fCreate(((RectAngleSize.e[1u])*( (now_u[1u].e[0u]) )), 
					((RectAngleSize.e[1u])*( (now_u[1u].e[1u]) )), 
					((RectAngleSize.e[1u])*( (now_u[1u].e[2u]) )), &CenterHeight);

	RB_Vec3fAdd(&CenterRel, &CenterHeight, &CenterOfs);
	RB_Vec3fAdd(CPos, &CenterOfs, &(RectAngle_obj.CenterPos));

	RB_Mat3fCreate(
		now_u[0u].e[0u], now_u[1u].e[0u], now_u[2u].e[0u],
		now_u[0u].e[1u], now_u[1u].e[1u], now_u[2u].e[1u],
		now_u[0u].e[2u], now_u[1u].e[2u], now_u[2u].e[2u],
		&(RectAngle_obj.CenterRot)
	);

	memcpy( RectAngleObject, &RectAngle_obj, sizeof(OBJECT_T));
}

//================================================================================================
//丸い長方形 vs OBB 当たり判定
//TODO 処理を関数の関数に分ける必要あり
RBSTATIC bool CollDetRoundRectAngle_vs_OBB(uint32_t roundrectangle_id, uint32_t area_id)
{
	bool ret = false;
	bool skip_f = true;

	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	RB_Vec3f RectAngleUint[3u] = { 0.0f };
	SEGMENT_T RectAngleEdge[4u] = { 0.0f };
	RB_Vec3f RoundRectAngleSize = { 0.0f };

	//RoundRectAngleの最新位置情報を取得
	GetRoundRectAngleData(roundrectangle_id, RectAngleEdge, RectAngleUint, &RoundRectAngleSize);
	float Radius = RoundRectAngleSize.e[0u];

	//直方体のエッジ(12本)とカプセルの円柱部分(線分)との最近接点を計算する
	RB_Vec3f EdgeSt[12u];
	RB_Vec3f EdgeEd[12u];
	GetBoxEdges(area_id, EdgeSt, EdgeEd);

//RectAngle用のObjectを作成
	OBJECT_T RectAngle_obj;
	GenerateRectAngleObject(&(ObjectData[roundrectangle_id].CenterPos), RectAngleUint, &RoundRectAngleSize, &RectAngle_obj);

	//直方体のエッジ(12本)に対する丸い長方形のエッジ上の最近接点を計算する
	if(skip_f)
	{
		uint8_t i = 0u;
		uint8_t colorId = 1u;
		bool flag = false;

		//エッジ毎との最近接点を求める
		//求めた最近接点を中心とした球体の領域を作り、OBBと当たり判定を行う
		while(i < 12u )
		{
			uint8_t n = 0u;
			//4u: RectAngleのEdge
			while(n < 4u)
			{
				//OBB上のエッジとRectAngle上のエッジでそれぞれの最近接点(p1, p2)を計算
				RB_Vec3f p1, p2;
				float t1, t2;
				float dSq = ClosestPt_SegmentSegment(&(RectAngleEdge[n].StPos), &(RectAngleEdge[n].EdPos), 
				&EdgeSt[i], &EdgeEd[i], &t1, &t2, &p1, &p2);

//====================================================================================
				//RectAngleをOBB と OBB上の球体 の当たり判定 
				if( (n == 0u) && (CollDetSphere_vs_OBB_Unit(&p2, Radius, &RectAngle_obj, 3u)))
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
				//OBBとRectAngle上の球体と当たり判定
				if((CollDetSphere_vs_OBB_Unit(&p1, Radius, &ObjectData[area_id], colorId)))
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

//=================================================================================================
RBSTATIC bool IsOverlapSphere_RectAngle(RBCONST RB_Vec3f *LCS_CPos, float Radius, RBCONST RB_Vec3f *RectAngleSize)
{
	bool ret = false;

	RB_Vec3f LCS_ClosestPt;
	for(uint8_t i = 0u; i < 3u; i++)
	{
		//ボックス上の球体への最近接点を計算
		LCS_ClosestPt.e[i] = Clamp(LCS_CPos->e[i], 0.0f, (RectAngleSize->e[i]));
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

//================================================================================================
#if 0
RBSTATIC bool IsOverlapRoundRectAngle_OBB(SEGMENT_T *RectAngleVertexs, float Radius, SEGMENT_T *BoxEdges, 
RBCONST RB_Vec3f *BoxSize, RB_Vec3f *RectAngleUnit, RB_Vec3f *RectAngleSize )
{
	bool flag = false;
	bool ret = false;
	uint8_t i = 0u;
	while(i < 12u)
	{
		uint8_t n = 0u;
		//4u: RectAngleのEdge
		while(n < 4u)
		{
			//OBB上のエッジとRectAngle上のエッジでそれぞれの最近接点(RectAngle_ClosestPt, Box_ClosestPt)を計算
			float t, s, dSq;
			RB_Vec3f RectAngle_ClosestPt, Box_ClosestPt;
			//TODO 必要な回数を減らす
			//線分と線分の最近接点を計算する
			dSq = ClosestPt_SegmentSegment(&(RectAngleVertexs[n].StPos), &(RectAngleVertexs[n].EdPos), 
			&(BoxEdges[i].StPos), &(BoxEdges[i].EdPos), &t, &s, &RectAngle_ClosestPt, &Box_ClosestPt);

#if 0
			//RectAngle座標系ベースの位置に変換する
			RB_Vec3f Rel, LCS_Box_ClosestPt;
			RB_Vec3fSub(&Box_ClosestPt, &(RectAngleVertexs[0u].StPos), &Rel);
			ConvBoxAreaLocalPos(&Rel, RectAngleUnit, &LCS_Box_ClosestPt);

//====================================================================================
			//RectAngleをOBB と OBB上の球体 の当たり判定 
			if( (n == 0u) && (IsOverlapSphere_RectAngle(&LCS_Box_ClosestPt, Radius, RectAngleSize)) )
			{
				flag = true;
				break;
			}
#endif
//====================================================================================
			//OBBとRectAngle上の球体と当たり判定
			//求めた最近接点を中心とした球体の領域を作り、OBBと当たり判定を行う
			//当たりがあればその時点でエッジとの判定を終了
			if(IsOverlapSphere_OBB(&RectAngle_ClosestPt, Radius, BoxSize))
			{
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
	return ret;
}
#endif

RBSTATIC bool IsCollision_RoundRectAngle(uint32_t rectangle_id, uint32_t area_id)
{
	bool ret = false;

	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	SEGMENT_T RectAngleEdge[4u];
//RoundRectAngleの最新位置情報を取得
	RB_Vec3f RectAngleUnit[3u] = { 0.0f };
	RB_Vec3f RoundRectAngleSize = { 0.0f };
	//GetRoundRectAngleData(uint32_t rectangle_id, SEGMENT_T *EdgeData, RB_Vec3f *now_u, RB_Vec3f *RectAngleSize)
	GetRoundRectAngleData(rectangle_id, RectAngleEdge, RectAngleUnit, &RoundRectAngleSize);
	float Radius = RoundRectAngleSize.e[0u];

//ボックスエリアの情報を取得===========================================================
	OBJECT_T BoxObject = ObjectData[area_id];
	RB_Vec3f BoxUnit[3u];
	GetBoxAreaUnitVec3f(area_id, BoxUnit);

	BOX_T BoxData = BoxObject.BoxData;
	RB_Vec3f BoxSize = BoxData.BoxSize;
	bool BoxAreaType = BoxData.AreaType;
//======================================================================================

//======BoxAreaのタイプで内包判定, 衝突判定を実施する=========//
	RB_Vec3f LocalVertex[4u] = { 0.0f };
	//
	//P3----------P2
	// |          |
	// |          |
	//P0 -------- P1
	//

	//Box中心からの距離を計算
	//Box基準の座標系位置に変換
	for(uint8_t i = 0u; i < 4u; i++)
	{	
		RB_Vec3f Rel;
		RB_Vec3fSub(&(RectAngleEdge[i].StPos), &BoxObject.CenterPos, &Rel);
		ConvBoxAreaLocalPos(&Rel, BoxUnit, &LocalVertex[i]);
	}
	
	//WorkAreaの場合
	if(BoxAreaType)
	{
		for(uint8_t i = 0u; i < 4u; i++)
		{
			if(IsOutSideSphere_OBB(&LocalVertex[i], Radius, &BoxSize))
			{
				ret = true;
				break;
			}
		}
	}
	//BlockAreaの場合
	else
	{
		ret = CollDetRoundRectAngle_vs_OBB(rectangle_id, area_id);
	}

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
						//ret[j] += (uint8_t)IsCollision_Sphere(i, (j + ObjectBaseNum));
						break;

					case 2u:
						//ret[j] += (uint8_t)IsCollision_Capsule(i, (j + ObjectBaseNum));
						break;

					case 3u:
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