#ifndef RB_MATH_H
#define RB_MATH_H
#include "MainCommon.h"
#include "MainTypeDef.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// 3次元ベクトル
typedef struct
{
	float e[3];
}RB_Vec3f;

// 3x3行列ベクトル
typedef struct
{
	float e[3][3];
}RB_Mat3f;

//ded<==>rad
float Rad2Deg(float rad);
float Deg2Rad(float deg);

//Vector3f===========================================================
void RB_Vec3fCreate(RBCONST float x, RBCONST float y, RBCONST float z, RB_Vec3f *v);
float RB_Vec3fGetElem(RBCONST RB_Vec3f *v, RBCONST uint8_t i);
void RB_Vec3fSetElem(RB_Vec3f *v, RBCONST uint8_t i, RBCONST float x);

void RB_Vec3fAdd(RBCONST RB_Vec3f *v1, RBCONST RB_Vec3f *v2, RB_Vec3f *v_ans);
void RB_Vec3fSub(RBCONST RB_Vec3f *v1, RBCONST RB_Vec3f *v2, RB_Vec3f *v_ans);
bool RB_Vec3fMatch(RBCONST RB_Vec3f *v1, RBCONST RB_Vec3f *v2);
float RB_Vec3fDot(RBCONST RB_Vec3f *v1, RBCONST RB_Vec3f *v2);
void RB_Vec3fCross(RBCONST RB_Vec3f *v1, RBCONST RB_Vec3f *v2, RB_Vec3f *v_ans);

float RB_Vec3fNorm(RBCONST RB_Vec3f *v);
void RB_Vec3fNormalize(RBCONST RB_Vec3f *v, RB_Vec3f *v_ans);

//Matrix3f============================================================
void RB_Mat3fCreate(
	RBCONST float e11, RBCONST float e12, RBCONST float e13,
	RBCONST float e21, RBCONST float e22, RBCONST float e23,
	RBCONST float e31, RBCONST float e32, RBCONST float e33,
	RB_Mat3f *m
);
float RB_Mat3fGetElem(RBCONST RB_Mat3f *m, RBCONST uint8_t r, RBCONST uint8_t c);
void RB_Mat3fSetElem(RB_Mat3f *m, RBCONST uint8_t r, RBCONST uint8_t c, RBCONST float x);
void RB_MulMatVec3f(RBCONST RB_Mat3f *m, RBCONST RB_Vec3f *v, RB_Vec3f *mv_ans);
void RB_MulMatMat3f(RBCONST RB_Mat3f *m1, RBCONST RB_Mat3f *m2, RB_Mat3f *m_ans);
void RB_AxisRotateMat3f(RBCONST RB_Vec3f *v_axis, RBCONST float rad, RB_Mat3f *m_ans);

void RB_Mat3fTermOut(RBCONST char *str, RBCONST RB_Mat3f *m);
void RB_Vec3fTermOut(RBCONST char *str, RBCONST RB_Vec3f *v);

//上記関数のテスト
void UT_RB_Math(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* RB_MATH_H */
