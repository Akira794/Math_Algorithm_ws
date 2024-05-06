#include "CollisionDet.h"
#include "MainCommon.h"
#include "MainTypeDef.h"
#include "DbgCmd.h"
#include "RB_Math.h"
#include <math.h>

RBSTATIC uint32_t f_SegmentId = 0u;

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
	DbgCmd_SetSegment(f_SegmentId, &wcs_q, &(ObjectData[mon_id].CenterPos));

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
	DbgCmd_SetSegment(f_SegmentId, &(ObjectData[area_id].CenterPos), &d);
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
//	DbgCmd_SetSegment(f_SegmentId, &v_pos, wcs_pos);
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
	DbgCmd_SetSegment(f_SegmentId, &lcs_StPos, &lcs_EdPos);
#endif
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

	Dbg_LCSPos(11u, 1u);
	Dbg_LCSPos(12u, 1u);

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