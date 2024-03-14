#include "MainCommon.h"
#include "MainTypeDef.h"
#include "RB_Math.h"
#include <math.h>
#define RB_PI 3.14159265f

RBSTATIC RBCONST RB_Vec3f f_RB_Vec3fzero = { { 0.0f, 0.0f, 0.0f} };
RBSTATIC RBCONST RB_Mat3f f_RB_Mat3fident = { { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } } };

float Rad2Deg(float rad)
{
	return rad * (180.0f / (float)RB_PI);
}

float Deg2Rad(float deg)
{
	return deg * ((float)RB_PI / 180.0f);
}

void RB_Vec3fCreate(RBCONST float x, RBCONST float y, RBCONST float z, RB_Vec3f *v)
{
	v->e[0] = x;
	v->e[1] = y;
	v->e[2] = z;
}

float RB_Vec3fGetElem(RBCONST RB_Vec3f *v, RBCONST uint8_t i)
{
	RBAssert(i < 3u);
	return v->e[i];
}

void RB_Vec3fSetElem(RB_Vec3f *v, RBCONST uint8_t i, RBCONST float x)
{
	RBAssert(i < 3u);
	v->e[i] = x;
}

void RB_Vec3fAdd(RBCONST RB_Vec3f *v1, RBCONST RB_Vec3f *v2, RB_Vec3f *v_ans)
{
	v_ans->e[0] = v1->e[0] + v2->e[0];
	v_ans->e[1] = v1->e[1] + v2->e[1];
	v_ans->e[2] = v1->e[2] + v2->e[2];
}

void RB_Vec3fSub(RBCONST RB_Vec3f *v1, RBCONST RB_Vec3f *v2, RB_Vec3f *v_ans)
{
	v_ans->e[0] = v1->e[0] - v2->e[0];
	v_ans->e[1] = v1->e[1] - v2->e[1];
	v_ans->e[2] = v1->e[2] - v2->e[2];
}

bool RB_Vec3fMatch(RBCONST RB_Vec3f *v1, RBCONST RB_Vec3f *v2)
{
	RBCONST float eps = 1E-05f;
	bool ret = false;

	for(uint8_t i = 0; i < 3u; i++)
	{
		/// 差分が1E-05より大きい場合 ENDへ移動
		if(fabsf((v2->e[i]) - (v1->e[i])) > eps)
		{
			goto END;
		}
	}

	ret = true;

END:
	return ret;
}

float RB_Vec3fDot(RBCONST RB_Vec3f *v1, RBCONST RB_Vec3f *v2)
{
	float ret;
		ret = v1->e[0] * v2->e[0] + v1->e[1] * v2->e[1] + v1->e[2] * v2->e[2]; 
	return ret;
}

void RB_Vec3fCross(RBCONST RB_Vec3f *v1, RBCONST RB_Vec3f *v2, RB_Vec3f *v_ans)
{
	v_ans->e[0] = v1->e[1] * v2->e[2] - v1->e[2] * v2->e[1]; 
	v_ans->e[1] = v1->e[2] * v2->e[0] - v1->e[0] * v2->e[2];
	v_ans->e[2] = v1->e[0] * v2->e[1] - v1->e[1] * v2->e[0];
}

void RB_CalcVerticalVec3f(RBCONST RB_Vec3f *v1, RB_Vec3f *v_ans)
{
	RB_Vec3f tmp, v2, v1_normalize;
	RB_Vec3fCreate(1.0f, 0.0f, 0.0f, &tmp);

	RB_Vec3fNormalize(v1, &v1_normalize);
	if(RB_Vec3fMatch(&tmp, &v1_normalize))
	{
		RB_Vec3fCreate(0.0f, 1.0f, 0.0f, &tmp);
	}

	RB_Vec3fCross(v1, &tmp, &v2);
	RB_Vec3fNormalize(&v2, v_ans);
}

float RB_Vec3fNorm(RBCONST RB_Vec3f *v)
{
	return sqrtf(RB_Vec3fDot(v,v));
}

void RB_Vec3fNormalize(RBCONST RB_Vec3f *v, RB_Vec3f *v_ans)
{
	//ノルムを計算
	float square = RB_Vec3fNorm(v);
	RBCONST float eps = 1E-05f;

	//ノルムが0より大きいとき
	if(square > eps)
	{
		//3次元ベクトルをノルムで割り、正規化ベクトルを導出する
		float mag = 1.0f / square;

		for(uint8_t i = 0u; i < 3u; i++)
		{
		v_ans->e[i] = v->e[i] * mag;
		}
	}
	//それ以外(ノルムが0の場合)はゼロベクトルを返す
	else
	{
		for(uint8_t i = 0u; i < 3u; i++)
		{
			v_ans->e[i] = 0.0f;
		}
	}
}

float CalcAngleBetweenVec3f(uint8_t axis, RBCONST RB_Vec3f *rel)
{
	float num, deno;
	RB_Vec3f nmlvec;

	if(axis > 2)
	{
		num = 1.0f;
		deno = 1.0f;
	}
	else
	{
		switch(axis)
		{
			case 0:
				RB_Vec3fCreate(1.0f, 0.0f, 0.0f, &nmlvec);
				break;

			case 1:
				RB_Vec3fCreate(0.0f, 1.0f, 0.0f, &nmlvec);
				break;

			case 2:
				RB_Vec3fCreate(0.0f, 0.0f, 1.0f, &nmlvec);
				break;
		}
		num = RB_Vec3fDot(rel, &nmlvec);
		deno = (RB_Vec3fNorm(rel)) * (RB_Vec3fNorm(&nmlvec));
	}

	float phi = ((float)RB_PI * 0.5f) - acosf( num / deno);
	return phi;
}

void RB_Mat3fCreate(
RBCONST float e11, RBCONST float e12, RBCONST float e13,
RBCONST float e21, RBCONST float e22, RBCONST float e23,
RBCONST float e31, RBCONST float e32, RBCONST float e33,
RB_Mat3f *m
)
{
	m->e[0][0] = e11; m->e[0][1] = e12; m->e[0][2] = e13;
	m->e[1][0] = e21; m->e[1][1] = e22; m->e[1][2] = e23;
	m->e[2][0] = e31; m->e[2][1] = e32; m->e[2][2] = e33;
}

float RB_Mat3fGetElem(RBCONST RB_Mat3f *m, RBCONST uint8_t r, RBCONST uint8_t c)
{
	RBAssert((r < 3u) || (c < 3u));
	return m->e[r][c];
}

void RB_Mat3fSetElem(RB_Mat3f *m, RBCONST uint8_t r, RBCONST uint8_t c, RBCONST float x)
{
	m->e[r][c] = x;
}

void RB_MulMatVec3f(RBCONST RB_Mat3f *m, RBCONST RB_Vec3f *v, RB_Vec3f *mv_ans)
{
	//M・v = vec
	mv_ans->e[0] = m->e[0][0] * v->e[0] + m->e[0][1] * v->e[1] + m->e[0][2] * v->e[2];
	mv_ans->e[1] = m->e[1][0] * v->e[0] + m->e[1][1] * v->e[1] + m->e[1][2] * v->e[2];
	mv_ans->e[2] = m->e[2][0] * v->e[0] + m->e[2][1] * v->e[1] + m->e[2][2] * v->e[2];
}

void RB_MulMatMat3f(RBCONST RB_Mat3f *m1, RBCONST RB_Mat3f *m2, RB_Mat3f *m_ans)
{
    ///要素数分繰り返す
	for (uint8_t i = 0u; i < 3u; i++)
	{
		///要素数分繰り返す
		for (uint8_t j = 0u; j < 3u; j++)
		{
			///値の初期化
			m_ans->e[i][j] = 0.0f;

			///成分毎に乗算と加算を行う
			for (uint8_t k = 0u; k < 3u; k++)
			{
				m_ans->e[i][j] += m1->e[i][k] * m2->e[k][j];
			}
		}
	}
}

void RB_AxisRotateMat3f(RBCONST RB_Vec3f *v_axis, RBCONST float rad, RB_Mat3f *m_ans)
{
	RB_Mat3f m_ret;
	RBCONST float eps = 1E-05f;

	//角度に対する三角関数の計算値を格納
	float Cosq = cosf(rad);
	float Sinq = sinf(rad);
	float CosDeriv = 1.0f - Cosq;

	RB_Vec3f n_Vec3f;
	//正規化
	RB_Vec3fNormalize(v_axis, &n_Vec3f);

	//単位ベクトルではなかったとき(ゼロベクトルなら)
	if(RB_Vec3fMatch(&n_Vec3f, &f_RB_Vec3fzero))
	{
		//radがどんな値だろうと単位行列にする
		RB_Mat3fCreate(
			1.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 1.0f, &m_ret);	
	}
	//単位ベクトルなら
	else
	{
		//単位ベクトルを各成分に分解
		float nx = RB_Vec3fGetElem(&n_Vec3f, 0u);
		float ny = RB_Vec3fGetElem(&n_Vec3f, 1u);
		float nz = RB_Vec3fGetElem(&n_Vec3f, 2u);

		//ロドリゲスの回転公式を使用
		//任意の3次元単位ベクトルn_Vec3fまわりに角度[rad]だけ回転させる回転行列R(rad)を導出
		//  C = cosf(rad) S = sinf(rad) (1-C)=CosDeriv
		//			-                                             -
		//			| nxnx(1-C)+C   nxny(1-C)-nzS   nxnz(1-C)+nyS |
		//	R(rad) =| nynx(1-C)+nzS nyny(1-C)+C     nynz(1-C)-nxS |
		//			| nznx(1-C)-nyS nzny(1-C)+nxS   nznz(1-C)+C   |
		//			-			                                  -
		RB_Mat3fCreate(
			(nx * nx * CosDeriv + Cosq), (nx * ny * CosDeriv - nz * Sinq), (nx * nz * CosDeriv + ny * Sinq),
			(ny * nx * CosDeriv + nz * Sinq), (ny * ny * CosDeriv + Cosq), (ny * nz * CosDeriv - nx * Sinq),
			(nz * nx * CosDeriv - ny * Sinq), (nz * ny * CosDeriv + nx * Sinq), (nz * nz * CosDeriv + Cosq), &m_ret);
	}
	//m_ansに要素をコピーする
	memcpy( m_ans, &m_ret, sizeof(RB_Mat3f));
}

void RB_VecRotateVec3f(float rad, RBCONST RB_Vec3f *norm, RBCONST RB_Vec3f *rel, RB_Vec3f *v_ans)
{
	RB_Mat3f m_ret;
	RBCONST float eps = 1E-05f;

	//角度に対する三角関数の計算値を格納
	float Cosq = cosf(rad);
	float Sinq = sinf(rad);
	float CosDeriv = 1.0f - Cosq;

	float Dot_NormRelVec = RB_Vec3fDot(norm, rel);
	RB_Vec3f Cross_NormRelVec;
	RB_Vec3fCross(norm, rel, &Cross_NormRelVec);

	v_ans->e[0] = (CosDeriv * Dot_NormRelVec) * (norm->e[0]) + Cosq * (rel->e[0]) + Sinq * (Cross_NormRelVec.e[0]);
	v_ans->e[1] = (CosDeriv * Dot_NormRelVec) * (norm->e[1]) + Cosq * (rel->e[1]) + Sinq * (Cross_NormRelVec.e[1]);
	v_ans->e[2] = (CosDeriv * Dot_NormRelVec) * (norm->e[2]) + Cosq * (rel->e[2]) + Sinq * (Cross_NormRelVec.e[2]);

}


void RB_Mat3fTermOut(RBCONST char *str, RBCONST RB_Mat3f *m)
{
	printf("\n %s >>\n", str);
	if (!m)
	{
		printf("\r\n null 3f matrix\r\n");
	}
	else
	{
		printf("\n %.4f, %.4f, %.4f\r\n",
			(float)m->e[0][0],
			(float)m->e[0][1],
			(float)m->e[0][2]
		);
		printf("\n %.4f, %.4f, %.4f\r\n",
			(float)m->e[1][0],
			(float)m->e[1][1],
			(float)m->e[1][2]
		);
		printf("\n %.4f, %.4f, %.4f\r\n\n",
			(float)m->e[2][0],
			(float)m->e[2][1],
			(float)m->e[2][2]
		);
	}
}

void RB_Vec3fTermOut(RBCONST char *str, RBCONST RB_Vec3f *v)
{
	printf("\n %s >>\n", str);
	if (!v)
	{
		printf("\r\n null 3f vector\r\n");
	}
	else
	{
		printf("\n x:%.4f\n y:%.4f\n z:%.4f\r\n\n",
			(float)v->e[0],
			(float)v->e[1],
			(float)v->e[2]
		);
	}
}




