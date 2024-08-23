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

#if 0
	RB_Vec3fCreate(RB_Vec3fDot(&d, &u[0u], RB_Vec3fDot(&d, &u[1u], RB_Vec3fDot(&d, &u[2u], &local_dist);
	DbgCmd_SetVec3f("Vec3f_Q", &local_dist);
#endif

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
//TODO 複数の当たり判定に対応する必要あり
RBSTATIC bool Sphere_vs_OBB(uint32_t area_id, uint32_t mon_id)
{
	bool ret = false;
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	SSV_T Sphere = ObjectData[mon_id].Sphere;
	float Radius = Sphere.Radius;

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

	*radius = Capsule.Radius;
	RB_Vec3f rel_CentralAxis;
	RB_Vec3f wcs_edpos;

	//姿勢を中心軸に反映
	RB_MulMatVec3f( &(ObjectData[mon_id].CenterRot), &(Capsule.EndPos), &rel_CentralAxis);

	//中心軸のEndPosをWCSに反映
	RB_Vec3fAdd(&(ObjectData[mon_id].CenterPos), &rel_CentralAxis, &wcs_edpos);

	*StPos = ObjectData[mon_id].CenterPos;
	*EdPos = wcs_edpos;

}

RBSTATIC bool ClosestPt_OBBCenterPos_Segment(uint32_t area_id, uint32_t mon_id)
{
	bool ret = false;

	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	float t, Radius;
	RB_Vec3f d;
	RB_Vec3f CapsuleStPos, CapsuleEdPos;
	GetCapsuleData(mon_id, &CapsuleStPos, &CapsuleEdPos, &Radius);

	ClosestPtPointSegment( &(ObjectData[area_id].CenterPos), &CapsuleStPos, &CapsuleEdPos, &t, &d);

#if 1
	f_SegmentId++;

	//Box中心からカプセル上の点に最も近い点までのベクトルを描画
	DbgCmd_SetSegment(f_SegmentId, 3u, &(ObjectData[area_id].CenterPos), &d);
#endif
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

			//tが[0, 1]の中にあれば終了。そうでなければtをクランプ、sをtの新しい値に対して以下を用いて再計算

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


//=========================================================================================

void CollisionDet_Init(void)
{

}

void CollisionDet_PreStartProc(void)
{

}


void CollisionDet_Cycle(void)
{
	bool colldet_result[SEGMENT_MAXID] = { 1u };

	if(CollDetCapsule_vs_Capsule(1u, 2u))
	{
		printf("<INFO>: Hit!\n");
	}

//	Dbg_LCSPos(11u, 1u);
//	Dbg_LCSPos(12u, 1u);

//	ClosestPt_OBBCenterPos_Segment(11u, 1u);
//	ClosestPt_OBBCenterPos_Segment(12u, 1u);

#if 0
	DbgCmd_SetOverlapStatus(11u, (Sphere_vs_OBB(11u, 1u) || Sphere_vs_OBB(11u, 2u)) );
	DbgCmd_SetOverlapStatus(12u, (Sphere_vs_OBB(12u, 1u) || Sphere_vs_OBB(12u, 2u)) );
#endif

	f_SegmentId = 0u;
}

void CollisionDet_Destroy(void)
{

}