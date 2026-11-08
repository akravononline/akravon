// Cynex: d3dx8 matematik kahin testi - d3dx8.lib ile Cy_ uygulamamizi karsilastir (windows x86)
#include <windows.h>
#include <d3dx8.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "d3dx_oracle_decls.h"

static int g_fail = 0, g_total = 0;

static float R(float a = -10, float b = 10) { return a + (b - a) * (rand() / (float)RAND_MAX); }

static void Check(const char* name, const float* a, const float* b, int n, float tol = 1e-4f)
{
	float worst = 0;
	for (int i = 0; i < n; ++i)
	{
		float d = fabsf(a[i] - b[i]);
		float s = fabsf(a[i]) > 1.0f ? fabsf(a[i]) : 1.0f;
		if (d / s > worst) worst = d / s;
	}
	++g_total;
	if (worst > tol)
	{
		++g_fail;
		printf("FAIL %-36s rel=%g\n", name, worst);
		for (int i = 0; i < n; ++i) printf("   [%d] d3dx=%g cy=%g\n", i, a[i], b[i]);
	}
}

static D3DXMATRIX RM() { D3DXMATRIX m; for (int i = 0; i < 16; ++i) (&m._11)[i] = R(); return m; }
static D3DXVECTOR3 RV3() { return D3DXVECTOR3(R(), R(), R()); }
static D3DXQUATERNION RQ() { D3DXQUATERNION q(R(-1, 1), R(-1, 1), R(-1, 1), R(-1, 1)); D3DXQuaternionNormalize(&q, &q); return q; }

#define M16(m) (&(m)._11)

int main()
{
	srand(1234);
	for (int it = 0; it < 200; ++it)
	{
		D3DXMATRIX a = RM(), b = RM(), o1, o2;
		D3DXVECTOR3 v = RV3(), v2 = RV3(), v3 = RV3(), r1, r2;
		D3DXVECTOR4 w1, w2, v4(R(), R(), R(), R());
		D3DXQUATERNION q = RQ(), q2 = RQ(), qa, qb;
		float ang = R(-6, 6), f1, f2;

		D3DXMatrixMultiply(&o1, &a, &b); Cy_D3DXMatrixMultiply(&o2, &a, &b); Check("MatrixMultiply", M16(o1), M16(o2), 16);
		D3DXMatrixTranspose(&o1, &a); Cy_D3DXMatrixTranspose(&o2, &a); Check("MatrixTranspose", M16(o1), M16(o2), 16);
		f1 = D3DXMatrixfDeterminant(&a); f2 = Cy_D3DXMatrixfDeterminant(&a); Check("MatrixfDeterminant", &f1, &f2, 1, 1e-3f);
		D3DXMatrixInverse(&o1, &f1, &a); Cy_D3DXMatrixInverse(&o2, &f2, &a); Check("MatrixInverse", M16(o1), M16(o2), 16, 2e-3f); Check("MatrixInverse det", &f1, &f2, 1, 1e-3f);
		D3DXMatrixScaling(&o1, v.x, v.y, v.z); Cy_D3DXMatrixScaling(&o2, v.x, v.y, v.z); Check("MatrixScaling", M16(o1), M16(o2), 16);
		D3DXMatrixTranslation(&o1, v.x, v.y, v.z); Cy_D3DXMatrixTranslation(&o2, v.x, v.y, v.z); Check("MatrixTranslation", M16(o1), M16(o2), 16);
		D3DXMatrixRotationX(&o1, ang); Cy_D3DXMatrixRotationX(&o2, ang); Check("MatrixRotationX", M16(o1), M16(o2), 16);
		D3DXMatrixRotationY(&o1, ang); Cy_D3DXMatrixRotationY(&o2, ang); Check("MatrixRotationY", M16(o1), M16(o2), 16);
		D3DXMatrixRotationZ(&o1, ang); Cy_D3DXMatrixRotationZ(&o2, ang); Check("MatrixRotationZ", M16(o1), M16(o2), 16);
		D3DXMatrixRotationAxis(&o1, &v, ang); Cy_D3DXMatrixRotationAxis(&o2, &v, ang); Check("MatrixRotationAxis", M16(o1), M16(o2), 16);
		D3DXMatrixRotationQuaternion(&o1, &q); Cy_D3DXMatrixRotationQuaternion(&o2, &q); Check("MatrixRotationQuaternion", M16(o1), M16(o2), 16);
		D3DXMatrixRotationYawPitchRoll(&o1, v.x, v.y, v.z); Cy_D3DXMatrixRotationYawPitchRoll(&o2, v.x, v.y, v.z); Check("MatrixRotationYawPitchRoll", M16(o1), M16(o2), 16);
		D3DXMatrixTransformation(&o1, &v, &q, &v2, &v3, &q2, &v); Cy_D3DXMatrixTransformation(&o2, &v, &q, &v2, &v3, &q2, &v); Check("MatrixTransformation", M16(o1), M16(o2), 16, 1e-3f);
		D3DXMatrixTransformation(&o1, NULL, NULL, &v2, NULL, &q2, &v); Cy_D3DXMatrixTransformation(&o2, NULL, NULL, &v2, NULL, &q2, &v); Check("MatrixTransformation(null)", M16(o1), M16(o2), 16, 1e-3f);
		D3DXVECTOR3 up(0, 0, 1);
		D3DXMatrixLookAtRH(&o1, &v, &v2, &up); Cy_D3DXMatrixLookAtRH(&o2, &v, &v2, &up); Check("MatrixLookAtRH", M16(o1), M16(o2), 16);
		D3DXMatrixLookAtLH(&o1, &v, &v2, &up); Cy_D3DXMatrixLookAtLH(&o2, &v, &v2, &up); Check("MatrixLookAtLH", M16(o1), M16(o2), 16);
		float zn = R(0.1f, 5), zf = R(50, 5000), fov = R(0.3f, 2.5f), asp = R(0.5f, 2.5f);
		D3DXMatrixPerspectiveFovRH(&o1, fov, asp, zn, zf); Cy_D3DXMatrixPerspectiveFovRH(&o2, fov, asp, zn, zf); Check("MatrixPerspectiveFovRH", M16(o1), M16(o2), 16);
		D3DXMatrixPerspectiveFovLH(&o1, fov, asp, zn, zf); Cy_D3DXMatrixPerspectiveFovLH(&o2, fov, asp, zn, zf); Check("MatrixPerspectiveFovLH", M16(o1), M16(o2), 16);
		D3DXMatrixOrthoRH(&o1, v.x, v.y, zn, zf); Cy_D3DXMatrixOrthoRH(&o2, v.x, v.y, zn, zf); Check("MatrixOrthoRH", M16(o1), M16(o2), 16);
		D3DXMatrixOrthoLH(&o1, v.x, v.y, zn, zf); Cy_D3DXMatrixOrthoLH(&o2, v.x, v.y, zn, zf); Check("MatrixOrthoLH", M16(o1), M16(o2), 16);
		D3DXMatrixOrthoOffCenterRH(&o1, 0, 1280, 720, 0, 0, 400); Cy_D3DXMatrixOrthoOffCenterRH(&o2, 0, 1280, 720, 0, 0, 400); Check("MatrixOrthoOffCenterRH ui", M16(o1), M16(o2), 16);
		D3DXMatrixOrthoOffCenterRH(&o1, v.x, v2.x, v.y, v2.y, zn, zf); Cy_D3DXMatrixOrthoOffCenterRH(&o2, v.x, v2.x, v.y, v2.y, zn, zf); Check("MatrixOrthoOffCenterRH", M16(o1), M16(o2), 16);
		D3DXMatrixOrthoOffCenterLH(&o1, v.x, v2.x, v.y, v2.y, zn, zf); Cy_D3DXMatrixOrthoOffCenterLH(&o2, v.x, v2.x, v.y, v2.y, zn, zf); Check("MatrixOrthoOffCenterLH", M16(o1), M16(o2), 16);
		D3DXMatrixPerspectiveOffCenterRH(&o1, v.x, v2.x, v.y, v2.y, zn, zf); Cy_D3DXMatrixPerspectiveOffCenterRH(&o2, v.x, v2.x, v.y, v2.y, zn, zf); Check("MatrixPerspectiveOffCenterRH", M16(o1), M16(o2), 16);

		D3DXQuaternionMultiply(&qa, &q, &q2); Cy_D3DXQuaternionMultiply(&qb, &q, &q2); Check("QuaternionMultiply", &qa.x, &qb.x, 4);
		D3DXQuaternionRotationAxis(&qa, &v, ang); Cy_D3DXQuaternionRotationAxis(&qb, &v, ang); Check("QuaternionRotationAxis", &qa.x, &qb.x, 4);
		D3DXQuaternionRotationYawPitchRoll(&qa, v.x, v.y, v.z); Cy_D3DXQuaternionRotationYawPitchRoll(&qb, v.x, v.y, v.z); Check("QuaternionRotationYawPitchRoll", &qa.x, &qb.x, 4);
		D3DXQuaternionSlerp(&qa, &q, &q2, R(0, 1));
		{
			float t = R(0, 1);
			D3DXQuaternionSlerp(&qa, &q, &q2, t); Cy_D3DXQuaternionSlerp(&qb, &q, &q2, t); Check("QuaternionSlerp", &qa.x, &qb.x, 4, 1e-3f);
		}
		D3DXQuaternionRotationMatrix(&qa, &o1);
		{
			D3DXMATRIX rm; D3DXMatrixRotationQuaternion(&rm, &q);
			D3DXQuaternionRotationMatrix(&qa, &rm); Cy_D3DXQuaternionRotationMatrix(&qb, &rm); Check("QuaternionRotationMatrix", &qa.x, &qb.x, 4, 1e-3f);
		}
		D3DXQuaternionInverse(&qa, &q); Cy_D3DXQuaternionInverse(&qb, &q); Check("QuaternionInverse", &qa.x, &qb.x, 4);

		D3DXVECTOR2 p2(R(), R()), p2a, p2b;
		D3DXVec2Normalize(&p2a, &p2); Cy_D3DXVec2Normalize(&p2b, &p2); Check("Vec2Normalize", &p2a.x, &p2b.x, 2);
		D3DXVec3Normalize(&r1, &v); Cy_D3DXVec3Normalize(&r2, &v); Check("Vec3Normalize", &r1.x, &r2.x, 3);
		D3DXVec3TransformCoord(&r1, &v, &a); Cy_D3DXVec3TransformCoord(&r2, &v, &a); Check("Vec3TransformCoord", &r1.x, &r2.x, 3, 1e-3f);
		D3DXVec3TransformNormal(&r1, &v, &a); Cy_D3DXVec3TransformNormal(&r2, &v, &a); Check("Vec3TransformNormal", &r1.x, &r2.x, 3);
		D3DXVec3Transform(&w1, &v, &a); Cy_D3DXVec3Transform(&w2, &v, &a); Check("Vec3Transform", &w1.x, &w2.x, 4);
		D3DXVec4Transform(&w1, &v4, &a); Cy_D3DXVec4Transform(&w2, &v4, &a); Check("Vec4Transform", &w1.x, &w2.x, 4);
		D3DXVec4Normalize(&w1, &v4); Cy_D3DXVec4Normalize(&w2, &v4); Check("Vec4Normalize", &w1.x, &w2.x, 4);

		D3DVIEWPORT8 vp = { 10, 20, 1280, 720, 0.0f, 1.0f };
		D3DXMATRIX proj, view, world;
		D3DXMatrixPerspectiveFovRH(&proj, fov, asp, zn, zf);
		D3DXVECTOR3 eye = RV3() * 3, at = RV3();
		D3DXMatrixLookAtRH(&view, &eye, &at, &up);
		D3DXMatrixRotationYawPitchRoll(&world, v.x, v.y, v.z);
		D3DXVECTOR3 pt = at + RV3() * 0.1f;
		D3DXVec3Project(&r1, &pt, &vp, &proj, &view, &world); Cy_D3DXVec3Project(&r2, &pt, &vp, &proj, &view, &world); Check("Vec3Project", &r1.x, &r2.x, 3, 1e-3f);
		D3DXVECTOR3 sp(R(0, 1280), R(0, 720), R(0, 1));
		D3DXVec3Unproject(&r1, &sp, &vp, &proj, &view, &world); Cy_D3DXVec3Unproject(&r2, &sp, &vp, &proj, &view, &world); Check("Vec3Unproject", &r1.x, &r2.x, 3, 1e-2f);
		D3DXVec3Unproject(&r1, &sp, &vp, &proj, &view, NULL); Cy_D3DXVec3Unproject(&r2, &sp, &vp, &proj, &view, NULL); Check("Vec3Unproject(null world)", &r1.x, &r2.x, 3, 1e-2f);

		D3DXPLANE pl(R(), R(), R(), R()), pa, pb;
		D3DXPlaneNormalize(&pa, &pl); Cy_D3DXPlaneNormalize(&pb, &pl); Check("PlaneNormalize", &pa.a, &pb.a, 4);
		D3DXPlaneFromPoints(&pa, &v, &v2, &v3); Cy_D3DXPlaneFromPoints(&pb, &v, &v2, &v3); Check("PlaneFromPoints", &pa.a, &pb.a, 4, 1e-3f);
		D3DXPlaneTransform(&pa, &pl, &a); Cy_D3DXPlaneTransform(&pb, &pl, &a); Check("PlaneTransform", &pa.a, &pb.a, 4);
		if (D3DXPlaneIntersectLine(&r1, &pl, &v, &v2) && Cy_D3DXPlaneIntersectLine(&r2, &pl, &v, &v2)) Check("PlaneIntersectLine", &r1.x, &r2.x, 3, 1e-2f);
		D3DXVECTOR4 lt(R(), R(), R(), 1);
		D3DXMatrixShadow(&o1, &lt, &pl); Cy_D3DXMatrixShadow(&o2, &lt, &pl); Check("MatrixShadow", M16(o1), M16(o2), 16, 1e-3f);
		D3DXMatrixReflect(&o1, &pl); Cy_D3DXMatrixReflect(&o2, &pl); Check("MatrixReflect", M16(o1), M16(o2), 16);
		D3DXVec3Hermite(&r1, &v, &v2, &v3, &v, 0.3f); Cy_D3DXVec3Hermite(&r2, &v, &v2, &v3, &v, 0.3f); Check("Vec3Hermite", &r1.x, &r2.x, 3);
		D3DXVec3CatmullRom(&r1, &v, &v2, &v3, &v, 0.3f); Cy_D3DXVec3CatmullRom(&r2, &v, &v2, &v3, &v, 0.3f); Check("Vec3CatmullRom", &r1.x, &r2.x, 3);
	}
	// Cynex: sifir vektor kenar durumlari
	D3DXVECTOR3 z(0, 0, 0), r1(9, 9, 9), r2(9, 9, 9);
	D3DXVec3Normalize(&r1, &z); Cy_D3DXVec3Normalize(&r2, &z);
	printf("Vec3Normalize(0): d3dx=(%g %g %g) cy=(%g %g %g)\n", r1.x, r1.y, r1.z, r2.x, r2.y, r2.z);
	D3DXMATRIX sing; memset(&sing, 0, sizeof(sing));
	printf("Inverse(singular): d3dx=%p cy=%p\n", (void*)D3DXMatrixInverse(&sing, NULL, &sing), (void*)Cy_D3DXMatrixInverse(&sing, NULL, &sing));
	printf("toplam %d karsilastirma, %d hata\n", g_total, g_fail);
	return g_fail ? 1 : 0;
}
