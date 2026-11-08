// Cynex: kara kutu deneme - ease ile iki kontrol karisiminin yerel pozu
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

struct T { float p[3], q[4], ss[9]; bool has; };

static T SampleOne(granny_control* c, granny_animation* an, const char* bone, bool normalize)
{
	T out; out.has = false;
	granny_track_group* g = an->TrackGroups[0];
	for (int i = 0; i < an->TrackGroupCount; ++i)
		if (!_stricmp(an->TrackGroups[i]->Name, "Bip01"))
			g = an->TrackGroups[i];
	for (int t = 0; t < g->TransformTrackCount; ++t)
		if (!strcmp(g->TransformTracks[t].Name, bone))
		{
			granny_transform_track& tr = g->TransformTracks[t];
			float lc = GrannyGetControlClampedLocalClock(c);
			granny_bool32 u, o; GrannyGetControlLoopState(c, &u, &o);
			GrannyEvaluateCurveAtT(3, false, u != 0, &tr.PositionCurve, o != 0, an->Duration, lc, out.p, GrannyCurveIdentityPosition);
			GrannyEvaluateCurveAtT(4, normalize, u != 0, &tr.OrientationCurve, o != 0, an->Duration, lc, out.q, GrannyCurveIdentityOrientation);
			GrannyEvaluateCurveAtT(9, false, u != 0, &tr.ScaleShearCurve, o != 0, an->Duration, lc, out.ss, GrannyCurveIdentityScaleShear);
			out.has = true;
		}
	return out;
}

int main(int argc, char** argv)
{
	// Cynex: probe_blend3 <model> <animA> <animB> <animA_conv> <animB_conv> <clock>
	granny_file_info* im = GrannyGetFileInfo(Load(argv[1]));
	granny_animation* A = GrannyGetFileInfo(Load(argv[2]))->Animations[0];
	granny_animation* B = GrannyGetFileInfo(Load(argv[3]))->Animations[0];
	granny_animation* AC = GrannyGetFileInfo(Load(argv[4]))->Animations[0];
	granny_animation* BC = GrannyGetFileInfo(Load(argv[5]))->Animations[0];
	granny_model* mdl = im->Models[0];
	granny_skeleton* sk = mdl->Skeleton;
	granny_model_instance* mi = GrannyInstantiateModel(mdl);
	granny_local_pose* lp = GrannyNewLocalPose(sk->BoneCount);

	granny_control* ca = GrannyPlayControlledAnimation(0.0f, A, mi);
	GrannySetControlLoopCount(ca, 0);
	(void) argc;
	GrannySetModelClock(mi, 0.8f);
	GrannySetControlEaseOutCurve(ca, 0.8f, 1.0f, 1, 1, 0, 0);
	GrannySetControlEaseIn(ca, false); GrannySetControlEaseOut(ca, true);
	GrannyCompleteControlAt(ca, 1.0f);
	granny_control* cb = GrannyPlayControlledAnimation(0.8f, B, mi);
	GrannySetControlSpeed(cb, 1.3f); GrannySetControlLoopCount(cb, 1);
	GrannySetControlEaseIn(cb, true); GrannySetControlEaseOut(cb, false);
	GrannySetControlEaseInCurve(cb, 0.8f, 1.0f, 0, 0, 1, 1);
	float clk = (float) atof(argv[6]);
	GrannySetModelClock(mi, clk);
	GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
	float wa = GrannyGetControlEffectiveWeight(ca), wb = GrannyGetControlEffectiveWeight(cb);
	printf("clk %g wa %g wb %g\n", clk, wa, wb);
	float err[4][2] = {};
	int worst[4] = { -1, -1, -1, -1 };
	for (int b = 0; b < sk->BoneCount; ++b)
	{
		const granny_transform* r = GrannyGetLocalPoseTransform(lp, b);
		const granny_transform& rest = sk->Bones[b].LocalTransform;
		for (int cand = 0; cand < 4; ++cand)
		{
			bool norm = (cand & 1) != 0;
			bool restRef = (cand & 2) == 0;
			T ta = SampleOne(ca, AC, sk->Bones[b].Name, norm), tb = SampleOne(cb, BC, sk->Bones[b].Name, norm);
			float W = 0, P[3] = {}, Q[4] = {};
			const float* ref = rest.Orientation;
			bool first = true;
			auto acc = [&](const T& t, float w)
			{
				if (!t.has || w <= 0.001f) return;
				float d = restRef ? (t.q[0]*ref[0]+t.q[1]*ref[1]+t.q[2]*ref[2]+t.q[3]*ref[3]) : (first ? 1.0f : t.q[0]*Q[0]+t.q[1]*Q[1]+t.q[2]*Q[2]+t.q[3]*Q[3]);
				float s = d < 0 ? -1.0f : 1.0f;
				for (int k = 0; k < 3; ++k) P[k] += w * t.p[k];
				for (int k = 0; k < 4; ++k) Q[k] += s * w * t.q[k];
				W += w; first = false;
			};
			acc(ta, wa); acc(tb, wb);
			if (W < 0.2f) { T tr; memcpy(tr.p, rest.Position, 12); memcpy(tr.q, rest.Orientation, 16); tr.has = true; acc(tr, 0.2f - W); }
			float l = sqrtf(Q[0]*Q[0]+Q[1]*Q[1]+Q[2]*Q[2]+Q[3]*Q[3]);
			float ep = 0, eq = 0;
			float dq = Q[0]*r->Orientation[0]+Q[1]*r->Orientation[1]+Q[2]*r->Orientation[2]+Q[3]*r->Orientation[3];
			float sg = dq < 0 ? -1.0f : 1.0f;
			for (int k = 0; k < 3; ++k) ep = fmaxf(ep, fabsf(P[k] / W - r->Position[k]));
			for (int k = 0; k < 4; ++k) eq = fmaxf(eq, fabsf(sg * Q[k] / l - r->Orientation[k]));
			if (eq > err[cand][1]) worst[cand] = b;
			err[cand][0] = fmaxf(err[cand][0], ep); err[cand][1] = fmaxf(err[cand][1], eq);
		}
	}
	{
		const char* bn = sk->Bones[0].Name;
		T ta = SampleOne(ca, AC, bn, true), tb = SampleOne(cb, BC, bn, true);
		const granny_transform* r = GrannyGetLocalPoseTransform(lp, 0);
		const float* q0 = sk->Bones[0].LocalTransform.Orientation;
		printf("bone0 qa=(%g %g %g %g) qb=(%g %g %g %g) rest=(%g %g %g %g) oracle=(%g %g %g %g) flags=%u restflags=%u\n", ta.q[0], ta.q[1], ta.q[2], ta.q[3], tb.q[0], tb.q[1], tb.q[2], tb.q[3], q0[0], q0[1], q0[2], q0[3], r->Orientation[0], r->Orientation[1], r->Orientation[2], r->Orientation[3], r->Flags, sk->Bones[0].LocalTransform.Flags);
	}
	{
		const char* bn = sk->Bones[0].Name;
		T ta = SampleOne(ca, AC, bn, true), tb = SampleOne(cb, BC, bn, true);
		granny_local_pose* tmp = GrannyNewLocalPose(sk->BoneCount);
		for (int order = 0; order < 2; ++order)
			for (int mode = 0; mode < 4; ++mode)
			{
				GrannyBeginLocalPoseAccumulation(tmp, 0, 1, NULL);
				granny_transform xa, xb; memset(&xa, 0, sizeof(xa)); memset(&xb, 0, sizeof(xb));
				xa.Flags = xb.Flags = 3; memcpy(xa.Position, ta.p, 12); memcpy(xa.Orientation, ta.q, 16); memcpy(xb.Position, tb.p, 12); memcpy(xb.Orientation, tb.q, 16);
				for (int k = 0; k < 3; ++k) { xa.ScaleShear[k][k] = 1; xb.ScaleShear[k][k] = 1; }
				if (order == 0) { GrannyAccumulateLocalTransform(tmp, 0, 0, wa, sk, (granny_quaternion_mode) mode, &xa); GrannyAccumulateLocalTransform(tmp, 0, 0, wb, sk, (granny_quaternion_mode) mode, &xb); }
				else { GrannyAccumulateLocalTransform(tmp, 0, 0, wb, sk, (granny_quaternion_mode) mode, &xb); GrannyAccumulateLocalTransform(tmp, 0, 0, wa, sk, (granny_quaternion_mode) mode, &xa); }
				GrannyEndLocalPoseAccumulation(tmp, 0, 1, sk, NULL);
				const float* q = GrannyGetLocalPoseTransform(tmp, 0)->Orientation;
				const float* r = GrannyGetLocalPoseTransform(lp, 0)->Orientation;
				float e = 0; for (int k = 0; k < 4; ++k) e = fmaxf(e, fabsf(q[k] - r[k]));
				printf("order %d mode %d q=(%.4f %.4f %.4f %.4f) err %.3g\n", order, mode, q[0], q[1], q[2], q[3], e);
			}
	}
	const char* names[4] = { "raw+restref", "norm+restref", "raw+accumref", "norm+accumref" };
	for (int cand = 0; cand < 4; ++cand) printf("%-14s pos %g ori %g (bone %d)\n", names[cand], err[cand][0], err[cand][1], worst[cand]);
	return 0;
}
