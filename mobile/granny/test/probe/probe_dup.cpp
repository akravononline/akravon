// Cynex: kara kutu deneme - ayni isimli kemik/track ve bos egri
#include <granny.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

static granny_file* Load(const char* path)
{
	FILE* fp = fopen(path, "rb"); fseek(fp, 0, SEEK_END); long n = ftell(fp); fseek(fp, 0, SEEK_SET);
	std::vector<char> buf(n); fread(buf.data(), 1, n, fp); fclose(fp);
	return GrannyReadEntireFileFromMemory((int)n, buf.data());
}

static void Print(const char* tag, const granny_transform* t)
{
	printf("%-22s f=%u p=(%.4f %.4f %.4f) q=(%.4f %.4f %.4f %.4f) s=(%.3f %.3f %.3f)\n", tag, t->Flags, t->Position[0], t->Position[1], t->Position[2],
		t->Orientation[0], t->Orientation[1], t->Orientation[2], t->Orientation[3], t->ScaleShear[0][0], t->ScaleShear[1][1], t->ScaleShear[2][2]);
}

int main(int argc, char** argv)
{
	granny_file_info* im = GrannyGetFileInfo(Load(argv[1]));
	granny_file_info* ia = GrannyGetFileInfo(Load(argv[2]));
	float clk = argc > 3 ? (float) atof(argv[3]) : 0.0167f;
	granny_model* mdl = im->Models[0];
	granny_skeleton* sk = mdl->Skeleton;
	granny_animation* an = ia->Animations[0];
	granny_int32x gi = 0;
	GrannyFindTrackGroupForModel(an, mdl->Name, &gi);
	granny_track_group* g = an->TrackGroups[gi];
	granny_model_instance* mi = GrannyInstantiateModel(mdl);
	granny_control* c = GrannyPlayControlledAnimation(0.0f, an, mi);
	GrannySetControlLoopCount(c, 0);
	GrannySetModelClock(mi, clk);
	granny_local_pose* lp = GrannyNewLocalPose(sk->BoneCount);
	GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
	for (int b = 0; b < sk->BoneCount; ++b)
	{
		int dupBone = 0, dupTrack = 0;
		for (int o = 0; o < sk->BoneCount; ++o) if (o != b && !strcmp(sk->Bones[o].Name, sk->Bones[b].Name)) ++dupBone;
		for (int t = 0; t < g->TransformTrackCount; ++t) if (!strcmp(g->TransformTracks[t].Name, sk->Bones[b].Name)) ++dupTrack;
		bool emptyCurve = false;
		for (int t = 0; t < g->TransformTrackCount; ++t)
			if (!strcmp(g->TransformTracks[t].Name, sk->Bones[b].Name))
			{
				const granny_curve2* cs[3] = { &g->TransformTracks[t].PositionCurve, &g->TransformTracks[t].OrientationCurve, &g->TransformTracks[t].ScaleShearCurve };
				for (int k = 0; k < 3; ++k)
				{
					const granny_curve_data_header* h = (const granny_curve_data_header*) cs[k]->CurveData.Object;
					if (h && h->Format == 1 && ((const granny_curve_data_da_k32f_c32f*) h)->KnotCount == 0) emptyCurve = true;
				}
			}
		if (!dupBone && dupTrack <= 1 && !emptyCurve)
			continue;
		char tag[64];
		sprintf(tag, "bone %d %s", b, sk->Bones[b].Name);
		Print(tag, GrannyGetLocalPoseTransform(lp, b));
		Print("   rest", &sk->Bones[b].LocalTransform);
		for (int t = 0; t < g->TransformTrackCount; ++t)
		{
			if (strcmp(g->TransformTracks[t].Name, sk->Bones[b].Name))
				continue;
			granny_transform_track& tr = g->TransformTracks[t];
			granny_transform e;
			e.Flags = 0;
			float lc = GrannyGetControlClampedLocalClock(c);
			GrannyEvaluateCurveAtT(3, false, true, &tr.PositionCurve, true, an->Duration, lc, e.Position, GrannyCurveIdentityPosition);
			GrannyEvaluateCurveAtT(4, true, true, &tr.OrientationCurve, true, an->Duration, lc, e.Orientation, GrannyCurveIdentityOrientation);
			GrannyEvaluateCurveAtT(9, false, true, &tr.ScaleShearCurve, true, an->Duration, lc, &e.ScaleShear[0][0], GrannyCurveIdentityScaleShear);
			sprintf(tag, "   track %d", t);
			Print(tag, &e);
		}
	}
	return 0;
}
