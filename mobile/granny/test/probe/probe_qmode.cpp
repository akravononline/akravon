// Cynex: kara kutu deneme - egri turune gore quaternion komsuluk kurali
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

int main()
{
	const char* dir = "C:/CynexMobilBuild/granny/testdata/warrior/";
	char p[512];
	sprintf(p, "%swarrior_novice.gr2", dir); granny_file_info* im = GrannyGetFileInfo(Load(p));
	sprintf(p, "%srun.gr2", dir); granny_animation* A = GrannyGetFileInfo(Load(p))->Animations[0];
	sprintf(p, "%sattack.gr2", dir); granny_animation* B = GrannyGetFileInfo(Load(p))->Animations[0];
	granny_model* mdl = im->Models[0];
	granny_skeleton* sk = mdl->Skeleton;
	const int bone = 1;
	const char* name = sk->Bones[bone].Name;
	granny_transform_track* tA = 0; granny_transform_track* tB = 0;
	for (int t = 0; t < A->TrackGroups[0]->TransformTrackCount; ++t) if (!strcmp(A->TrackGroups[0]->TransformTracks[t].Name, name)) tA = &A->TrackGroups[0]->TransformTracks[t];
	for (int t = 0; t < B->TrackGroups[0]->TransformTrackCount; ++t) if (!strcmp(B->TrackGroups[0]->TransformTracks[t].Name, name)) tB = &B->TrackGroups[0]->TransformTracks[t];
	granny_curve2 saved = tB->OrientationCurve;
	const float* rest = sk->Bones[bone].LocalTransform.Orientation;
	// Cynex: rest'e gore ters yarim kure, A ornegine gore ayni yarim kure olacak bir sabit
	granny_model_instance* mi = GrannyInstantiateModel(mdl);
	granny_local_pose* lp = GrannyNewLocalPose(sk->BoneCount);
	float qa[4];
	{
		granny_control* c = GrannyPlayControlledAnimation(0.0f, A, mi);
		GrannySetModelClock(mi, 0.3f);
		GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
		memcpy(qa, GrannyGetLocalPoseTransform(lp, bone)->Orientation, 16);
		GrannyFreeControl(c);
	}
	printf("rest (%g %g %g %g) qa (%g %g %g %g)\n", rest[0], rest[1], rest[2], rest[3], qa[0], qa[1], qa[2], qa[3]);
	float qc[4] = { -rest[0] * 0.8f + 0.1f, -rest[1] * 0.8f, -rest[2] * 0.8f - 0.2f, -rest[3] * 0.8f };
	float l = sqrtf(qc[0]*qc[0]+qc[1]*qc[1]+qc[2]*qc[2]+qc[3]*qc[3]);
	for (int k = 0; k < 4; ++k) qc[k] /= l;

	granny_curve2 k1; granny_curve_data_da_k32f_c32f d1; float knot0 = 0;
	GrannyCurveMakeStaticDaK32fC32f(&k1, &d1, 1, 0, 4, &knot0, qc);
	float knots2[2] = { 0, B->Duration }, ctrl2[8];
	memcpy(ctrl2, qc, 16); memcpy(ctrl2 + 4, qc, 16);
	granny_curve2 k2; granny_curve_data_da_k32f_c32f d2;
	GrannyCurveMakeStaticDaK32fC32f(&k2, &d2, 2, 1, 4, knots2, ctrl2);
	granny_curve_builder* bld = GrannyBeginCurve(GrannyCurveDataD4Constant32fType, 0, 4, 1);
	GrannyPushCurveKnotArray(bld, &knot0); GrannyPushCurveControlArray(bld, qc);
	granny_curve2* k3 = GrannyEndCurve(bld);
	granny_curve_data_da_keyframes32f kd; memset(&kd, 0, sizeof(kd)); kd.CurveDataHeader.Format = 0; kd.Dimension = 4; kd.ControlCount = 4; kd.Controls = qc;
	granny_curve2 k4; k4.CurveData.Type = GrannyCurveDataDaKeyframes32fType; k4.CurveData.Object = &kd;

	granny_curve2* tests[4] = { &k1, &k2, k3, &k4 };
	const char* names[4] = { "DaK32f 1knot", "DaK32f deg1", "D4Constant", "Keyframes" };
	for (int i = 0; i < 4; ++i)
	{
		tB->OrientationCurve = *tests[i];
		granny_control* ca = GrannyPlayControlledAnimation(0.0f, A, mi);
		granny_control* cb = GrannyPlayControlledAnimation(0.0f, B, mi);
		GrannySetControlWeight(ca, 0.6f); GrannySetControlWeight(cb, 0.4f);
		GrannySetModelClock(mi, 0.3f);
		GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
		const float* r = GrannyGetLocalPoseTransform(lp, bone)->Orientation;
		float sa = (qa[0]*rest[0]+qa[1]*rest[1]+qa[2]*rest[2]+qa[3]*rest[3]) < 0 ? -1.f : 1.f;
		float sc = (qc[0]*rest[0]+qc[1]*rest[1]+qc[2]*rest[2]+qc[3]*rest[3]) < 0 ? -1.f : 1.f;
		float nb[4], dr[4];
		for (int k = 0; k < 4; ++k) { nb[k] = 0.6f*sa*qa[k] + 0.4f*sc*qc[k]; dr[k] = 0.6f*sa*qa[k] + 0.4f*qc[k]; }
		float ln = sqrtf(nb[0]*nb[0]+nb[1]*nb[1]+nb[2]*nb[2]+nb[3]*nb[3]), ld = sqrtf(dr[0]*dr[0]+dr[1]*dr[1]+dr[2]*dr[2]+dr[3]*dr[3]);
		float en = 0, ed = 0;
		for (int k = 0; k < 4; ++k) { en = fmaxf(en, fminf(fabsf(nb[k]/ln - r[k]), fabsf(nb[k]/ln + r[k]))); ed = fmaxf(ed, fminf(fabsf(dr[k]/ld - r[k]), fabsf(dr[k]/ld + r[k]))); }
		printf("%-14s result (%.4f %.4f %.4f %.4f) err neighborhood %.2g direct %.2g\n", names[i], r[0], r[1], r[2], r[3], en, ed);
		GrannyFreeControl(ca); GrannyFreeControl(cb);
	}
	tB->OrientationCurve = saved;
	return 0;
}
