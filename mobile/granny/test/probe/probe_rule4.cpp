// Cynex: kara kutu deneme - spline isaret karari pencerenin hangi noktasina bagli (derece 1/2, farkli u)
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
	granny_transform_track* tA = &A->TrackGroups[0]->TransformTracks[0];
	granny_transform_track* tB = &B->TrackGroups[0]->TransformTracks[0];
	const float rest[4] = { 0, 0, 0, 1 };
	granny_model_instance* mi = GrannyInstantiateModel(mdl);
	granny_local_pose* lp = GrannyNewLocalPose(sk->BoneCount);
	float qb[4] = { 0.2f, 0.1f, -0.3f, 0.92f }; Norm(qb);
	float knot0 = 0;
	const float D = A->Duration;

	// Cynex: kontrol noktalari w: +0.3, 0.0x, -0.6, -0.8 (zincir uyumlu)
	float pts[5][4] = { { 0, 0, 0.95f, 0.3f }, { 0.2f, 0, 0.97f, 0.02f }, { 0.3f, 0, 0.75f, -0.6f }, { 0.2f, 0.1f, 0.55f, -0.8f }, { 0.1f, 0.2f, 0.4f, -0.9f } };
	for (auto& q : pts) Norm(q);
	for (int degree = 1; degree <= 2; ++degree)
	{
		const int n = 5;
		float knots[5] = { 0, 0.25f * D, 0.5f * D, 0.75f * D, D };
		float ctrls[20];
		for (int i = 0; i < n; ++i) memcpy(ctrls + 4 * i, pts[i], 16);
		granny_curve2 ca, cb; granny_curve_data_da_k32f_c32f da, db;
		GrannyCurveMakeStaticDaK32fC32f(&ca, &da, n, degree, 4, knots, ctrls);
		GrannyCurveMakeStaticDaK32fC32f(&cb, &db, 1, 0, 4, &knot0, qb);
		granny_curve2 sa = tA->OrientationCurve, sb = tB->OrientationCurve;
		tA->OrientationCurve = ca; tB->OrientationCurve = cb;
		for (float u = 0.02f; u < 1.0f; u += 0.08f)
		{
			float clk = u * D;
			granny_control* c1 = GrannyPlayControlledAnimation(0.0f, A, mi);
			granny_control* c2 = GrannyPlayControlledAnimation(0.0f, B, mi);
			GrannySetControlLoopCount(c1, getenv("LOOP0") ? 0 : 1);
			GrannySetControlWeight(c1, 0.6f); GrannySetControlWeight(c2, 0.4f);
			GrannySetModelClock(mi, clk);
			GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
			const float* r = GrannyGetLocalPoseTransform(lp, 0)->Orientation;
			float qa[4];
			GrannyEvaluateCurveAtT(4, true, false, &ca, false, D, clk, qa, GrannyCurveIdentityOrientation);
			int k = GrannyCurveFindKnot(&ca, clk);
			float e[2];
			for (int rel = 0; rel < 2; ++rel)
			{
				float acc[4]; for (int i = 0; i < 4; ++i) acc[i] = 0.6f * qa[i] + (rel ? -1.f : 1.f) * 0.4f * qb[i];
				float l = sqrtf(Dot(acc, acc)); e[rel] = 0;
				for (int i = 0; i < 4; ++i) e[rel] = fmaxf(e[rel], fminf(fabsf(acc[i] / l - r[i]), fabsf(acc[i] / l + r[i])));
			}
			printf("deg %d u=%.2f k=%d window w:", degree, u, k);
			for (int j = k - degree; j <= k; ++j) printf(" %+.2f", pts[j < 0 ? 0 : (j > 4 ? 4 : j)][3]);
			printf(" | qa.rest %+.3f -> A %s\n", Dot(qa, rest), e[0] < e[1] ? "kept" : "FLIPPED");
			GrannyFreeControl(c1); GrannyFreeControl(c2);
		}
		tA->OrientationCurve = sa; tB->OrientationCurve = sb;
	}
	return 0;
}
