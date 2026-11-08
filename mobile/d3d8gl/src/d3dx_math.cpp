// Cynex: d3dx8 matematik (satir vektor, d3dx ile ayni formuller)
#include <d3dx8.h>
#include <string.h>
#include <math.h>

#ifdef CYNEX_D3DX_ORACLE
#include "d3dx_oracle_decls.h"
#include "d3dx_oracle_names.h"
#endif

static inline D3DXMATRIX* Ident(D3DXMATRIX* m)
{
	memset(m, 0, sizeof(*m));
	m->_11 = m->_22 = m->_33 = m->_44 = 1.0f;
	return m;
}

// ---------------------------------------------------------------- vektor

D3DXVECTOR2* WINAPI D3DXVec2Normalize(D3DXVECTOR2* pOut, CONST D3DXVECTOR2* pV)
{
	float l = sqrtf(pV->x * pV->x + pV->y * pV->y);
	if (l == 0.0f) { pOut->x = pOut->y = 0.0f; }
	else { float x = pV->x / l, y = pV->y / l; pOut->x = x; pOut->y = y; }
	return pOut;
}

D3DXVECTOR4* WINAPI D3DXVec2Transform(D3DXVECTOR4* pOut, CONST D3DXVECTOR2* pV, CONST D3DXMATRIX* m)
{
	D3DXVECTOR4 r;
	r.x = pV->x * m->_11 + pV->y * m->_21 + m->_41;
	r.y = pV->x * m->_12 + pV->y * m->_22 + m->_42;
	r.z = pV->x * m->_13 + pV->y * m->_23 + m->_43;
	r.w = pV->x * m->_14 + pV->y * m->_24 + m->_44;
	*pOut = r;
	return pOut;
}

D3DXVECTOR2* WINAPI D3DXVec2TransformCoord(D3DXVECTOR2* pOut, CONST D3DXVECTOR2* pV, CONST D3DXMATRIX* m)
{
	D3DXVECTOR4 r;
	D3DXVec2Transform(&r, pV, m);
	float iw = r.w != 0.0f ? 1.0f / r.w : 0.0f;
	pOut->x = r.x * iw;
	pOut->y = r.y * iw;
	return pOut;
}

D3DXVECTOR2* WINAPI D3DXVec2TransformNormal(D3DXVECTOR2* pOut, CONST D3DXVECTOR2* pV, CONST D3DXMATRIX* m)
{
	float x = pV->x * m->_11 + pV->y * m->_21;
	float y = pV->x * m->_12 + pV->y * m->_22;
	pOut->x = x; pOut->y = y;
	return pOut;
}

D3DXVECTOR3* WINAPI D3DXVec3Normalize(D3DXVECTOR3* pOut, CONST D3DXVECTOR3* pV)
{
	float l = sqrtf(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z);
	if (l == 0.0f) { pOut->x = pOut->y = pOut->z = 0.0f; }
	else
	{
		float x = pV->x / l, y = pV->y / l, z = pV->z / l;
		pOut->x = x; pOut->y = y; pOut->z = z;
	}
	return pOut;
}

D3DXVECTOR3* WINAPI D3DXVec3Hermite(D3DXVECTOR3* pOut, CONST D3DXVECTOR3* v1, CONST D3DXVECTOR3* t1, CONST D3DXVECTOR3* v2, CONST D3DXVECTOR3* t2, FLOAT s)
{
	float h1 = 2.0f * s * s * s - 3.0f * s * s + 1.0f;
	float h2 = s * s * s - 2.0f * s * s + s;
	float h3 = -2.0f * s * s * s + 3.0f * s * s;
	float h4 = s * s * s - s * s;
	pOut->x = h1 * v1->x + h2 * t1->x + h3 * v2->x + h4 * t2->x;
	pOut->y = h1 * v1->y + h2 * t1->y + h3 * v2->y + h4 * t2->y;
	pOut->z = h1 * v1->z + h2 * t1->z + h3 * v2->z + h4 * t2->z;
	return pOut;
}

D3DXVECTOR3* WINAPI D3DXVec3CatmullRom(D3DXVECTOR3* pOut, CONST D3DXVECTOR3* v0, CONST D3DXVECTOR3* v1, CONST D3DXVECTOR3* v2, CONST D3DXVECTOR3* v3, FLOAT s)
{
	float s2 = s * s, s3 = s2 * s;
	pOut->x = 0.5f * (2.0f * v1->x + (v2->x - v0->x) * s + (2.0f * v0->x - 5.0f * v1->x + 4.0f * v2->x - v3->x) * s2 + (v3->x - 3.0f * v2->x + 3.0f * v1->x - v0->x) * s3);
	pOut->y = 0.5f * (2.0f * v1->y + (v2->y - v0->y) * s + (2.0f * v0->y - 5.0f * v1->y + 4.0f * v2->y - v3->y) * s2 + (v3->y - 3.0f * v2->y + 3.0f * v1->y - v0->y) * s3);
	pOut->z = 0.5f * (2.0f * v1->z + (v2->z - v0->z) * s + (2.0f * v0->z - 5.0f * v1->z + 4.0f * v2->z - v3->z) * s2 + (v3->z - 3.0f * v2->z + 3.0f * v1->z - v0->z) * s3);
	return pOut;
}

D3DXVECTOR3* WINAPI D3DXVec3BaryCentric(D3DXVECTOR3* pOut, CONST D3DXVECTOR3* v1, CONST D3DXVECTOR3* v2, CONST D3DXVECTOR3* v3, FLOAT f, FLOAT g)
{
	pOut->x = v1->x + f * (v2->x - v1->x) + g * (v3->x - v1->x);
	pOut->y = v1->y + f * (v2->y - v1->y) + g * (v3->y - v1->y);
	pOut->z = v1->z + f * (v2->z - v1->z) + g * (v3->z - v1->z);
	return pOut;
}

D3DXVECTOR4* WINAPI D3DXVec3Transform(D3DXVECTOR4* pOut, CONST D3DXVECTOR3* pV, CONST D3DXMATRIX* m)
{
	D3DXVECTOR4 r;
	r.x = pV->x * m->_11 + pV->y * m->_21 + pV->z * m->_31 + m->_41;
	r.y = pV->x * m->_12 + pV->y * m->_22 + pV->z * m->_32 + m->_42;
	r.z = pV->x * m->_13 + pV->y * m->_23 + pV->z * m->_33 + m->_43;
	r.w = pV->x * m->_14 + pV->y * m->_24 + pV->z * m->_34 + m->_44;
	*pOut = r;
	return pOut;
}

D3DXVECTOR3* WINAPI D3DXVec3TransformCoord(D3DXVECTOR3* pOut, CONST D3DXVECTOR3* pV, CONST D3DXMATRIX* m)
{
	float x = pV->x * m->_11 + pV->y * m->_21 + pV->z * m->_31 + m->_41;
	float y = pV->x * m->_12 + pV->y * m->_22 + pV->z * m->_32 + m->_42;
	float z = pV->x * m->_13 + pV->y * m->_23 + pV->z * m->_33 + m->_43;
	float w = pV->x * m->_14 + pV->y * m->_24 + pV->z * m->_34 + m->_44;
	float iw = w != 0.0f ? 1.0f / w : 0.0f;
	pOut->x = x * iw;
	pOut->y = y * iw;
	pOut->z = z * iw;
	return pOut;
}

D3DXVECTOR3* WINAPI D3DXVec3TransformNormal(D3DXVECTOR3* pOut, CONST D3DXVECTOR3* pV, CONST D3DXMATRIX* m)
{
	float x = pV->x * m->_11 + pV->y * m->_21 + pV->z * m->_31;
	float y = pV->x * m->_12 + pV->y * m->_22 + pV->z * m->_32;
	float z = pV->x * m->_13 + pV->y * m->_23 + pV->z * m->_33;
	pOut->x = x; pOut->y = y; pOut->z = z;
	return pOut;
}

static void ConcatWVP(D3DXMATRIX* out, CONST D3DXMATRIX* proj, CONST D3DXMATRIX* view, CONST D3DXMATRIX* world)
{
	Ident(out);
	if (world) D3DXMatrixMultiply(out, out, world);
	if (view) D3DXMatrixMultiply(out, out, view);
	if (proj) D3DXMatrixMultiply(out, out, proj);
}

D3DXVECTOR3* WINAPI D3DXVec3Project(D3DXVECTOR3* pOut, CONST D3DXVECTOR3* pV, CONST D3DVIEWPORT8* vp,
	CONST D3DXMATRIX* proj, CONST D3DXMATRIX* view, CONST D3DXMATRIX* world)
{
	D3DXMATRIX m;
	ConcatWVP(&m, proj, view, world);
	D3DXVECTOR3 r;
	D3DXVec3TransformCoord(&r, pV, &m);
	if (vp)
	{
		r.x = vp->X + (1.0f + r.x) * vp->Width * 0.5f;
		r.y = vp->Y + (1.0f - r.y) * vp->Height * 0.5f;
		r.z = vp->MinZ + r.z * (vp->MaxZ - vp->MinZ);
	}
	*pOut = r;
	return pOut;
}

D3DXVECTOR3* WINAPI D3DXVec3Unproject(D3DXVECTOR3* pOut, CONST D3DXVECTOR3* pV, CONST D3DVIEWPORT8* vp,
	CONST D3DXMATRIX* proj, CONST D3DXMATRIX* view, CONST D3DXMATRIX* world)
{
	D3DXMATRIX m;
	ConcatWVP(&m, proj, view, world);
	if (!D3DXMatrixInverse(&m, NULL, &m))
		return NULL;
	D3DXVECTOR3 v = *pV;
	if (vp)
	{
		v.x = 2.0f * (v.x - vp->X) / vp->Width - 1.0f;
		v.y = 1.0f - 2.0f * (v.y - vp->Y) / vp->Height;
		v.z = (vp->MaxZ - vp->MinZ) != 0.0f ? (v.z - vp->MinZ) / (vp->MaxZ - vp->MinZ) : 0.0f;
	}
	D3DXVec3TransformCoord(pOut, &v, &m);
	return pOut;
}

D3DXVECTOR4* WINAPI D3DXVec4Normalize(D3DXVECTOR4* pOut, CONST D3DXVECTOR4* pV)
{
	float l = sqrtf(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z + pV->w * pV->w);
	if (l == 0.0f) { pOut->x = pOut->y = pOut->z = pOut->w = 0.0f; }
	else { D3DXVECTOR4 r(pV->x / l, pV->y / l, pV->z / l, pV->w / l); *pOut = r; }
	return pOut;
}

D3DXVECTOR4* WINAPI D3DXVec4Transform(D3DXVECTOR4* pOut, CONST D3DXVECTOR4* pV, CONST D3DXMATRIX* m)
{
	D3DXVECTOR4 r;
	r.x = pV->x * m->_11 + pV->y * m->_21 + pV->z * m->_31 + pV->w * m->_41;
	r.y = pV->x * m->_12 + pV->y * m->_22 + pV->z * m->_32 + pV->w * m->_42;
	r.z = pV->x * m->_13 + pV->y * m->_23 + pV->z * m->_33 + pV->w * m->_43;
	r.w = pV->x * m->_14 + pV->y * m->_24 + pV->z * m->_34 + pV->w * m->_44;
	*pOut = r;
	return pOut;
}

D3DXVECTOR4* WINAPI D3DXVec4Cross(D3DXVECTOR4* pOut, CONST D3DXVECTOR4* a, CONST D3DXVECTOR4* b, CONST D3DXVECTOR4* c)
{
	D3DXVECTOR4 r;
	r.x = a->y * (b->z * c->w - c->z * b->w) - a->z * (b->y * c->w - c->y * b->w) + a->w * (b->y * c->z - b->z * c->y);
	r.y = -(a->x * (b->z * c->w - c->z * b->w) - a->z * (b->x * c->w - c->x * b->w) + a->w * (b->x * c->z - c->x * b->z));
	r.z = a->x * (b->y * c->w - c->y * b->w) - a->y * (b->x * c->w - c->x * b->w) + a->w * (b->x * c->y - c->x * b->y);
	r.w = -(a->x * (b->y * c->z - c->y * b->z) - a->y * (b->x * c->z - c->x * b->z) + a->z * (b->x * c->y - c->x * b->y));
	*pOut = r;
	return pOut;
}

// ---------------------------------------------------------------- matris

FLOAT WINAPI D3DXMatrixfDeterminant(CONST D3DXMATRIX* m)
{
	const float s0 = m->_33 * m->_44 - m->_34 * m->_43;
	const float s1 = m->_32 * m->_44 - m->_34 * m->_42;
	const float s2 = m->_32 * m->_43 - m->_33 * m->_42;
	const float s3 = m->_31 * m->_44 - m->_34 * m->_41;
	const float s4 = m->_31 * m->_43 - m->_33 * m->_41;
	const float s5 = m->_31 * m->_42 - m->_32 * m->_41;
	return m->_11 * (m->_22 * s0 - m->_23 * s1 + m->_24 * s2)
		- m->_12 * (m->_21 * s0 - m->_23 * s3 + m->_24 * s4)
		+ m->_13 * (m->_21 * s1 - m->_22 * s3 + m->_24 * s5)
		- m->_14 * (m->_21 * s2 - m->_22 * s4 + m->_23 * s5);
}

D3DXMATRIX* WINAPI D3DXMatrixTranspose(D3DXMATRIX* pOut, CONST D3DXMATRIX* pM)
{
	D3DXMATRIX r;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			r.m[i][j] = pM->m[j][i];
	*pOut = r;
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixMultiply(D3DXMATRIX* pOut, CONST D3DXMATRIX* a, CONST D3DXMATRIX* b)
{
	D3DXMATRIX r;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			r.m[i][j] = a->m[i][0] * b->m[0][j] + a->m[i][1] * b->m[1][j] + a->m[i][2] * b->m[2][j] + a->m[i][3] * b->m[3][j];
	*pOut = r;
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixMultiplyTranspose(D3DXMATRIX* pOut, CONST D3DXMATRIX* a, CONST D3DXMATRIX* b)
{
	D3DXMATRIX r;
	D3DXMatrixMultiply(&r, a, b);
	return D3DXMatrixTranspose(pOut, &r);
}

D3DXMATRIX* WINAPI D3DXMatrixInverse(D3DXMATRIX* pOut, FLOAT* pDet, CONST D3DXMATRIX* pM)
{
	const float* a = &pM->m[0][0];
	float inv[16];
	inv[0] = a[5] * a[10] * a[15] - a[5] * a[11] * a[14] - a[9] * a[6] * a[15] + a[9] * a[7] * a[14] + a[13] * a[6] * a[11] - a[13] * a[7] * a[10];
	inv[4] = -a[4] * a[10] * a[15] + a[4] * a[11] * a[14] + a[8] * a[6] * a[15] - a[8] * a[7] * a[14] - a[12] * a[6] * a[11] + a[12] * a[7] * a[10];
	inv[8] = a[4] * a[9] * a[15] - a[4] * a[11] * a[13] - a[8] * a[5] * a[15] + a[8] * a[7] * a[13] + a[12] * a[5] * a[11] - a[12] * a[7] * a[9];
	inv[12] = -a[4] * a[9] * a[14] + a[4] * a[10] * a[13] + a[8] * a[5] * a[14] - a[8] * a[6] * a[13] - a[12] * a[5] * a[10] + a[12] * a[6] * a[9];
	inv[1] = -a[1] * a[10] * a[15] + a[1] * a[11] * a[14] + a[9] * a[2] * a[15] - a[9] * a[3] * a[14] - a[13] * a[2] * a[11] + a[13] * a[3] * a[10];
	inv[5] = a[0] * a[10] * a[15] - a[0] * a[11] * a[14] - a[8] * a[2] * a[15] + a[8] * a[3] * a[14] + a[12] * a[2] * a[11] - a[12] * a[3] * a[10];
	inv[9] = -a[0] * a[9] * a[15] + a[0] * a[11] * a[13] + a[8] * a[1] * a[15] - a[8] * a[3] * a[13] - a[12] * a[1] * a[11] + a[12] * a[3] * a[9];
	inv[13] = a[0] * a[9] * a[14] - a[0] * a[10] * a[13] - a[8] * a[1] * a[14] + a[8] * a[2] * a[13] + a[12] * a[1] * a[10] - a[12] * a[2] * a[9];
	inv[2] = a[1] * a[6] * a[15] - a[1] * a[7] * a[14] - a[5] * a[2] * a[15] + a[5] * a[3] * a[14] + a[13] * a[2] * a[7] - a[13] * a[3] * a[6];
	inv[6] = -a[0] * a[6] * a[15] + a[0] * a[7] * a[14] + a[4] * a[2] * a[15] - a[4] * a[3] * a[14] - a[12] * a[2] * a[7] + a[12] * a[3] * a[6];
	inv[10] = a[0] * a[5] * a[15] - a[0] * a[7] * a[13] - a[4] * a[1] * a[15] + a[4] * a[3] * a[13] + a[12] * a[1] * a[7] - a[12] * a[3] * a[5];
	inv[14] = -a[0] * a[5] * a[14] + a[0] * a[6] * a[13] + a[4] * a[1] * a[14] - a[4] * a[2] * a[13] - a[12] * a[1] * a[6] + a[12] * a[2] * a[5];
	inv[3] = -a[1] * a[6] * a[11] + a[1] * a[7] * a[10] + a[5] * a[2] * a[11] - a[5] * a[3] * a[10] - a[9] * a[2] * a[7] + a[9] * a[3] * a[6];
	inv[7] = a[0] * a[6] * a[11] - a[0] * a[7] * a[10] - a[4] * a[2] * a[11] + a[4] * a[3] * a[10] + a[8] * a[2] * a[7] - a[8] * a[3] * a[6];
	inv[11] = -a[0] * a[5] * a[11] + a[0] * a[7] * a[9] + a[4] * a[1] * a[11] - a[4] * a[3] * a[9] - a[8] * a[1] * a[7] + a[8] * a[3] * a[5];
	inv[15] = a[0] * a[5] * a[10] - a[0] * a[6] * a[9] - a[4] * a[1] * a[10] + a[4] * a[2] * a[9] + a[8] * a[1] * a[6] - a[8] * a[2] * a[5];

	float det = a[0] * inv[0] + a[1] * inv[4] + a[2] * inv[8] + a[3] * inv[12];
	if (pDet)
		*pDet = det;
	if (det == 0.0f)
		return NULL;
	float id = 1.0f / det;
	float* o = &pOut->m[0][0];
	for (int i = 0; i < 16; ++i)
		o[i] = inv[i] * id;
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixScaling(D3DXMATRIX* pOut, FLOAT sx, FLOAT sy, FLOAT sz)
{
	Ident(pOut);
	pOut->_11 = sx; pOut->_22 = sy; pOut->_33 = sz;
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixTranslation(D3DXMATRIX* pOut, FLOAT x, FLOAT y, FLOAT z)
{
	Ident(pOut);
	pOut->_41 = x; pOut->_42 = y; pOut->_43 = z;
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixRotationX(D3DXMATRIX* pOut, FLOAT a)
{
	float s = sinf(a), c = cosf(a);
	Ident(pOut);
	pOut->_22 = c; pOut->_23 = s;
	pOut->_32 = -s; pOut->_33 = c;
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixRotationY(D3DXMATRIX* pOut, FLOAT a)
{
	float s = sinf(a), c = cosf(a);
	Ident(pOut);
	pOut->_11 = c; pOut->_13 = -s;
	pOut->_31 = s; pOut->_33 = c;
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixRotationZ(D3DXMATRIX* pOut, FLOAT a)
{
	float s = sinf(a), c = cosf(a);
	Ident(pOut);
	pOut->_11 = c; pOut->_12 = s;
	pOut->_21 = -s; pOut->_22 = c;
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixRotationAxis(D3DXMATRIX* pOut, CONST D3DXVECTOR3* pV, FLOAT a)
{
	D3DXVECTOR3 v;
	D3DXVec3Normalize(&v, pV);
	float s = sinf(a), c = cosf(a), t = 1.0f - c;
	Ident(pOut);
	pOut->_11 = t * v.x * v.x + c;
	pOut->_12 = t * v.x * v.y + s * v.z;
	pOut->_13 = t * v.x * v.z - s * v.y;
	pOut->_21 = t * v.x * v.y - s * v.z;
	pOut->_22 = t * v.y * v.y + c;
	pOut->_23 = t * v.y * v.z + s * v.x;
	pOut->_31 = t * v.x * v.z + s * v.y;
	pOut->_32 = t * v.y * v.z - s * v.x;
	pOut->_33 = t * v.z * v.z + c;
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixRotationQuaternion(D3DXMATRIX* pOut, CONST D3DXQUATERNION* q)
{
	const float x = q->x, y = q->y, z = q->z, w = q->w;
	Ident(pOut);
	pOut->_11 = 1.0f - 2.0f * (y * y + z * z);
	pOut->_12 = 2.0f * (x * y + z * w);
	pOut->_13 = 2.0f * (x * z - y * w);
	pOut->_21 = 2.0f * (x * y - z * w);
	pOut->_22 = 1.0f - 2.0f * (x * x + z * z);
	pOut->_23 = 2.0f * (y * z + x * w);
	pOut->_31 = 2.0f * (x * z + y * w);
	pOut->_32 = 2.0f * (y * z - x * w);
	pOut->_33 = 1.0f - 2.0f * (x * x + y * y);
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixRotationYawPitchRoll(D3DXMATRIX* pOut, FLOAT yaw, FLOAT pitch, FLOAT roll)
{
	// Cynex: once roll (z), sonra pitch (x), sonra yaw (y)
	const float sr = sinf(roll), cr = cosf(roll);
	const float sp = sinf(pitch), cp = cosf(pitch);
	const float sy = sinf(yaw), cy = cosf(yaw);
	Ident(pOut);
	pOut->_11 = cr * cy + sr * sp * sy;
	pOut->_12 = sr * cp;
	pOut->_13 = sr * sp * cy - cr * sy;
	pOut->_21 = cr * sp * sy - sr * cy;
	pOut->_22 = cr * cp;
	pOut->_23 = sr * sy + cr * sp * cy;
	pOut->_31 = cp * sy;
	pOut->_32 = -sp;
	pOut->_33 = cp * cy;
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixTransformation(D3DXMATRIX* pOut, CONST D3DXVECTOR3* pSC, CONST D3DXQUATERNION* pSR,
	CONST D3DXVECTOR3* pS, CONST D3DXVECTOR3* pRC, CONST D3DXQUATERNION* pR, CONST D3DXVECTOR3* pT)
{
	D3DXMATRIX m, t;
	Ident(&m);
	D3DXVECTOR3 sc = pSC ? *pSC : D3DXVECTOR3(0, 0, 0);
	D3DXVECTOR3 rc = pRC ? *pRC : D3DXVECTOR3(0, 0, 0);

	if (pS)
	{
		D3DXMATRIX msr, msri;
		D3DXMatrixTranslation(&t, -sc.x, -sc.y, -sc.z);
		D3DXMatrixMultiply(&m, &m, &t);
		if (pSR)
		{
			D3DXMatrixRotationQuaternion(&msr, pSR);
			D3DXMatrixTranspose(&msri, &msr);	// Cynex: dik matrisin tersi devrigi
			D3DXMatrixMultiply(&m, &m, &msri);
		}
		D3DXMatrixScaling(&t, pS->x, pS->y, pS->z);
		D3DXMatrixMultiply(&m, &m, &t);
		if (pSR)
			D3DXMatrixMultiply(&m, &m, &msr);
		D3DXMatrixTranslation(&t, sc.x, sc.y, sc.z);
		D3DXMatrixMultiply(&m, &m, &t);
	}
	if (pR)
	{
		D3DXMatrixTranslation(&t, -rc.x, -rc.y, -rc.z);
		D3DXMatrixMultiply(&m, &m, &t);
		D3DXMatrixRotationQuaternion(&t, pR);
		D3DXMatrixMultiply(&m, &m, &t);
		D3DXMatrixTranslation(&t, rc.x, rc.y, rc.z);
		D3DXMatrixMultiply(&m, &m, &t);
	}
	if (pT)
	{
		D3DXMatrixTranslation(&t, pT->x, pT->y, pT->z);
		D3DXMatrixMultiply(&m, &m, &t);
	}
	*pOut = m;
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixAffineTransformation(D3DXMATRIX* pOut, FLOAT s, CONST D3DXVECTOR3* pRC, CONST D3DXQUATERNION* pR, CONST D3DXVECTOR3* pT)
{
	D3DXVECTOR3 sv(s, s, s);
	return D3DXMatrixTransformation(pOut, NULL, NULL, &sv, pRC, pR, pT);
}

static D3DXMATRIX* LookAt(D3DXMATRIX* pOut, const D3DXVECTOR3& eye, const D3DXVECTOR3& zaxis, const D3DXVECTOR3* up)
{
	D3DXVECTOR3 xaxis, yaxis;
	D3DXVec3Cross(&xaxis, up, &zaxis);
	D3DXVec3Normalize(&xaxis, &xaxis);
	D3DXVec3Cross(&yaxis, &zaxis, &xaxis);
	Ident(pOut);
	pOut->_11 = xaxis.x; pOut->_12 = yaxis.x; pOut->_13 = zaxis.x;
	pOut->_21 = xaxis.y; pOut->_22 = yaxis.y; pOut->_23 = zaxis.y;
	pOut->_31 = xaxis.z; pOut->_32 = yaxis.z; pOut->_33 = zaxis.z;
	pOut->_41 = -D3DXVec3Dot(&xaxis, &eye);
	pOut->_42 = -D3DXVec3Dot(&yaxis, &eye);
	pOut->_43 = -D3DXVec3Dot(&zaxis, &eye);
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixLookAtRH(D3DXMATRIX* pOut, CONST D3DXVECTOR3* pEye, CONST D3DXVECTOR3* pAt, CONST D3DXVECTOR3* pUp)
{
	D3DXVECTOR3 z = *pEye - *pAt;
	D3DXVec3Normalize(&z, &z);
	return LookAt(pOut, *pEye, z, pUp);
}

D3DXMATRIX* WINAPI D3DXMatrixLookAtLH(D3DXMATRIX* pOut, CONST D3DXVECTOR3* pEye, CONST D3DXVECTOR3* pAt, CONST D3DXVECTOR3* pUp)
{
	D3DXVECTOR3 z = *pAt - *pEye;
	D3DXVec3Normalize(&z, &z);
	return LookAt(pOut, *pEye, z, pUp);
}

D3DXMATRIX* WINAPI D3DXMatrixPerspectiveRH(D3DXMATRIX* pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf)
{
	memset(pOut, 0, sizeof(*pOut));
	pOut->_11 = 2.0f * zn / w;
	pOut->_22 = 2.0f * zn / h;
	pOut->_33 = zf / (zn - zf);
	pOut->_34 = -1.0f;
	pOut->_43 = zn * zf / (zn - zf);
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixPerspectiveLH(D3DXMATRIX* pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf)
{
	memset(pOut, 0, sizeof(*pOut));
	pOut->_11 = 2.0f * zn / w;
	pOut->_22 = 2.0f * zn / h;
	pOut->_33 = zf / (zf - zn);
	pOut->_34 = 1.0f;
	pOut->_43 = zn * zf / (zn - zf);
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixPerspectiveFovRH(D3DXMATRIX* pOut, FLOAT fovy, FLOAT aspect, FLOAT zn, FLOAT zf)
{
	float ys = 1.0f / tanf(fovy * 0.5f);
	memset(pOut, 0, sizeof(*pOut));
	pOut->_11 = ys / aspect;
	pOut->_22 = ys;
	pOut->_33 = zf / (zn - zf);
	pOut->_34 = -1.0f;
	pOut->_43 = zn * zf / (zn - zf);
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixPerspectiveFovLH(D3DXMATRIX* pOut, FLOAT fovy, FLOAT aspect, FLOAT zn, FLOAT zf)
{
	float ys = 1.0f / tanf(fovy * 0.5f);
	memset(pOut, 0, sizeof(*pOut));
	pOut->_11 = ys / aspect;
	pOut->_22 = ys;
	pOut->_33 = zf / (zf - zn);
	pOut->_34 = 1.0f;
	pOut->_43 = -zn * zf / (zf - zn);
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixPerspectiveOffCenterRH(D3DXMATRIX* pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn, FLOAT zf)
{
	memset(pOut, 0, sizeof(*pOut));
	pOut->_11 = 2.0f * zn / (r - l);
	pOut->_22 = 2.0f * zn / (t - b);
	pOut->_31 = (l + r) / (r - l);
	pOut->_32 = (t + b) / (t - b);
	pOut->_33 = zf / (zn - zf);
	pOut->_34 = -1.0f;
	pOut->_43 = zn * zf / (zn - zf);
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixPerspectiveOffCenterLH(D3DXMATRIX* pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn, FLOAT zf)
{
	memset(pOut, 0, sizeof(*pOut));
	pOut->_11 = 2.0f * zn / (r - l);
	pOut->_22 = 2.0f * zn / (t - b);
	pOut->_31 = (l + r) / (l - r);
	pOut->_32 = (t + b) / (b - t);
	pOut->_33 = zf / (zf - zn);
	pOut->_34 = 1.0f;
	pOut->_43 = zn * zf / (zn - zf);
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixOrthoRH(D3DXMATRIX* pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf)
{
	Ident(pOut);
	pOut->_11 = 2.0f / w;
	pOut->_22 = 2.0f / h;
	pOut->_33 = 1.0f / (zn - zf);
	pOut->_43 = zn / (zn - zf);
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixOrthoLH(D3DXMATRIX* pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf)
{
	Ident(pOut);
	pOut->_11 = 2.0f / w;
	pOut->_22 = 2.0f / h;
	pOut->_33 = 1.0f / (zf - zn);
	pOut->_43 = zn / (zn - zf);
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixOrthoOffCenterRH(D3DXMATRIX* pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn, FLOAT zf)
{
	Ident(pOut);
	pOut->_11 = 2.0f / (r - l);
	pOut->_22 = 2.0f / (t - b);
	pOut->_33 = 1.0f / (zn - zf);
	pOut->_41 = (l + r) / (l - r);
	pOut->_42 = (t + b) / (b - t);
	pOut->_43 = zn / (zn - zf);
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixOrthoOffCenterLH(D3DXMATRIX* pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn, FLOAT zf)
{
	Ident(pOut);
	pOut->_11 = 2.0f / (r - l);
	pOut->_22 = 2.0f / (t - b);
	pOut->_33 = 1.0f / (zf - zn);
	pOut->_41 = (l + r) / (l - r);
	pOut->_42 = (t + b) / (b - t);
	pOut->_43 = zn / (zn - zf);
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixShadow(D3DXMATRIX* pOut, CONST D3DXVECTOR4* L, CONST D3DXPLANE* pP)
{
	D3DXPLANE p;
	D3DXPlaneNormalize(&p, pP);
	float d = p.a * L->x + p.b * L->y + p.c * L->z + p.d * L->w;
	const float pl[4] = { p.a, p.b, p.c, p.d };
	const float lv[4] = { L->x, L->y, L->z, L->w };
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			pOut->m[i][j] = (i == j ? d : 0.0f) - pl[i] * lv[j];
	return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixReflect(D3DXMATRIX* pOut, CONST D3DXPLANE* pP)
{
	D3DXPLANE p;
	D3DXPlaneNormalize(&p, pP);
	Ident(pOut);
	pOut->_11 = 1.0f - 2.0f * p.a * p.a; pOut->_12 = -2.0f * p.a * p.b; pOut->_13 = -2.0f * p.a * p.c;
	pOut->_21 = -2.0f * p.a * p.b; pOut->_22 = 1.0f - 2.0f * p.b * p.b; pOut->_23 = -2.0f * p.b * p.c;
	pOut->_31 = -2.0f * p.a * p.c; pOut->_32 = -2.0f * p.b * p.c; pOut->_33 = 1.0f - 2.0f * p.c * p.c;
	pOut->_41 = -2.0f * p.a * p.d; pOut->_42 = -2.0f * p.b * p.d; pOut->_43 = -2.0f * p.c * p.d;
	return pOut;
}

// ---------------------------------------------------------------- dordey

D3DXQUATERNION* WINAPI D3DXQuaternionRotationAxis(D3DXQUATERNION* pOut, CONST D3DXVECTOR3* pV, FLOAT a)
{
	D3DXVECTOR3 v;
	D3DXVec3Normalize(&v, pV);
	float s = sinf(a * 0.5f);
	pOut->x = v.x * s;
	pOut->y = v.y * s;
	pOut->z = v.z * s;
	pOut->w = cosf(a * 0.5f);
	return pOut;
}

D3DXQUATERNION* WINAPI D3DXQuaternionRotationYawPitchRoll(D3DXQUATERNION* pOut, FLOAT yaw, FLOAT pitch, FLOAT roll)
{
	const float sy = sinf(yaw * 0.5f), cy = cosf(yaw * 0.5f);
	const float sp = sinf(pitch * 0.5f), cp = cosf(pitch * 0.5f);
	const float sr = sinf(roll * 0.5f), cr = cosf(roll * 0.5f);
	pOut->x = cy * sp * cr + sy * cp * sr;
	pOut->y = sy * cp * cr - cy * sp * sr;
	pOut->z = cy * cp * sr - sy * sp * cr;
	pOut->w = cy * cp * cr + sy * sp * sr;
	return pOut;
}

D3DXQUATERNION* WINAPI D3DXQuaternionMultiply(D3DXQUATERNION* pOut, CONST D3DXQUATERNION* a, CONST D3DXQUATERNION* b)
{
	// Cynex: d3dx sirasi: once a sonra b donmesi (b*a)
	D3DXQUATERNION r;
	r.x = b->w * a->x + b->x * a->w + b->y * a->z - b->z * a->y;
	r.y = b->w * a->y - b->x * a->z + b->y * a->w + b->z * a->x;
	r.z = b->w * a->z + b->x * a->y - b->y * a->x + b->z * a->w;
	r.w = b->w * a->w - b->x * a->x - b->y * a->y - b->z * a->z;
	*pOut = r;
	return pOut;
}

D3DXQUATERNION* WINAPI D3DXQuaternionNormalize(D3DXQUATERNION* pOut, CONST D3DXQUATERNION* q)
{
	float l = sqrtf(q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w);
	if (l == 0.0f) { pOut->x = pOut->y = pOut->z = pOut->w = 0.0f; }
	else { D3DXQUATERNION r(q->x / l, q->y / l, q->z / l, q->w / l); *pOut = r; }
	return pOut;
}

D3DXQUATERNION* WINAPI D3DXQuaternionInverse(D3DXQUATERNION* pOut, CONST D3DXQUATERNION* q)
{
	float n = q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w;
	if (n == 0.0f) { pOut->x = pOut->y = pOut->z = pOut->w = 0.0f; return pOut; }
	D3DXQUATERNION r(-q->x / n, -q->y / n, -q->z / n, q->w / n);
	*pOut = r;
	return pOut;
}

D3DXQUATERNION* WINAPI D3DXQuaternionRotationMatrix(D3DXQUATERNION* pOut, CONST D3DXMATRIX* m)
{
	float tr = m->_11 + m->_22 + m->_33;
	D3DXQUATERNION q;
	if (tr > 0.0f)
	{
		float s = sqrtf(tr + 1.0f);
		q.w = s * 0.5f;
		s = 0.5f / s;
		q.x = (m->_23 - m->_32) * s;
		q.y = (m->_31 - m->_13) * s;
		q.z = (m->_12 - m->_21) * s;
	}
	else
	{
		int i = 0;
		if (m->_22 > m->_11) i = 1;
		if (m->_33 > m->m[i][i]) i = 2;
		int j = (i + 1) % 3, k = (j + 1) % 3;
		float s = sqrtf(m->m[i][i] - m->m[j][j] - m->m[k][k] + 1.0f);
		float qv[3];
		qv[i] = s * 0.5f;
		if (s != 0.0f) s = 0.5f / s;
		q.w = (m->m[j][k] - m->m[k][j]) * s;
		qv[j] = (m->m[i][j] + m->m[j][i]) * s;
		qv[k] = (m->m[i][k] + m->m[k][i]) * s;
		q.x = qv[0]; q.y = qv[1]; q.z = qv[2];
	}
	*pOut = q;
	return pOut;
}

void WINAPI D3DXQuaternionToAxisAngle(CONST D3DXQUATERNION* q, D3DXVECTOR3* pAxis, FLOAT* pAngle)
{
	if (pAxis) { pAxis->x = q->x; pAxis->y = q->y; pAxis->z = q->z; }
	if (pAngle) *pAngle = 2.0f * acosf(q->w);
}

D3DXQUATERNION* WINAPI D3DXQuaternionSlerp(D3DXQUATERNION* pOut, CONST D3DXQUATERNION* a, CONST D3DXQUATERNION* b, FLOAT t)
{
	float dot = a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
	float eps = 1.0f;
	if (dot < 0.0f) { eps = -1.0f; dot = -dot; }
	float u = 1.0f - t, v = t * eps;
	if (1.0f - dot > 0.001f)
	{
		float th = acosf(dot), s = sinf(th);
		u = sinf(th * (1.0f - t)) / s;
		v = sinf(th * t) / s * eps;
	}
	D3DXQUATERNION r(u * a->x + v * b->x, u * a->y + v * b->y, u * a->z + v * b->z, u * a->w + v * b->w);
	*pOut = r;
	return pOut;
}

// ---------------------------------------------------------------- duzlem / renk

D3DXPLANE* WINAPI D3DXPlaneNormalize(D3DXPLANE* pOut, CONST D3DXPLANE* p)
{
	float l = sqrtf(p->a * p->a + p->b * p->b + p->c * p->c);
	if (l == 0.0f) { pOut->a = pOut->b = pOut->c = pOut->d = 0.0f; }
	else { D3DXPLANE r(p->a / l, p->b / l, p->c / l, p->d / l); *pOut = r; }
	return pOut;
}

D3DXPLANE* WINAPI D3DXPlaneFromPointNormal(D3DXPLANE* pOut, CONST D3DXVECTOR3* pt, CONST D3DXVECTOR3* n)
{
	pOut->a = n->x; pOut->b = n->y; pOut->c = n->z;
	pOut->d = -D3DXVec3Dot(pt, n);
	return pOut;
}

D3DXPLANE* WINAPI D3DXPlaneFromPoints(D3DXPLANE* pOut, CONST D3DXVECTOR3* v1, CONST D3DXVECTOR3* v2, CONST D3DXVECTOR3* v3)
{
	D3DXVECTOR3 e1 = *v2 - *v1, e2 = *v3 - *v1, n;
	D3DXVec3Cross(&n, &e1, &e2);
	D3DXVec3Normalize(&n, &n);
	return D3DXPlaneFromPointNormal(pOut, v1, &n);
}

D3DXVECTOR3* WINAPI D3DXPlaneIntersectLine(D3DXVECTOR3* pOut, CONST D3DXPLANE* p, CONST D3DXVECTOR3* v1, CONST D3DXVECTOR3* v2)
{
	D3DXVECTOR3 d = *v2 - *v1;
	float den = p->a * d.x + p->b * d.y + p->c * d.z;
	if (den == 0.0f)
		return NULL;
	float t = -(p->a * v1->x + p->b * v1->y + p->c * v1->z + p->d) / den;
	*pOut = *v1 + d * t;
	return pOut;
}

D3DXPLANE* WINAPI D3DXPlaneTransform(D3DXPLANE* pOut, CONST D3DXPLANE* p, CONST D3DXMATRIX* m)
{
	D3DXPLANE r;
	r.a = p->a * m->_11 + p->b * m->_21 + p->c * m->_31 + p->d * m->_41;
	r.b = p->a * m->_12 + p->b * m->_22 + p->c * m->_32 + p->d * m->_42;
	r.c = p->a * m->_13 + p->b * m->_23 + p->c * m->_33 + p->d * m->_43;
	r.d = p->a * m->_14 + p->b * m->_24 + p->c * m->_34 + p->d * m->_44;
	*pOut = r;
	return pOut;
}

D3DXCOLOR* WINAPI D3DXColorAdjustSaturation(D3DXCOLOR* pOut, CONST D3DXCOLOR* c, FLOAT s)
{
	float grey = c->r * 0.2125f + c->g * 0.7154f + c->b * 0.0721f;
	pOut->r = grey + s * (c->r - grey);
	pOut->g = grey + s * (c->g - grey);
	pOut->b = grey + s * (c->b - grey);
	pOut->a = c->a;
	return pOut;
}

D3DXCOLOR* WINAPI D3DXColorAdjustContrast(D3DXCOLOR* pOut, CONST D3DXCOLOR* c, FLOAT s)
{
	pOut->r = 0.5f + s * (c->r - 0.5f);
	pOut->g = 0.5f + s * (c->g - 0.5f);
	pOut->b = 0.5f + s * (c->b - 0.5f);
	pOut->a = c->a;
	return pOut;
}
