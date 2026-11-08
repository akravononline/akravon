// Cynex: kara kutu deneme - ornekleme, karisim, kok hareketi
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

static void PrintT(const char* tag, const granny_transform* t)
{
	printf("%s f=%u p=(%.6f %.6f %.6f) q=(%.6f %.6f %.6f %.6f) ss=(%.4f %.4f %.4f | %.4f %.4f %.4f | %.4f %.4f %.4f)\n", tag, t->Flags,
		t->Position[0], t->Position[1], t->Position[2], t->Orientation[0], t->Orientation[1], t->Orientation[2], t->Orientation[3],
		t->ScaleShear[0][0], t->ScaleShear[0][1], t->ScaleShear[0][2], t->ScaleShear[1][0], t->ScaleShear[1][1], t->ScaleShear[1][2], t->ScaleShear[2][0], t->ScaleShear[2][1], t->ScaleShear[2][2]);
}

int main(int argc, char** argv)
{
	const char* model = argc > 1 ? argv[1] : "C:/CynexMobilBuild/granny/testdata/warrior/warrior_novice.gr2";
	const char* animA = argc > 2 ? argv[2] : "C:/CynexMobilBuild/granny/testdata/warrior/run.gr2";
	const char* animB = argc > 3 ? argv[3] : "C:/CynexMobilBuild/granny/testdata/warrior/wait.gr2";
	granny_file_info* im = GrannyGetFileInfo(Load(model));
	granny_file_info* ia = GrannyGetFileInfo(Load(animA));
	granny_file_info* ib = GrannyGetFileInfo(Load(animB));
	granny_model* pModel = im->Models[0];
	granny_skeleton* sk = pModel->Skeleton;
	granny_animation* A = ia->Animations[0];
	granny_animation* B = ib->Animations[0];
	for (granny_animation* an : { A, B })
	{
		granny_track_group* g = an->TrackGroups[0];
		printf("anim dur=%g tracks=%d flags=0x%X looptrans=(%g %g %g) periodic=%p name=%s\n", an->Duration, g->TransformTrackCount, g->Flags,
			g->LoopTranslation[0], g->LoopTranslation[1], g->LoopTranslation[2], (void*) g->PeriodicLoop, g->Name);
		PrintT("  initialplacement", &g->InitialPlacement);
		for (int t = 0; t < g->TransformTrackCount && t < 3; ++t)
		{
			granny_transform_track& tr = g->TransformTracks[t];
			printf("  track %d %s flags=%d posknots=%d deg=%d oriknots=%d deg=%d\n", t, tr.Name, tr.Flags,
				GrannyCurveGetKnotCount(&tr.PositionCurve), GrannyCurveGetDegree(&tr.PositionCurve),
				GrannyCurveGetKnotCount(&tr.OrientationCurve), GrannyCurveGetDegree(&tr.OrientationCurve));
			if (GrannyCurveIsTypeDaK32fC32f(&tr.PositionCurve))
			{
				granny_curve_data_da_k32f_c32f* d = GrannyCurveGetContentsOfDaK32fC32f(&tr.PositionCurve);
				printf("   pos knots: first=%g last=%g count=%d ctrl0=(%g %g %g) ctrlN=(%g %g %g)\n", d->Knots[0], d->Knots[d->KnotCount - 1], d->KnotCount,
					d->Controls[0], d->Controls[1], d->Controls[2], d->Controls[d->ControlCount - 3], d->Controls[d->ControlCount - 2], d->Controls[d->ControlCount - 1]);
			}
		}
	}
	printf("skeleton bones=%d\n", sk->BoneCount);
	for (int b = 0; b < sk->BoneCount && b < 3; ++b)
	{
		printf("bone %d %s parent=%d\n", b, sk->Bones[b].Name, sk->Bones[b].ParentIndex);
		PrintT("  rest", &sk->Bones[b].LocalTransform);
	}

	granny_model_instance* mi = GrannyInstantiateModel(pModel);
	granny_local_pose* lp = GrannyNewLocalPose(sk->BoneCount);

	// Cynex: tek kontrol, elle egri degerlendirme ile kiyas
	{
		granny_control* c = GrannyPlayControlledAnimation(0.0f, A, mi);
		GrannySetControlLoopCount(c, 0);
		float clocks[] = { 0.0f, 0.1f, 0.3333f, 0.6f, 0.6666f, 0.7f, 1.0f, 2.05f };
		for (float clk : clocks)
		{
			GrannySetModelClock(mi, clk);
			GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
			float lc = GrannyGetControlClampedLocalClock(c);
			granny_bool32 u, o;
			GrannyGetControlLoopState(c, &u, &o);
			float maxd[3] = { 0, 0, 0 };
			int maxb = -1;
			granny_track_group* g = A->TrackGroups[0];
			for (int b = 0; b < sk->BoneCount; ++b)
			{
				int ti = -1;
				for (int t = 0; t < g->TransformTrackCount; ++t)
					if (!_stricmp(g->TransformTracks[t].Name, sk->Bones[b].Name)) { ti = t; break; }
				granny_transform* r = GrannyGetLocalPoseTransform(lp, b);
				if (ti < 0) continue;
				granny_transform_track& tr = g->TransformTracks[ti];
				float p[3], q[4], ss[9];
				GrannyEvaluateCurveAtT(3, false, u != 0, &tr.PositionCurve, o != 0, A->Duration, lc, p, GrannyCurveIdentityPosition);
				GrannyEvaluateCurveAtT(4, true, u != 0, &tr.OrientationCurve, o != 0, A->Duration, lc, q, GrannyCurveIdentityOrientation);
				GrannyEvaluateCurveAtT(9, false, u != 0, &tr.ScaleShearCurve, o != 0, A->Duration, lc, ss, GrannyCurveIdentityScaleShear);
				for (int k = 0; k < 3; ++k) maxd[0] = fmaxf(maxd[0], fabsf(p[k] - r->Position[k]));
				float dot = q[0]*r->Orientation[0] + q[1]*r->Orientation[1] + q[2]*r->Orientation[2] + q[3]*r->Orientation[3];
				float sgn = dot < 0 ? -1.0f : 1.0f;
				for (int k = 0; k < 4; ++k) maxd[1] = fmaxf(maxd[1], fabsf(sgn*q[k] - r->Orientation[k]));
				for (int k = 0; k < 9; ++k) maxd[2] = fmaxf(maxd[2], fabsf(ss[k] - (&r->ScaleShear[0][0])[k]));
				if (b == 0 && (clk == 0.1f)) { PrintT("  sampled b0", r); printf("  manual q=(%g %g %g %g) p=(%g %g %g)\n", q[0], q[1], q[2], q[3], p[0], p[1], p[2]); }
				if (dot < 0 && maxb < 0) maxb = b;
			}
			printf("clk %.4f lc %.4f u%d o%d: maxdiff pos %.3g ori %.3g ss %.3g  firstNegDotBone=%d\n", clk, lc, u, o, maxd[0], maxd[1], maxd[2], maxb);
		}
		// Cynex: kok hareketi
		for (float clk = 0.0f; clk < 2.0f; clk += 0.25f)
		{
			GrannySetModelClock(mi, clk);
			float tr[3], rot[3];
			GrannyGetRootMotionVectors(mi, 0.1f, tr, rot, false);
			float m[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 }, out[16];
			GrannyUpdateModelMatrix(mi, 0.1f, m, out, false);
			printf("root clk %.3f lc %.4f: tr=(%g %g %g) rot=(%g %g %g) mat t=(%g %g %g)\n", clk, GrannyGetControlRawLocalClock(c), tr[0], tr[1], tr[2], rot[0], rot[1], rot[2], out[12], out[13], out[14]);
		}
		GrannyFreeControl(c);
	}
	return 0;
}
