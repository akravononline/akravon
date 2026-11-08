// Cynex: kara kutu deneme - iki kontrol karisimi ve kok hareketi kenar durumlari
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

struct T { float p[3], q[4], ss[9]; };

static bool SampleOne(granny_control* c, granny_animation* an, const char* bone, T& out)
{
	granny_track_group* g = an->TrackGroups[0];
	for (int t = 0; t < g->TransformTrackCount; ++t)
		if (!strcmp(g->TransformTracks[t].Name, bone))
		{
			granny_transform_track& tr = g->TransformTracks[t];
			float lc = GrannyGetControlClampedLocalClock(c);
			granny_bool32 u, o; GrannyGetControlLoopState(c, &u, &o);
			GrannyEvaluateCurveAtT(3, false, u != 0, &tr.PositionCurve, o != 0, an->Duration, lc, out.p, GrannyCurveIdentityPosition);
			GrannyEvaluateCurveAtT(4, true, u != 0, &tr.OrientationCurve, o != 0, an->Duration, lc, out.q, GrannyCurveIdentityOrientation);
			GrannyEvaluateCurveAtT(9, false, u != 0, &tr.ScaleShearCurve, o != 0, an->Duration, lc, out.ss, GrannyCurveIdentityScaleShear);
			return true;
		}
	return false;
}

int main()
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
	printf("fill threshold default=%g\n", GrannyGetLocalPoseFillThreshold(lp));

	float weightsA[] = { 1.0f, 0.7f, 0.3f, 0.05f, 0.0f };
	for (float wA : weightsA)
		for (float wB : { 1.0f, 0.4f, 0.0f, 0.001f })
		{
			granny_control* ca = GrannyPlayControlledAnimation(0.0f, A, mi);
			granny_control* cb = GrannyPlayControlledAnimation(0.0f, B, mi);
			GrannySetControlLoopCount(ca, 0);
			GrannySetControlWeight(ca, wA);
			GrannySetControlWeight(cb, wB);
			GrannySetModelClock(mi, 0.37f);
			GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
			float err[4][3] = {};
			for (int b = 0; b < sk->BoneCount; ++b)
			{
				T ta, tb;
				bool hasA = SampleOne(ca, A, sk->Bones[b].Name, ta);
				bool hasB = SampleOne(cb, B, sk->Bones[b].Name, tb);
				const granny_transform& rest = sk->Bones[b].LocalTransform;
				granny_transform* r = GrannyGetLocalPoseTransform(lp, b);
				for (int cand = 0; cand < 4; ++cand)
				{
					float tw = 0, pp[3] = {}, qq[4] = {}, ss[9] = {};
					const float* ref = rest.Orientation;
					bool first = true;
					auto acc = [&](const T& t, float w)
					{
						if (w <= 0) return;
						float s = 1.0f;
						if (cand == 1) { float d = t.q[0]*ref[0]+t.q[1]*ref[1]+t.q[2]*ref[2]+t.q[3]*ref[3]; s = d < 0 ? -1.0f : 1.0f; }
						if (cand == 2 && !first) { float d = t.q[0]*qq[0]+t.q[1]*qq[1]+t.q[2]*qq[2]+t.q[3]*qq[3]; s = d < 0 ? -1.0f : 1.0f; }
						for (int k = 0; k < 3; ++k) pp[k] += w * t.p[k];
						for (int k = 0; k < 4; ++k) qq[k] += s * w * t.q[k];
						for (int k = 0; k < 9; ++k) ss[k] += w * t.ss[k];
						tw += w; first = false;
					};
					if (hasA) acc(ta, wA);
					if (hasB) acc(tb, wB);
					float fill = cand == 3 ? 0.0f : 0.2f;
					if (tw < fill)
					{
						T trest; memcpy(trest.p, rest.Position, 12); memcpy(trest.q, rest.Orientation, 16); memcpy(trest.ss, rest.ScaleShear, 36);
						acc(trest, fill - tw);
					}
					if (tw > 0)
					{
						for (int k = 0; k < 3; ++k) pp[k] /= tw;
						for (int k = 0; k < 9; ++k) ss[k] /= tw;
						float l = sqrtf(qq[0]*qq[0]+qq[1]*qq[1]+qq[2]*qq[2]+qq[3]*qq[3]);
						for (int k = 0; k < 4; ++k) qq[k] /= l;
					}
					else { memcpy(pp, rest.Position, 12); memcpy(qq, rest.Orientation, 16); memcpy(ss, rest.ScaleShear, 36); }
					for (int k = 0; k < 3; ++k) err[cand][0] = fmaxf(err[cand][0], fabsf(pp[k] - r->Position[k]));
					for (int k = 0; k < 4; ++k) err[cand][1] = fmaxf(err[cand][1], fabsf(qq[k] - r->Orientation[k]));
					for (int k = 0; k < 9; ++k) err[cand][2] = fmaxf(err[cand][2], fabsf(ss[k] - (&r->ScaleShear[0][0])[k]));
				}
			}
			printf("wA=%.3f wB=%.3f:", wA, wB);
			for (int cand = 0; cand < 4; ++cand) printf("  c%d p%.2g q%.2g s%.2g", cand, err[cand][0], err[cand][1], err[cand][2]);
			printf("  flags0=%u\n", GrannyGetLocalPoseTransform(lp, 0)->Flags);
			GrannyFreeControl(ca); GrannyFreeControl(cb);
		}

	// Cynex: kok hareketi kenar durumlari
	auto Root = [&](const char* tag, granny_control* c, float clk, float dt)
	{
		GrannySetModelClock(mi, clk);
		float tr[3], rot[3];
		GrannyGetRootMotionVectors(mi, dt, tr, rot, false);
		printf("%-10s clk %.3f dt %.3f raw %.4f idx %d: tr=(%g %g %g) rot=(%g %g %g)\n", tag, clk, dt, GrannyGetControlRawLocalClock(c), GrannyGetControlLoopIndex(c), tr[0], tr[1], tr[2], rot[0], rot[1], rot[2]);
	};
	{
		granny_control* c = GrannyPlayControlledAnimation(0.0f, A, mi);
		for (float clk = 0.0f; clk < 1.2f; clk += 0.1f) Root("loop1", c, clk, 0.1f);
		Root("loop1big", c, 0.7f, 0.5f);
		Root("loop1dt0", c, 0.5f, 0.0f);
		Root("loop1neg", c, 0.5f, -0.1f);
		GrannySetControlSpeed(c, 2.0f);
		Root("speed2", c, 0.55f, 0.05f);
		GrannySetControlWeight(c, 0.5f);
		Root("w0.5", c, 0.56f, 0.01f);
		GrannySetControlLoopCount(c, 3);
		Root("loop3", c, 1.0f, 0.2f);
		GrannySetControlSpeed(c, -1.0f);
		Root("speed-1", c, 1.1f, 0.1f);
		GrannySetControlSpeed(c, 1.0f);
		GrannySetControlWeight(c, 1.0f);
		GrannySetControlEaseIn(c, true);
		GrannySetControlEaseInCurve(c, 1.0f, 2.0f, 0, 0, 1, 1);
		Root("ease", c, 1.5f, 0.1f);
		Root("ease0", c, 0.9f, 0.1f);
		// Cynex: ikinci kontrol (birikimsiz)
		granny_control* c2 = GrannyPlayControlledAnimation(0.0f, B, mi);
		GrannySetControlLoopCount(c2, 0);
		Root("two", c, 1.5f, 0.1f);
		GrannySetControlWeight(c2, 3.0f);
		Root("two w3", c, 1.5f, 0.1f);
		GrannyFreeControl(c2);
		float m[16] = { 0,1,0,0, -1,0,0,0, 0,0,1,0, 10,20,30,1 }, out[16];
		GrannySetModelClock(mi, 1.6f);
		GrannyUpdateModelMatrix(mi, 0.1f, m, out, false);
		printf("mat: "); for (int i = 0; i < 16; ++i) printf("%g ", out[i]); printf("\n");
		GrannyUpdateModelMatrix(mi, 0.1f, m, out, true);
		printf("inv: "); for (int i = 0; i < 16; ++i) printf("%g ", out[i]); printf("\n");
		GrannyFreeControl(c);
	}
	return 0;
}
