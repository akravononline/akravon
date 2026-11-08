// Cynex: kara kutu deneme - kucuk agirlik esigi, quaternion komsulugu, sira
#include <granny.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>

static granny_file* Load(const char* path)
{
	FILE* fp = fopen(path, "rb");
	fseek(fp, 0, SEEK_END); long n = ftell(fp); fseek(fp, 0, SEEK_SET);
	std::vector<char> buf(n); fread(buf.data(), 1, n, fp); fclose(fp);
	return GrannyReadEntireFileFromMemory((int)n, buf.data());
}

static void NegateOrientation(granny_animation* an, int trackIndex)
{
	granny_transform_track& tr = an->TrackGroups[0]->TransformTracks[trackIndex];
	if (GrannyCurveIsTypeDaK32fC32f(&tr.OrientationCurve))
	{
		granny_curve_data_da_k32f_c32f* d = GrannyCurveGetContentsOfDaK32fC32f(&tr.OrientationCurve);
		for (int i = 0; i < d->ControlCount; ++i) d->Controls[i] = -d->Controls[i];
	}
}

int main(int argc, char** argv)
{
	const char* dir = "C:/CynexMobilBuild/granny/testdata/warrior/";
	char p[512];
	sprintf(p, "%swarrior_novice.gr2", dir); granny_file_info* im = GrannyGetFileInfo(Load(p));
	sprintf(p, "%srun.gr2", dir); granny_animation* A = GrannyGetFileInfo(Load(p))->Animations[0];
	sprintf(p, "%sattack.gr2", dir); granny_animation* B = GrannyGetFileInfo(Load(p))->Animations[0];
	granny_model* pModel = im->Models[0];
	granny_skeleton* sk = pModel->Skeleton;
	granny_model_instance* mi = GrannyInstantiateModel(pModel);
	granny_local_pose* lp = GrannyNewLocalPose(sk->BoneCount);
	granny_local_pose* lpA = GrannyNewLocalPose(sk->BoneCount);

	// Cynex: esik - B agirligi kuculurken sonuc A ile ayni mi
	for (float wB : { 0.01f, 0.005f, 0.002f, 0.0011f, 0.001f, 0.0009f, 0.0005f, 0.0001f, 0.00001f })
	{
		granny_control* ca = GrannyPlayControlledAnimation(0.0f, A, mi);
		GrannySetModelClock(mi, 0.37f);
		GrannySampleModelAnimations(mi, 0, sk->BoneCount, lpA);
		granny_control* cb = GrannyPlayControlledAnimation(0.0f, B, mi);
		GrannySetControlWeight(cb, wB);
		GrannySetModelClock(mi, 0.37f);
		GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
		float d = 0;
		for (int b = 0; b < sk->BoneCount; ++b)
			for (int k = 0; k < 3; ++k) d = fmaxf(d, fabsf(GrannyGetLocalPoseTransform(lp, b)->Position[k] - GrannyGetLocalPoseTransform(lpA, b)->Position[k]));
		printf("wB=%g diff vs A-only = %g\n", wB, d);
		GrannyFreeControl(ca); GrannyFreeControl(cb);
	}
	// Cynex: tek kontrol, kucuk agirlik (fill ile)
	for (float wA : { 0.001f, 0.0009f, 0.0f })
	{
		granny_control* ca = GrannyPlayControlledAnimation(0.0f, A, mi);
		GrannySetControlWeight(ca, wA);
		GrannySetModelClock(mi, 0.37f);
		GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
		const granny_transform* r = GrannyGetLocalPoseTransform(lp, 5);
		printf("single wA=%g b5 p=(%g %g %g) rest=(%g %g %g)\n", wA, r->Position[0], r->Position[1], r->Position[2],
			sk->Bones[5].LocalTransform.Position[0], sk->Bones[5].LocalTransform.Position[1], sk->Bones[5].LocalTransform.Position[2]);
		GrannyFreeControl(ca);
	}
	// Cynex: quaternion komsulugu - B'nin bir kemiginin yonunu ters cevir
	{
		int bone = argc > 1 ? atoi(argv[1]) : 1;
		const char* name = sk->Bones[bone].Name;
		int tA = -1, tB = -1;
		for (int t = 0; t < A->TrackGroups[0]->TransformTrackCount; ++t) if (!strcmp(A->TrackGroups[0]->TransformTracks[t].Name, name)) tA = t;
		for (int t = 0; t < B->TrackGroups[0]->TransformTrackCount; ++t) if (!strcmp(B->TrackGroups[0]->TransformTracks[t].Name, name)) tB = t;
		auto Run = [&](const char* tag)
		{
			granny_control* ca = GrannyPlayControlledAnimation(0.0f, A, mi);
			granny_control* cb = GrannyPlayControlledAnimation(0.0f, B, mi);
			GrannySetControlWeight(ca, 0.6f); GrannySetControlWeight(cb, 0.3f);
			GrannySetModelClock(mi, 0.37f);
			GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
			const float* q = GrannyGetLocalPoseTransform(lp, bone)->Orientation;
			printf("%-12s q=(%.6f %.6f %.6f %.6f)\n", tag, q[0], q[1], q[2], q[3]);
			GrannyFreeControl(ca); GrannyFreeControl(cb);
		};
		printf("rest q=(%g %g %g %g)\n", sk->Bones[bone].LocalTransform.Orientation[0], sk->Bones[bone].LocalTransform.Orientation[1], sk->Bones[bone].LocalTransform.Orientation[2], sk->Bones[bone].LocalTransform.Orientation[3]);
		Run("normal");
		NegateOrientation(B, tB); Run("negB");
		NegateOrientation(A, tA); Run("negA+B");
		NegateOrientation(B, tB); Run("negA");
		NegateOrientation(A, tA);
		// Cynex: sira etkisi - once B sonra A oynat
		granny_control* cb = GrannyPlayControlledAnimation(0.0f, B, mi);
		granny_control* ca = GrannyPlayControlledAnimation(0.0f, A, mi);
		GrannySetControlWeight(ca, 0.6f); GrannySetControlWeight(cb, 0.3f);
		GrannySetModelClock(mi, 0.37f);
		GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
		const float* q = GrannyGetLocalPoseTransform(lp, bone)->Orientation;
		printf("%-12s q=(%.6f %.6f %.6f %.6f)\n", "orderBA", q[0], q[1], q[2], q[3]);
		GrannyFreeControl(ca); GrannyFreeControl(cb);
		// Cynex: tek kontrol, dusuk agirlik + rest fill, ters yon
		NegateOrientation(A, tA);
		ca = GrannyPlayControlledAnimation(0.0f, A, mi);
		GrannySetControlWeight(ca, 0.1f);
		GrannySetModelClock(mi, 0.37f);
		GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
		q = GrannyGetLocalPoseTransform(lp, bone)->Orientation;
		printf("%-12s q=(%.6f %.6f %.6f %.6f)\n", "negA w.1", q[0], q[1], q[2], q[3]);
		NegateOrientation(A, tA);
		GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
		q = GrannyGetLocalPoseTransform(lp, bone)->Orientation;
		printf("%-12s q=(%.6f %.6f %.6f %.6f)\n", "A w.1", q[0], q[1], q[2], q[3]);
		GrannyFreeControl(ca);
		// Cynex: tek kontrol tam agirlik, ters yon: isaret korunuyor mu
		NegateOrientation(A, tA);
		ca = GrannyPlayControlledAnimation(0.0f, A, mi);
		GrannySetModelClock(mi, 0.37f);
		GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
		q = GrannyGetLocalPoseTransform(lp, bone)->Orientation;
		printf("%-12s q=(%.6f %.6f %.6f %.6f)\n", "negA w1", q[0], q[1], q[2], q[3]);
		GrannyFreeControl(ca);
		NegateOrientation(A, tA);
	}
	// Cynex: hic kontrol yok
	GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
	printf("nocontrol b1 p=(%g %g %g) flags=%u\n", GrannyGetLocalPoseTransform(lp, 1)->Position[0], GrannyGetLocalPoseTransform(lp, 1)->Position[1], GrannyGetLocalPoseTransform(lp, 1)->Position[2], GrannyGetLocalPoseTransform(lp, 1)->Flags);
	return 0;
}
