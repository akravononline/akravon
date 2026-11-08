// Cynex: kara kutu deneme - sabit egrilerle quaternion isaret kurali (rastgele ciftler)
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
static void RandQ(float* q) { float l; do { for (int k = 0; k < 4; ++k) q[k] = (rand() / (float) RAND_MAX) * 2 - 1; l = sqrtf(Dot(q, q)); } while (l < 0.2f || l > 1); for (int k = 0; k < 4; ++k) q[k] /= l; }

int main(int argc, char** argv)
{
	granny_file_info* im = GrannyGetFileInfo(Load(argv[1]));
	granny_animation* A = GrannyGetFileInfo(Load(argv[2]))->Animations[0];
	granny_animation* B = GrannyGetFileInfo(Load(argv[3]))->Animations[0];
	const int bone = atoi(argv[4]);
	granny_model* mdl = im->Models[0];
	granny_skeleton* sk = mdl->Skeleton;
	granny_int32x ga = 0, gb = 0;
	GrannyFindTrackGroupForModel(A, mdl->Name, &ga); GrannyFindTrackGroupForModel(B, mdl->Name, &gb);
	granny_transform_track* tA = 0; granny_transform_track* tB = 0;
	for (int t = 0; t < A->TrackGroups[ga]->TransformTrackCount; ++t) if (!strcmp(A->TrackGroups[ga]->TransformTracks[t].Name, sk->Bones[bone].Name)) tA = &A->TrackGroups[ga]->TransformTracks[t];
	for (int t = 0; t < B->TrackGroups[gb]->TransformTrackCount; ++t) if (!strcmp(B->TrackGroups[gb]->TransformTracks[t].Name, sk->Bones[bone].Name)) tB = &B->TrackGroups[gb]->TransformTracks[t];
	if (!tA || !tB) { printf("track yok\n"); return 1; }
	const float* rest = sk->Bones[bone].LocalTransform.Orientation;
	printf("bone %s rest (%g %g %g %g) flags %u\n", sk->Bones[bone].Name, rest[0], rest[1], rest[2], rest[3], sk->Bones[bone].LocalTransform.Flags);
	granny_model_instance* mi = GrannyInstantiateModel(mdl);
	granny_local_pose* lp = GrannyNewLocalPose(sk->BoneCount);
	srand(1234);
	int agreeRest = 0, agreeAccum = 0, agreeIdent = 0, total = 0;
	float qa[4], qb[4];
	float knot = 0;
	for (int it = 0; it < 300; ++it)
	{
		RandQ(qa); RandQ(qb);
		granny_curve2 ca, cb; granny_curve_data_da_k32f_c32f da, db;
		GrannyCurveMakeStaticDaK32fC32f(&ca, &da, 1, 0, 4, &knot, qa);
		GrannyCurveMakeStaticDaK32fC32f(&cb, &db, 1, 0, 4, &knot, qb);
		granny_curve2 sa = tA->OrientationCurve, sb = tB->OrientationCurve;
		tA->OrientationCurve = ca; tB->OrientationCurve = cb;
		granny_control* c1 = GrannyPlayControlledAnimation(0.0f, A, mi);
		granny_control* c2 = GrannyPlayControlledAnimation(0.0f, B, mi);
		GrannySetControlWeight(c1, 0.6f); GrannySetControlWeight(c2, 0.4f);
		GrannySetModelClock(mi, 0.1f);
		GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
		const float* r = GrannyGetLocalPoseTransform(lp, bone)->Orientation;
		float e[2];
		for (int rel = 0; rel < 2; ++rel)
		{
			float acc[4]; float s = rel ? -1.f : 1.f;
			for (int k = 0; k < 4; ++k) acc[k] = 0.6f * qa[k] + s * 0.4f * qb[k];
			float l = sqrtf(Dot(acc, acc)); e[rel] = 0;
			for (int k = 0; k < 4; ++k) e[rel] = fmaxf(e[rel], fminf(fabsf(acc[k] / l - r[k]), fabsf(acc[k] / l + r[k])));
		}
		int obsRel = e[0] < e[1] ? 1 : -1;
		const float ident[4] = { 0, 0, 0, 1 };
		int hRest = (Dot(qa, rest) < 0) == (Dot(qb, rest) < 0) ? 1 : -1;
		int hAcc = Dot(qa, qb) < 0 ? -1 : 1;
		int hId = (Dot(qa, ident) < 0) == (Dot(qb, ident) < 0) ? 1 : -1;
		agreeRest += obsRel == hRest; agreeAccum += obsRel == hAcc; agreeIdent += obsRel == hId; ++total;
		if (argc > 5 && obsRel != hRest) printf("  diff: qa.rest %+.3f qb.rest %+.3f qa.qb %+.3f obs %d\n", Dot(qa, rest), Dot(qb, rest), Dot(qa, qb), obsRel);
		GrannyFreeControl(c1); GrannyFreeControl(c2);
		tA->OrientationCurve = sa; tB->OrientationCurve = sb;
	}
	printf("total %d rest %d accum %d ident %d\n", total, agreeRest, agreeAccum, agreeIdent);
	return 0;
}
