#include "MainCommon.h"
#include "MainTypeDef.h"
#include "RB_Math.h"

RBSTATIC void UT_Name(RBCONST char *str, uint32_t num);
RBSTATIC uint32_t f_UT_Mi_Num = 0u;
RBSTATIC RBCONST RB_Vec3f f_RB_Vec3fzero = { { 0.0f, 0.0f, 0.0f} };
RBSTATIC RBCONST RB_Mat3f f_RB_Mat3fident = { { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } } };
RBSTATIC RBCONST RB_Mat3f f_RB_Mat3zero = { { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } } };
//テスト用補助関数====================================

RBSTATIC void UT_Name(RBCONST char *str, uint32_t num)
{
	static uint32_t pre_Manum = 0u;
	if(num == 1u)
	{
		f_UT_Mi_Num++;        
	}

	printf("\n==============================================");
	printf("\n [UT-%03u-%02u] \n[%s]\n", f_UT_Mi_Num, num, str);

	pre_Manum = num;
}

RBSTATIC void UT_Vec3fFunctions(void)
{
	RB_Vec3f v1 = f_RB_Vec3fzero;
	RB_Vec3f v2 = f_RB_Vec3fzero;
	RB_Vec3f v_ans1 = f_RB_Vec3fzero;
	RB_Vec3f v_ans2 = f_RB_Vec3fzero;
	RB_Vec3f v_ans3 = f_RB_Vec3fzero;

//==========================================
	UT_Name("RB_Vec3fTermOut(RBCONST char *str, RBCONST RB_Vec3f *v);", 1u);
	RB_Vec3fTermOut("v1", &v1);

	UT_Name("RB_Vec3fCreate(RBCONST float x, RBCONST float y, RBCONST float z, RB_Vec3f *v);", 1u);
	RB_Vec3fCreate(1.0f, 2.0f, 3.0f, &v1);
	RB_Vec3fTermOut("v1", &v1);

	UT_Name("RB_Vec3fCreate(RBCONST float x, RBCONST float y, RBCONST float z, RB_Vec3f *v);", 2u);
	RB_Vec3fCreate(-1.0f, -4.0f, -9.0f, &v1);
	RB_Vec3fTermOut("v1", &v1);

	for(uint8_t i = 0u; i < 3u; i++)
	{
		UT_Name("RB_Vec3fGetElem(RBCONST RB_Vec3f *v, RBCONST uint8_t i);", i+1u);
		printf("RB_Vec3fGetElem(&v1, %u)=%.4f\n",i, RB_Vec3fGetElem(&v1, i));
	}

	UT_Name("RB_Vec3fSetElem(RB_Vec3f *v, RBCONST uint8_t i, RBCONST float x);", 1u);
	printf("RB_Vec3fSetElem(&v1, 0u, %.3f)\n", 5.0f);
	RB_Vec3fSetElem(&v1, 0u, 5.0f);
	RB_Vec3fTermOut("v1", &v1);

	UT_Name("RB_Vec3fSetElem(RB_Vec3f *v, RBCONST uint8_t i, RBCONST float x);", 2u);
	printf("RB_Vec3fSetElem(&v1, 1u, %.3f)\n", 10.0f);
	RB_Vec3fSetElem(&v1, 1u, 10.0f);
	RB_Vec3fTermOut("v1", &v1);

	UT_Name("RB_Vec3fSetElem(RB_Vec3f *v, RBCONST uint8_t i, RBCONST float x);", 3u);
	printf("RB_Vec3fSetElem(&v1, 2u, %.3f)\n", 15.0f);
	RB_Vec3fSetElem(&v1, 2u, 15.0f);
	RB_Vec3fTermOut("v1", &v1);

	UT_Name("RB_Vec3fAdd(RBCONST RB_Vec3f *v1, RBCONST RB_Vec3f *v2, RB_Vec3f *v_ans);", 1u);

	RB_Vec3fCreate(10.0f, 20.0f, 30.0f, &v1);
	RB_Vec3fCreate(-1.0f, -4.0f, -9.0f, &v2);

	RB_Vec3fTermOut("v1", &v1);
	RB_Vec3fTermOut("v2", &v2);

	RB_Vec3fAdd(&v1, &v2, &v_ans1);
	RB_Vec3fTermOut("v_ans1", &v_ans1);
	v_ans1 = f_RB_Vec3fzero;

	UT_Name("RB_Vec3fSub(RBCONST RB_Vec3f *v1, RBCONST RB_Vec3f *v2, RB_Vec3f *v_ans);", 1u);

	RB_Vec3fCreate(10.0f, 20.0f, 30.0f, &v1);
	RB_Vec3fCreate(-1.0f, -4.0f, -9.0f, &v2);

	RB_Vec3fTermOut("v1", &v1);
	RB_Vec3fTermOut("v2", &v2);

	RB_Vec3fSub(&v1, &v2, &v_ans1);
	RB_Vec3fTermOut("v_ans1", &v_ans1);
	v_ans1 = f_RB_Vec3fzero;

	UT_Name("RB_Vec3fSub(RBCONST RB_Vec3f *v1, RBCONST RB_Vec3f *v2, RB_Vec3f *v_ans);", 1u);

	RB_Vec3fCreate(10.0f, 20.0f, 30.0f, &v2);
	RB_Vec3fCreate(-1.0f, -4.0f, -9.0f, &v1);

	RB_Vec3fTermOut("v1", &v1);
	RB_Vec3fTermOut("v2", &v2);

	RB_Vec3fSub(&v1, &v2, &v_ans1);
	RB_Vec3fTermOut("v_ans1", &v_ans1);
	v_ans1 = f_RB_Vec3fzero;

	UT_Name("RB_Vec3fMatch(RBCONST RB_Vec3f *v1, RBCONST RB_Vec3f *v2);", 1u);
	RB_Vec3fCreate(10.0f, 20.0f, 30.0f, &v2);
	RB_Vec3fCreate(-1.0f, -4.0f, -9.0f, &v1);

	RB_Vec3fTermOut("v1", &v1);
	RB_Vec3fTermOut("v2", &v2);

	printf("RB_Vec3fMatch(&v1, &v2)=%u\n",RB_Vec3fMatch(&v1, &v2));

	v1 = v2;

	RB_Vec3fTermOut("v1", &v1);
	printf("RB_Vec3fMatch(&v1, &v2)=%u\n",RB_Vec3fMatch(&v1, &v2));

	UT_Name("RB_Vec3fDot(RBCONST RB_Vec3f *v1, RBCONST RB_Vec3f *v2);", 1u);
	RB_Vec3fCreate(1.0f, 2.0f, 3.0f, &v1);
	RB_Vec3fCreate(-2.0f, -4.0f, -8.0f, &v2);

	RB_Vec3fTermOut("v1", &v1);
	RB_Vec3fTermOut("v2", &v2);

	RB_Vec3fDot(&v1, &v2);
	printf("RB_Vec3fDot(&v1, &v2)=%.3f",RB_Vec3fDot(&v1, &v2));

	UT_Name("RB_Vec3fCross(RBCONST RB_Vec3f *v1, RBCONST RB_Vec3f *v2, RB_Vec3f *v_ans);", 1u);
	RB_Vec3fCreate(1.0f, 2.0f, 3.0f, &v1);
	RB_Vec3fCreate(-2.0f, -4.0f, -8.0f, &v2);

	RB_Vec3fTermOut("v1", &v1);
	RB_Vec3fTermOut("v2", &v2);

	RB_Vec3fCross(&v1, &v2, &v_ans1);
	RB_Vec3fTermOut("v_ans1", &v_ans1);
	v_ans1 = f_RB_Vec3fzero;

	UT_Name("RB_Vec3fNorm(RBCONST RB_Vec3f *v);", 1u);
	RB_Vec3fCreate(1.0f, 0.0f, 0.0f, &v1);
	RB_Vec3fTermOut("v1", &v1);
	printf("RB_Vec3fNorm(&v1)=%.3f\n", RB_Vec3fNorm(&v1));

	UT_Name("RB_Vec3fNorm(RBCONST RB_Vec3f *v);", 2u);
	RB_Vec3fCreate(1.0f, 1.0f, 1.0f, &v1);
	RB_Vec3fTermOut("v1", &v1);
	printf("RB_Vec3fNorm(&v1)=%.3f\n", RB_Vec3fNorm(&v1));

	UT_Name("RB_Vec3fNormalize(RBCONST RB_Vec3f *v, RB_Vec3f *v_ans);", 1u);
	RB_Vec3fCreate(2.0f, 3.0f, 5.0f, &v1);
	RB_Vec3fTermOut("v1", &v1);
	RB_Vec3fNormalize(&v1, &v_ans1);
	RB_Vec3fTermOut("v_ans1", &v_ans1);
	printf("Check: RB_Vec3fNorm(&v_ans1)=%.3f\n", RB_Vec3fNorm(&v_ans1));
	v_ans1 = f_RB_Vec3fzero;
}

RBSTATIC void UT_Mat3fFunctions(void)
{
	RB_Vec3f v1 = f_RB_Vec3fzero;
	RB_Vec3f v_ans1 = f_RB_Vec3fzero;

	RB_Mat3f m1 = f_RB_Mat3zero;
	RB_Mat3f m2 = f_RB_Mat3zero;
	RB_Mat3f m3 = f_RB_Mat3zero;
	RB_Mat3f m_ans1 = f_RB_Mat3zero;
	RB_Mat3f m_ans2 = f_RB_Mat3zero;
	RB_Mat3f m_ans3 = f_RB_Mat3zero;

//==========================================
	UT_Name("RB_Mat3fTermOut(RBCONST char *str, RBCONST RB_Mat3f *m);", 1u);
	RB_Mat3fTermOut("m1", &m1);

	UT_Name("void RB_Mat3fCreate(RBCONST float e11, RBCONST float e12, RBCONST float e13,RBCONST float e21, RBCONST float e22, RBCONST float e23,RBCONST float e31, RBCONST float e32, RBCONST float e33,RB_Mat3f *m);", 1u);
	RB_Mat3fCreate(
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f, &m1);

	RB_Mat3fTermOut("m1", &m1);

	UT_Name("void RB_Mat3fCreate(RBCONST float e11, RBCONST float e12, RBCONST float e13,RBCONST float e21, RBCONST float e22, RBCONST float e23,RBCONST float e31, RBCONST float e32, RBCONST float e33,RB_Mat3f *m);", 2u);
	RB_Mat3fCreate(
		1.0f, 2.0f, 3.0f,
		1.0f, 4.0f, 9.0f,
		-1.0f, -4.0f, -9.0f, &m1);

	RB_Mat3fTermOut("m1", &m1);

	RB_Mat3fCreate(
		1.0f, 2.0f, 3.0f,
		4.0f, 5.0f, 6.0f,
		7.0f, 8.0f, 9.0f, &m1);

	RB_Mat3fTermOut("m1", &m1);
	uint8_t count = 0u;
	for (uint8_t i = 0u; i < 3u; i++)
	{
		///要素数分繰り返す
		for (uint8_t j = 0u; j < 3u; j++)
		{
			UT_Name("float RB_Mat3fGetElem(RBCONST RB_Mat3f *m, RBCONST uint8_t r, RBCONST uint8_t c);", count+1u);
			printf("m1->[%u][%u]=%.3f ", i, j, RB_Mat3fGetElem(&m1, i, j));
			printf("\n");
			count++;
		}
	}
	count = 0u;

	for (uint8_t i = 0u; i < 3u; i++)
	{
		///要素数分繰り返す
		for (uint8_t j = 0u; j < 3u; j++)
		{
			UT_Name("void RB_Mat3fSetElem(RB_Mat3f *m, RBCONST uint8_t r, RBCONST uint8_t c, RBCONST float x);", 1u);
			printf("m1:[%u][%u] <==%.3f ", i, j, -((float)(count + 10u)));
			RB_Mat3fSetElem(&m1, i, j, -((float)(count + 10u)));
			printf("\n");
			count++;
		}
		RB_Mat3fTermOut("m1", &m1);
	}	
	count = 0u;

	UT_Name("void RB_MulMatVec3f(RBCONST RB_Mat3f *m, RBCONST RB_Vec3f *v, RB_Vec3f *mv_ans);", 1u);
	RB_Vec3fCreate(1.0f, 4.0f, 9.0f, &v1);
	RB_Vec3fTermOut("v1", &v1);
	RB_Mat3fCreate(
		1.0f, 2.0f, 3.0f,
		4.0f, 5.0f, 6.0f,
		7.0f, 8.0f, 9.0f, &m1);
	RB_Mat3fTermOut("m1", &m1);

	RB_MulMatVec3f(&m1, &v1, &v_ans1);
	RB_Vec3fTermOut("v_ans1", &v_ans1);

	UT_Name("void RB_MulMatMat3f(RBCONST RB_Mat3f *m1, RBCONST RB_Mat3f *m2, RB_Mat3f *m_ans);", 1u);
	RB_Mat3fCreate(
		1.0f, 2.0f, 3.0f,
		4.0f, 5.0f, 6.0f,
		7.0f, 8.0f, 9.0f, &m1);
	RB_Mat3fTermOut("m1", &m1);

	RB_Mat3fCreate(
		1.0f, 4.0f, 7.0f,
		2.0f, 5.0f, 8.0f,
		3.0f, 6.0f, 9.0f, &m2);
	RB_Mat3fTermOut("m2", &m2);

	RB_MulMatMat3f(&m1, &m2, &m_ans1);
	RB_Mat3fTermOut("m_ans1", &m_ans1);

	UT_Name("void RB_AxisRotateMat3f(RBCONST RB_Vec3f *v_axis, RBCONST float rad, RB_Mat3f *m_ans);", 1u);

	RB_Vec3fCreate(0.0f, 0.0f, 0.0f, &v1);
	RB_Vec3fTermOut("v1", &v1);

	RB_AxisRotateMat3f(&v1, 75.0f, &m_ans1);
	RB_Mat3fTermOut("m_ans1", &m_ans1);

	UT_Name("void RB_AxisRotateMat3f(RBCONST RB_Vec3f *v_axis, RBCONST float rad, RB_Mat3f *m_ans);", 2u);

	RB_Vec3fCreate(1.0f, 0.0f, 0.0f, &v1);
	RB_Vec3fTermOut("v1", &v1);

	RB_AxisRotateMat3f(&v1, 90.0f, &m_ans1);
	RB_Mat3fTermOut("m_ans1", &m_ans1);

	UT_Name("void RB_AxisRotateMat3f(RBCONST RB_Vec3f *v_axis, RBCONST float rad, RB_Mat3f *m_ans);", 3u);

	RB_Vec3fCreate(0.0f, 1.0f, 0.0f, &v1);
	RB_Vec3fTermOut("v1", &v1);

	RB_AxisRotateMat3f(&v1, 90.0f, &m_ans1);
	RB_Mat3fTermOut("m_ans1", &m_ans1);

	UT_Name("void RB_AxisRotateMat3f(RBCONST RB_Vec3f *v_axis, RBCONST float rad, RB_Mat3f *m_ans);", 4u);

	RB_Vec3fCreate(0.0f, 0.0f, 1.0f, &v1);
	RB_Vec3fTermOut("v1", &v1);

	RB_AxisRotateMat3f(&v1, 90.0f, &m_ans1);
	RB_Mat3fTermOut("m_ans1", &m_ans1);

}

void UT_RB_Math(void)
{
	UT_Vec3fFunctions();
	UT_Mat3fFunctions();
}
