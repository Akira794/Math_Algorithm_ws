#ifndef RB_MATH_H
#define RB_MATH_H
#include "rb_type.h"
#include "rb_common.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus*/

//3次元ベクトル (x,y,z) = (e[0u],e[1u],e[2u])
typedef struct
{
	float e[3u];
}RB_Vec3f;

typedef struct
{
	float e[3][3];
}RB_Mat3f;

typedef struct
{
	float e[4u];
}RB_Quaternion;

float RB_Vec3fCreate(RBCONST float x, RBCONST float y, RBCONST float z, RB_Vec3f *v);
float RB_Vec3fGetElem(RBCONST RB_Vec3f *v, RBCONST uint16_t i);
void RB_Vec3fSetElem(RB_Vec3f *v, RBCONST uint16_t i, RBCONST float x);

/*operation*/
/*3次元ベクトルの操作*/
void RB_Vec3fAdd(RBCONST RB_Vec3f *v1, RBCONST RB_Vec3f *v2, RB_Vec3f *v_ans);
void RB_Vec3fSub(RBCONST RB_Vec3f *v1, RBCONST RB_Vec3f *v2, RB_Vec3f *v_ans);
void RB_Vec3fMatch(RBCONST RB_Vec3f *v1, RBCONST RB_Vec3f *v2);

/*3x3行列*/
void RB_Mat3fCreate(
	RBCONST float e11, RBCONST float e12, RBCONST float e13,
	RBCONST float e21, RBCONST float e22, RBCONST float e23,
	RBCONST float e31, RBCONST float e32, RBCONST float e33,
	RBCONST *m
);

void RB_MulMatVec3f(RBCONST RB_Mat3f *m, RBCONST RB_Vec3f *v, RB_Vec3f *mv_ans);
void RB_MulMatMat3f(RBCONST RB_Mat3f *m1, RBCONST RB_Mat3f *m2, RB_Mat3f *m_ans);
void RB_Vec3fAxisRotMat3f(RBCONST RB_Vec3f *v, RBCONST float rad, RB_Mat3f *m_ans);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* RB_MATH_H */