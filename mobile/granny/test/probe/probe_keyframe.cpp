// Cynex: kara kutu deneme - DaKeyframes32f orneklemede hangi kare kullaniliyor
#include <granny.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>

static granny_file* Load(const char* path)
{
	FILE* fp = fopen(path, "rb");
	if (!fp) { printf("yok %s\n", path); exit(1); }
	fseek(fp, 0, SEEK_END); long n = ftell(fp); fseek(fp, 0, SEEK_SET);
	std::vector<char> buf(n); fread(buf.data(), 1, n, fp); fclose(fp);
	return GrannyReadEntireFileFromMemory((int)n, buf.data());
}

int main(int argc, char** argv)
{
	granny_file_info* im = GrannyGetFileInfo(Load(argv[1]));
	granny_file_info* ia = GrannyGetFileInfo(Load(argv[2]));
	granny_model* mdl = im->Models[0];
	granny_animation* an = ia->Animations[0];
	granny_skeleton* sk = mdl->Skeleton;
	granny_int32x gi = 0;
	GrannyFindTrackGroupForModel(an, mdl->Name, &gi);
	granny_track_group* g = an->TrackGroups[gi];
	int ti = -1, bone = -1;
	for (int t = 0; t < g->TransformTrackCount && ti < 0; ++t)
	{
		const granny_curve_data_header* h = (const granny_curve_data_header*) g->TransformTracks[t].PositionCurve.CurveData.Object;
		if (h && h->Format == 0 && GrannyFindBoneByName(sk, g->TransformTracks[t].Name, &bone))
			ti = t;
	}
	if (ti < 0) { printf("keyframe track yok\n"); return 0; }
	granny_transform_track& tr = g->TransformTracks[ti];
	const granny_curve_data_da_keyframes32f* kf = (const granny_curve_data_da_keyframes32f*) tr.PositionCurve.CurveData.Object;
	printf("track '%s' bone %d dim %d controls %d (frames %d) dur %g timestep %g oversampling %g\n", tr.Name, bone, kf->Dimension, kf->ControlCount, kf->ControlCount / kf->Dimension, an->Duration, an->TimeStep, an->Oversampling);
	const int frames = kf->ControlCount / kf->Dimension;
	for (int f = 0; f < frames && f < 30; ++f)
		printf("  key %d: %g %g %g\n", f, kf->Controls[f * 3], kf->Controls[f * 3 + 1], kf->Controls[f * 3 + 2]);
	granny_model_instance* mi = GrannyInstantiateModel(mdl);
	granny_control* c = GrannyPlayControlledAnimation(0.0f, an, mi);
	GrannySetControlLoopCount(c, 0);
	granny_local_pose* lp = GrannyNewLocalPose(sk->BoneCount);
	for (float t = 0.0f; t <= an->Duration + 0.001f; t += an->TimeStep / 4.0f)
	{
		GrannySetModelClock(mi, t);
		GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
		const float* p = GrannyGetLocalPoseTransform(lp, bone)->Position;
		int match = -1;
		for (int f = 0; f < frames; ++f)
			if (fabsf(kf->Controls[f * 3] - p[0]) < 1e-4f && fabsf(kf->Controls[f * 3 + 1] - p[1]) < 1e-4f && fabsf(kf->Controls[f * 3 + 2] - p[2]) < 1e-4f) { match = f; break; }
		float e[3];
		GrannyEvaluateCurveAtT(3, false, false, &tr.PositionCurve, false, an->Duration, GrannyGetControlClampedLocalClock(c), e, GrannyCurveIdentityPosition);
		printf("t=%.4f lc=%.4f t/step=%.3f -> p=(%g %g %g) key=%d | evalAtT=(%g %g %g)\n", t, GrannyGetControlClampedLocalClock(c), GrannyGetControlClampedLocalClock(c) / an->TimeStep, p[0], p[1], p[2], match, e[0], e[1], e[2]);
	}
	return 0;
}
