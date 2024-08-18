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

//WCS(p)から見たOBBの最近接点を取得
//WCS(p)に対してOBB上(または内部)にあるWCS(p)の最近接点qを返す
RBSTATIC void ClosestPtPointOBB( RBCONST RB_Vec3f *p, RBCONST OBJECT_T *Object, RB_Vec3f *q)
{
	RBCONST RB_Mat3f *m = &Object->CenterRot;
	RBCONST BOX_T *Box_obj = &Object->Box;
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

//Sphere vs OBB
//Capsule用
RBSTATIC bool CollDetSphere_vs_OBB_Unit(RB_Vec3f *CPos, float Radius, uint32_t area_id, uint8_t ColorId)
{
	bool ret = false;
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	RB_Vec3f wcs_q;
	RB_Vec3f rel_q;

	//WCS(p)に対してOBB上(または内部)にあるWCS(p)の最近接点qを返す
	ClosestPtPointOBB( CPos, &ObjectData[area_id], &wcs_q);

#if 0
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

#if 1
//Sphere vs OBB
//TODO 複数の当たり判定に対応する必要あり
RBSTATIC bool CollDetSphere_vs_OBB(uint32_t mon_id, uint32_t area_id)
{
	bool ret = false;
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	SSV_T Sphere = ObjectData[mon_id].Sphere;
	float Radius = RB_Vec3fGetElem( &(Sphere.SSV_Size), 0u);

	RB_Vec3f wcs_q;
	RB_Vec3f rel_q;

	//WCS(p)に対してOBB上(または内部)にあるWCS(p)の最近接点qを返す
	ClosestPtPointOBB( &(ObjectData[mon_id].CenterPos), &ObjectData[area_id], &wcs_q);

#if 1
	//最近接点から球体中心までのベクトルを描画
	f_SegmentId++;
	DbgCmd_SetSegment(f_SegmentId, 3u, &wcs_q, &(ObjectData[mon_id].CenterPos));

	//DbgCmd_SetVec3f("nearest neighbor point: ", &wcs_q);
#endif

	//最近接点wcs_qと相対距離ベクトルrel_qを計算
	RB_Vec3fSub(&wcs_q, &(ObjectData[mon_id].CenterPos), &rel_q);

	//相対距離ベクトル rel_q を2乗し、大きさを求める
	//それが半径の2乗以下ならOBBに球体が接触していると判定
	if(RB_Vec3fDot(&rel_q,&rel_q) <= (Radius * Radius))
	{
		ret = true;
	}

	return ret;
}
#endif

RBSTATIC void ClosestPtPointSegment(RBCONST RB_Vec3f *CentralPos, RBCONST RB_Vec3f *StPos, RBCONST RB_Vec3f *EdPos, float *t, RB_Vec3f *d)
{
	RB_Vec3f rel_ab;
	RB_Vec3fSub(EdPos, StPos, &rel_ab);

	//ad上にcを投影, しかしDot(ab, ab)による除算は延期

	RB_Vec3f rel_ca;
	RB_Vec3fSub(CentralPos, StPos, &rel_ca);
	float t_ret = (RB_Vec3fDot(&rel_ca, &rel_ab));

	if( t_ret <= 0.0f)
	{
		//cは範囲[a,b]の外側, aの側に投影, aまでクランプ
		t_ret = 0.0f;
		*d = *StPos;

	}
	else
	{
		//denom = ||ab||^2なので常に非負の値
		float denom = RB_Vec3fDot(&rel_ab, &rel_ab);

		if(t_ret >= denom)
		{
			//cは範囲[a,b]の外側、bの側に射影され、bまでクランプ
			t_ret = 1.0f;
			*d = *EdPos;
		}
		else
		{
			//cは範囲[a,b]の内側に射影され、このときまで除算は延期
			t_ret = t_ret / denom;

			//dの値を計算 (d) = (StPos) + t_ret * (rel_ab)
			for(uint32_t i = 0u; i < 3u; i++)
			{
				d->e[i] = StPos->e[i] + (t_ret * rel_ab.e[i]);
			}
		}
	}

	*t = t_ret;
}

//カプセルの情報を取得
RBSTATIC void GetCapsuleData(uint32_t mon_id, RB_Vec3f *StPos, RB_Vec3f *EdPos, float *radius)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	uint8_t type = ObjectData[mon_id].ShapeType;
	RBAssert(type == 2u);

	SSV_T Capsule = ObjectData[mon_id].Capsule;

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

//Calculate position in Box coordinate system
RBSTATIC void CalcPosInLCS(uint32_t area_id, RBCONST RB_Vec3f *wcs_pos, RB_Vec3f *v_ans)
{
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	BOX_T Box_obj = ObjectData[area_id].Box;
	RB_Vec3f v_pos = ObjectData[area_id].CenterPos;
	RB_Vec3f v_xyz_size = Box_obj.BoxSize;
	RB_Mat3f m_rot = ObjectData[area_id].CenterRot;

	RB_Vec3f rel_pos;
	//WCS基準の位置wcs_posをLCSからの相対位置に変換
	RB_Vec3fSub(wcs_pos, &v_pos, &rel_pos);

	RB_Vec3f u[3u] = { 0.0f };

	//Boxの姿勢行列からBoxのローカル座標系の単位ベクトルを取得
	for(uint8_t i = 0u; i < 3u; i++)
	{
		//Areaの方向ベクトル(x)を求める
		u[i].e[0u] = RB_Mat3fGetElem(&m_rot, 0u, i);
		u[i].e[1u] = RB_Mat3fGetElem(&m_rot, 1u, i);
		u[i].e[2u] = RB_Mat3fGetElem(&m_rot, 2u, i);
	}

	//WCS基準の位置をLCS基準に変換
	for(uint8_t i = 0u; i < 3u; i++)
	{
		v_ans->e[i] = RB_Vec3fDot(&rel_pos,&u[i]);
	}


//	f_SegmentId++;
//	DbgCmd_SetSegment(f_SegmentId, 0u, &v_pos, wcs_pos);
}

//確認: BoxのLCSから見たカプセルの位置を描画
RBSTATIC void Dbg_LCSPos(uint32_t area_id, uint32_t mon_id)
{
	float t, Radius;
	RB_Vec3f CapsuleStPos, CapsuleEdPos;
	RB_Vec3f lcs_StPos, lcs_EdPos;

	//監視対象であるカプセルの現在位置(始点・終点・半径)を取得
	GetCapsuleData(mon_id, &CapsuleStPos, &CapsuleEdPos, &Radius);

	//area_idで指定したBoxのLCS(ローカル座標系)から見たカプセルの始点・終点位置を計算
	CalcPosInLCS(area_id, &CapsuleStPos, &lcs_StPos);
	CalcPosInLCS(area_id, &CapsuleEdPos, &lcs_EdPos);


#if 1
	DbgCmd_SetVec3f("lcs_StPos", &lcs_StPos);

	f_SegmentId++;
	//LCSから見たカプセルの始点・終点を描画
	DbgCmd_SetSegment(f_SegmentId, 1u, &lcs_StPos, &lcs_EdPos);
#endif
}

//線分と線分の最短距離
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
	RB_Vec3f *c1, *c2;

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

//カプセル vs カプセル 当たり判定
RBSTATIC bool CollDetCapsule_vs_Capsule(uint32_t capsule_1, uint32_t capsule_2)
{
	float R1, R2;
	RB_Vec3f C1St, C2St, C1Ed, C2Ed;
	
	GetCapsuleData(capsule_1, &C1St, &C1Ed, &R1);
	GetCapsuleData(capsule_2, &C2St, &C2Ed, &R2);

	float t1, t2;
	RB_Vec3f p1, p2;
	float dSq = ClosestPt_SegmentSegment(&C1St, &C1Ed, &C2St, &C2Ed, &t1, &t2, &p1, &p2);

#if 1
	//最近接点から球体中心までのベクトルを描画
	f_SegmentId++;
	DbgCmd_SetSegment( f_SegmentId, 1u, &p1, &p2);

#endif

	bool ret = false;
	if( dSq <= ((R1 + R2) * (R1 + R2)))
	{
		ret = true;
	}
	return ret;
}

//直方体のエッジを取得
RBSTATIC void GetBoxEdges(uint8_t area_id, RB_Vec3f *Edge_St, RB_Vec3f *Edge_Ed)
{

	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);
	RBCONST BOX_T Box_obj = ObjectData[area_id].Box;
	RB_Vec3f OBB_BoxSize = Box_obj.BoxSize;

	float lx = RB_Vec3fGetElem(&OBB_BoxSize, 0u);
	float ly = RB_Vec3fGetElem(&OBB_BoxSize, 1u);
	float lz = RB_Vec3fGetElem(&OBB_BoxSize, 2u);

	RB_Vec3f BoxInitArray[8u];
	RB_Vec3fCreate( -lx, -ly, -lz, &(BoxInitArray[0u]));//A0
	RB_Vec3fCreate( -lx,  ly, -lz, &(BoxInitArray[1u]));//B1
	RB_Vec3fCreate(  lx,  ly, -lz, &(BoxInitArray[2u]));//C2
	RB_Vec3fCreate(  lx, -ly, -lz, &(BoxInitArray[3u]));//D3
	RB_Vec3fCreate( -lx, -ly,  lz, &(BoxInitArray[4u]));//E4
	RB_Vec3fCreate( -lx,  ly,  lz, &(BoxInitArray[5u]));//F5
	RB_Vec3fCreate(  lx,  ly,  lz, &(BoxInitArray[6u]));//G6
	RB_Vec3fCreate(  lx, -ly,  lz, &(BoxInitArray[7u]));//H7

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


RBSTATIC void GetMonIdClosestPt(RB_Vec3f *Segment_St, RB_Vec3f *Segment_Ed, RB_Vec3f *StPos, RB_Vec3f *EdPos, RB_Vec3f *p1, RB_Vec3f *p2)
{
	float t1, t2, dSq;

	dSq = ClosestPt_SegmentSegment(Segment_St, Segment_Ed, StPos, EdPos, &t1, &t2, p1, p2);
}


//カプセル vs OBB 当たり判定
RBSTATIC bool CollDetCapsule_vs_OBB(uint32_t capsule_id, uint32_t area_id)
{
	bool ret = false;
	float R1;
	RB_Vec3f Cp_St, Cp_Ed;
	
	bool skip_f = true;

	//カプセルの情報を取得
	GetCapsuleData(capsule_id, &Cp_St, &Cp_Ed, &R1);

	//直方体のエッジ(12本)とカプセルの円柱部分(線分)との最近接点を計算する

	//Boxのサイズを取得
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);
	RBCONST BOX_T Box_obj = ObjectData[area_id].Box;
	RB_Vec3f l = Box_obj.BoxSize;
	RB_Vec3f BCpos = ObjectData[area_id].CenterPos;

	//ポリゴンの頂点を指定
	uint8_t PolygonVertex = 4u;
	uint8_t ObjectEdgeNum = 3 * PolygonVertex;

	RB_Vec3f EdgeSt[ObjectEdgeNum];
	RB_Vec3f EdgeEd[ObjectEdgeNum];

	GetBoxEdges(area_id, EdgeSt, EdgeEd);

	uint8_t i = 0u;
	uint8_t colorId = 1u;

	//エッジ毎とのカプセル側との最近接点を求める
	//求めた最近接点を中心とした球体の領域を作り、OBBと当たり判定を行う
	while(i < ObjectEdgeNum)
	{
		float t1, t2;
		RB_Vec3f p1, p2;
		//エッジ[i]とのカプセル内線分でそれぞれの最近接点を求める

		GetMonIdClosestPt(&Cp_St, &Cp_Ed,&EdgeSt[i], &EdgeEd[i], &p1, &p2);
#if 0
		//デバッグ用
			f_SegmentId++;
			DbgCmd_SetSegment( f_SegmentId, 2u, &p1, &p2);

#endif

	//求めた最近接点を中心とした球体の領域を作り、OBBと当たり判定を行う
	//当たりがあればその時点でエッジとの判定を終了
		if( (CollDetSphere_vs_OBB_Unit(&p1, R1, area_id, colorId)))
		{
			ret = true;
			break;
		}
			//次のエッジへ進む
		i++;
	}
	return ret;
}

#if 1
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
#endif

RBSTATIC void ClosestPtPointRectAngle(RB_Vec3f *Vertex, RB_Vec3f *CPos, RB_Vec3f *U_Normal, RB_Vec3f *Ans)
{
    //CPos->Vertexベクトル
    RB_Vec3f CPos_Vertex;

	RB_Vec3fSub(Vertex, CPos, &CPos_Vertex); 

    //法線U_NormalとCPos->Vertexを内積
    //法線の順方向に点Vertexがあればd > 0、 逆方向だとd < 0
	float d = RB_Vec3fDot(U_Normal, &CPos_Vertex);

    //内積値から平面上の最近点を求める

	for(uint8_t i = 0u; i < 3u; i++)
	{
		Ans->e[i] = Vertex->e[i] - ( (U_Normal->e[i]) * d );
	}
}

//丸い長方形 vs OBB 当たり判定
//TODO 処理を関数の関数に分ける必要あり
RBSTATIC bool CollDetRoundRectAngle_vs_OBB(uint32_t rectangle_id, uint32_t area_id)
{
	bool ret = false;
	bool skip_f = true;

	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	SSV_T RoundRectAngle_obj = ObjectData[rectangle_id].RoundRectAngle;

	RB_Vec3f CenterPos = ObjectData[rectangle_id].CenterPos;
	RB_Mat3f CenterRot = ObjectData[rectangle_id].CenterRot;

	RB_Vec3f u[3u] = { 0.0f };
	RB_Vec3f now_u[3u] = { 0.0f };

	u[0u] = RoundRectAngle_obj.Unit_Rel;
	u[1u] = RoundRectAngle_obj.Unit_Width;
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
	for(uint8_t i = 0u; i < 4u; i++)
	{
		f_SegmentId++;
		DbgCmd_SetSegment( f_SegmentId, 2u, &RectAngle_Edge_St[i], &RectAngle_Edge_Ed[i]);
	}
#endif

	//直方体のエッジ(12本)とカプセルの円柱部分(線分)との最近接点を計算する

	//Boxのサイズを取得
	RBCONST BOX_T Box_obj = ObjectData[area_id].Box;
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

	RectAngle_obj.Box.BoxSize = RectAngleSize;

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
#if 1
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

#if 1
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

			//衝突と判定したら即OBBエッジのwhileループを終了
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

//=========================================================================================

void CollisionDet_Init(void)
{

}

void CollisionDet_PreStartProc(void)
{

}


void CollisionDet_Cycle(void)
{
	uint8_t ret[OBJECT_MAXID] = { 0 };
	bool result[OBJECT_MAXID] = { 1 };

	//OBJECT_T ObjectData;
	//GetRoundRectAngleData(1u, &ObjectData);

#if 1
	for(uint8_t i = 1u; i < 5u; i++)
	{
		ret[0u] += (uint8_t)CollDetRoundRectAngle_vs_OBB(i, 11u);
		ret[1u] += (uint8_t)CollDetRoundRectAngle_vs_OBB(i, 12u);
	}

	uint8_t idx_base = 11u;

	for(uint8_t n = 0u; n <2u; n++)
	{
		result[n] = (ret[n] == 0u) ? false : true;
		idx_base += n;
		DbgCmd_SetOverlapStatus(idx_base, result[n]);
	}
#endif

#if 0
	for(uint8_t i = 1u; i < 2u; i++)
	{
		ret[0u] += (uint8_t)CollDetCapsule_vs_OBB(i, 11u);
		ret[1u] += (uint8_t)CollDetCapsule_vs_OBB(i, 12u);
	}

	uint8_t idx_base = 11u;

	for(uint8_t n = 0u; n <2u; n++)
	{
		result[n] = (ret[n] == 0u) ? false : true;
		idx_base += n;
		DbgCmd_SetOverlapStatus(idx_base, result[n]);
	}
#endif
	f_SegmentId = 0u;
}

void CollisionDet_Destroy(void)
{

}