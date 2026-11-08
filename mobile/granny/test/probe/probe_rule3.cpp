// Cynex: kara kutu deneme - spline orneginin isareti P0'a mi sonuca mi gore secilir, keyframe nasil karisir
#include <granny.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>

static granny_file* Load(const char* path)
{
	FILE* fp = fopen(path, "rb"); fseek(fp, 0, SEEK_END); long n = ftell(fp); fseek(fp, 0, SEEK_SET);
	std::vector<char> buf(n); fread(buf.data(), 1, n, fp); fclose(fp);
	return GrannyReadEntireFileFromMemory((int)n, buf.data());
}
static float Dot(const float* a, const float* b) { return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3]; }
static void Norm(float* q) { float l = sqrtf(Dot(q, q)); for (int k = 0; k < 4; ++k) q[k] /= l; }

int main()
{
	const char* dir = "C:/CynexMobilBuild/granny/testdata/warrior/";
	char p[512];
	sprintf(p, "%swarrior_novice.gr2", dir); granny_file_info* im = GrannyGetFileInfo(Load(p));
	sprintf(p, "%srun.gr2", dir); granny_animation* A = GrannyGetFileInfo(Load(p))->Animations[0];
	sprintf(p, "%sattack.gr2", dir); granny_animation* B = GrannyGetFileInfo(Load(p))->Animations[0];
	granny_model* mdl = im->Models[0];
	granny_skeleton* sk = mdl->Skeleton;
	const int bone = 0;
	granny_transform_track* tA = &A->TrackGroups[0]->TransformTracks[0];
	granny_transform_track* tB = &B->TrackGroups[0]->TransformTracks[0];
	printf("bone %s track A %s B %s rest (%g %g %g %g)\n", sk->Bones[bone].Name, tA->Name, tB->Name, sk->Bones[bone].LocalTransform.Orientation[0], sk->Bones[bone].LocalTransform.Orientation[1], sk->Bones[bone].LocalTransform.Orientation[2], sk->Bones[bone].LocalTransform.Orientation[3]);
	const float* rest = sk->Bones[bone].LocalTransform.Orientation;
	granny_model_instance* mi = GrannyInstantiateModel(mdl);
	granny_local_pose* lp = GrannyNewLocalPose(sk->BoneCount);

	// Cynex: P0 rest tarafinda (w>0), sonuc karsi tarafta (w<0)
	float q0[4] = { 0, 0, 0.98f, -0.2f }, q1[4] = { 0.3f, 0, 0.75f, 0.6f };
	Norm(q0); Norm(q1);
	float knots[2] = { 0.0f, A->Duration }, ctrls[8];
	memcpy(ctrls, q0, 16); memcpy(ctrls + 4, q1, 16);
	float qb[4] = { 0.2f, 0.1f, -0.3f, 0.92f }; Norm(qb);
	float knot0 = 0;

	for (int test = 0; test < 3; ++test)
	{
		granny_curve2 ca, cb; granny_curve_data_da_k32f_c32f da, db;
		granny_curve_data_da_keyframes32f kb; memset(&kb, 0, sizeof(kb));
		GrannyCurveMakeStaticDaK32fC32f(&ca, &da, 2, 1, 4, knots, ctrls);
		if (test == 0) GrannyCurveMakeStaticDaK32fC32f(&cb, &db, 1, 0, 4, &knot0, qb);
		else
		{
			// Cynex: gercek keyframe egrisi taklidi - kare sayisi animasyon suresi kadar
			static std::vector<float> keys;
			int frames = (int) (B->Duration / B->TimeStep) + 1;
			keys.resize(frames * 4);
			for (int f = 0; f < frames; ++f) memcpy(&keys[f * 4], qb, 16);
			if (test == 2) for (int f = 0; f < frames; ++f) for (int k = 0; k < 4; ++k) keys[f * 4 + k] = -qb[k];
			kb.CurveDataHeader.Format = 0; kb.CurveDataHeader.Degree = 0; kb.Dimension = 4; kb.ControlCount = frames * 4; kb.Controls = keys.data();
			cb.CurveData.Type = GrannyCurveDataDaKeyframes32fType; cb.CurveData.Object = &kb;
		}
		granny_curve2 sa = tA->OrientationCurve, sb = tB->OrientationCurve;
		tA->OrientationCurve = ca; tB->OrientationCurve = cb;
		for (float lt : { 0.1f, 0.9f })
		{
			granny_control* c1 = GrannyPlayControlledAnimation(0.0f, A, mi);
			granny_control* c2 = GrannyPlayControlledAnimation(0.0f, B, mi);
			GrannySetControlWeight(c1, 0.6f); GrannySetControlWeight(c2, 0.4f);
			float clk = lt * A->Duration;
			GrannySetModelClock(mi, clk);
			GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
			const float* r = GrannyGetLocalPoseTransform(lp, bone)->Orientation;
			float qa[4];
			GrannyEvaluateCurveAtT(4, true, false, &ca, false, A->Duration, clk, qa, GrannyCurveIdentityOrientation);
			float qbs[4];
			memcpy(qbs, test == 2 ? qb : qb, 16);
			if (test == 2) for (int k = 0; k < 4; ++k) qbs[k] = -qb[k];
			float e[2];
			for (int rel = 0; rel < 2; ++rel)
			{
				float acc[4]; for (int k = 0; k < 4; ++k) acc[k] = 0.6f * qa[k] + (rel ? -1.f : 1.f) * 0.4f * qbs[k];
				float l = sqrtf(Dot(acc, acc)); e[rel] = 0;
				for (int k = 0; k < 4; ++k) e[rel] = fmaxf(e[rel], fminf(fabsf(acc[k] / l - r[k]), fabsf(acc[k] / l + r[k])));
			}
			printf("test %d t=%.1f qa.rest %+.3f P0.rest %+.3f qb.rest %+.3f qa.qb %+.3f -> rel %s (e %.2g / %.2g) r.rest %+.3f\n", test, lt, Dot(qa, rest), Dot(q0, rest), Dot(qbs, rest), Dot(qa, qbs),
				e[0] < e[1] ? "same" : "flipped", e[0], e[1], Dot(r, rest));
			GrannyFreeControl(c1); GrannyFreeControl(c2);
		}
		tA->OrientationCurve = sa; tB->OrientationCurve = sb;
	}
	return 0;
}
