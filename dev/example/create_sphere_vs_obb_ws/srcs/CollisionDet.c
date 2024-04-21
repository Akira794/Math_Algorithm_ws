#include "CollisionDet.h"
#include "MainCommon.h"
#include "MainTypeDef.h"
#include "DbgCmd.h"
#include "RB_Math.h"

//pointから見たOBBの最近接点を取得
RBSTATIC void ClosestPtPointOBB( RB_Vec3f *p, OBJECT_T *Object, RB_Vec3f *q)
{
	RB_Mat3f *m = &Object->CenterRot;
	BOX_T *Box_obj = &Object->Box;
	RB_Vec3f *l = &Box_obj->BoxSize;

	RB_Vec3f u[3u] = { 0.0f };
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
	*q = Object->CenterPos;
#if 1
	for(uint8_t i = 0u; i < 3u; i++)
	{
		dist = RB_Vec3fDot(&d, &u[i]);

		if(dist > l->e[i])
		{
			dist = (l->e[i]);
		}
		if(dist < -(l->e[i]))
		{
			dist = -(l->e[i]);
		}
		q->e[0u] += dist * u[i].e[0u];
		q->e[1u] += dist * u[i].e[1u];
		q->e[2u] += dist * u[i].e[2u];
	}
#endif
}

//Sphere vs OBB 
RBSTATIC bool Sphere_OBB(uint32_t area_id, uint32_t mon_id)
{
	bool ret = false;
	OBJECT_T ObjectData[OBJECT_MAXID];
	DbgCmd_GetPoseCmd(ObjectData);

	SSV_T Sphere = ObjectData[mon_id].Sphere;
	float Radius = Sphere.Radius;

	RB_Vec3f LocalPos;
	RB_Vec3f v;

	ClosestPtPointOBB( &(ObjectData[mon_id].CenterPos), &ObjectData[area_id], &LocalPos);

	//DbgCmd_SetVec3f("nearest neighbor point: ", &LocalPos);

	RB_Vec3fSub(&LocalPos, &(ObjectData[mon_id].CenterPos), &v);

	if(RB_Vec3fDot(&v,&v) <= (Radius * Radius))
	{
		ret = true;
	}

	return ret;
}

void CollisionDet_Init(void)
{

}

void CollisionDet_PreStartProc(void)
{

}


void CollisionDet_Cycle(void)
{
	DbgCmd_SetOverlapStatus(11u, (Sphere_OBB(11u, 1u) || Sphere_OBB(11u, 2u)) );
	DbgCmd_SetOverlapStatus(12u, (Sphere_OBB(12u, 1u) || Sphere_OBB(12u, 2u)) );

}

void CollisionDet_Destroy(void)
{

}