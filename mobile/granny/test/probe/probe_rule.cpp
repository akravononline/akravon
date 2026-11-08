// Cynex: kara kutu deneme - iki kontrol karisiminda quaternion isaret kurali (tum kemikler)
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

static granny_track_group* Group(granny_animation* an, const char* model)
{
	granny_int32x gi = 0;
	GrannyFindTrackGroupForModel(an, model, &gi);
	return an->TrackGroups[gi];
}

static float Dot4(const float* a, const float* b) { return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3]; }

static bool SampleQ(granny_control* c, granny_animation* an, granny_track_group* g, const char* bone, float* q, bool* isKey, float* p0 = NULL)
{
	for (int t = 0; t < g->TransformTrackCount; ++t)
	{
		if (strcmp(g->TransformTracks[t].Name, bone)) continue;
		granny_transform_track& tr = g->TransformTracks[t];
		float lc = GrannyGetControlClampedLocalClock(c);
		granny_bool32 u, o; GrannyGetControlLoopState(c, &u, &o);
		const granny_curve_data_header* h = (const granny_curve_data_header*) tr.OrientationCurve.CurveData.Object;
		*isKey = h && h->Format == 0;
		if (*isKey)
		{
			const granny_curve_data_da_keyframes32f* k = (const granny_curve_data_da_keyframes32f*) h;
			int frames = k->ControlCount / 4;
			int f = (int) (lc / an->TimeStep);
			if (f >= frames) f = frames - 1;
			if (f < 0) f = 0;
			memcpy(q, k->Controls + 4 * f, 16);
			float l = sqrtf(q[0]*q[0]+q[1]*q[1]+q[2]*q[2]+q[3]*q[3]);
			for (int i = 0; i < 4; ++i) q[i] /= l;
		}
		else
		{
			GrannyEvaluateCurveAtT(4, true, u != 0, &tr.OrientationCurve, o != 0, an->Duration, lc, q, GrannyCurveIdentityOrientation);
			if (p0)
			{
				memcpy(p0, q, 16);
				if (h && h->Format == 1 && ((const granny_curve_data_da_k32f_c32f*) h)->KnotCount > 1)
				{
					int deg = h->Degree;
					int k = GrannyCurveFindKnot(&tr.OrientationCurve, lc);
					float ti[64], pi[256]; float* tip = 0; float* pip = 0;
					GrannyConstructBSplineBuffers(4, u ? &tr.OrientationCurve : 0, &tr.OrientationCurve, o ? &tr.OrientationCurve : 0, an->Duration, an->Duration, an->Duration, k, ti, pi, &tip, &pip, GrannyCurveIdentityOrientation);
					memcpy(p0, pip - 4 * deg, 16 * (deg + 1));
					for (int j = 1; j <= deg; ++j)
						if (Dot4(p0 + 4 * j, p0 + 4 * (j - 1)) < 0)
							for (int kk = 0; kk < 4; ++kk) p0[4 * j + kk] = -p0[4 * j + kk];
					p0[15] = (float) deg + 100.0f;
				}
			}
		}
		return true;
	}
	return false;
}

static float Dot(const float* a, const float* b) { return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3]; }

static void Inv4(const float* m, float* o)
{
	// Cynex: afin tersi (3x3 genel ters + oteleme)
	float a = m[0], b = m[1], c = m[2], d = m[4], e = m[5], f = m[6], g = m[8], h = m[9], i = m[10];
	float det = a*(e*i-f*h) - b*(d*i-f*g) + c*(d*h-e*g);
	float r[9] = { (e*i-f*h)/det, (c*h-b*i)/det, (b*f-c*e)/det, (f*g-d*i)/det, (a*i-c*g)/det, (c*d-a*f)/det, (d*h-e*g)/det, (b*g-a*h)/det, (a*e-b*d)/det };
	for (int k = 0; k < 3; ++k) { o[k*4+0] = r[k*3+0]; o[k*4+1] = r[k*3+1]; o[k*4+2] = r[k*3+2]; o[k*4+3] = 0; }
	for (int j = 0; j < 3; ++j) o[12+j] = -(m[12]*o[0*4+j] + m[13]*o[1*4+j] + m[14]*o[2*4+j]);
	o[15] = 1;
}

static void Mul4(const float* a, const float* b, float* o)
{
	for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) { float s = 0; for (int k = 0; k < 4; ++k) s += a[i*4+k]*b[k*4+j]; o[i*4+j] = s; }
}

static void BindLocalQ(granny_skeleton* sk, int b, float* q)
{
	float wb[16], wp[16], iwp[16], l[16];
	Inv4(&sk->Bones[b].InverseWorld4x4[0][0], wb);
	int p = sk->Bones[b].ParentIndex;
	if (p >= 0) { Inv4(&sk->Bones[p].InverseWorld4x4[0][0], wp); Inv4(wp, iwp); Mul4(wb, iwp, l); } else memcpy(l, wb, 64);
	for (int r = 0; r < 3; ++r) { float n = sqrtf(l[r*4]*l[r*4]+l[r*4+1]*l[r*4+1]+l[r*4+2]*l[r*4+2]); for (int k = 0; k < 3; ++k) l[r*4+k] /= n; }
	// Cynex: satir vektor matrisinden quaternion (BuildCompositeMatrix tersi)
	float tr = l[0] + l[5] + l[10];
	if (tr > 0) { float s = sqrtf(tr + 1) * 2; q[3] = 0.25f * s; q[0] = (l[6] - l[9]) / s; q[1] = (l[8] - l[2]) / s; q[2] = (l[1] - l[4]) / s; }
	else if (l[0] > l[5] && l[0] > l[10]) { float s = sqrtf(1 + l[0] - l[5] - l[10]) * 2; q[3] = (l[6] - l[9]) / s; q[0] = 0.25f * s; q[1] = (l[1] + l[4]) / s; q[2] = (l[8] + l[2]) / s; }
	else if (l[5] > l[10]) { float s = sqrtf(1 + l[5] - l[0] - l[10]) * 2; q[3] = (l[8] - l[2]) / s; q[0] = (l[1] + l[4]) / s; q[1] = 0.25f * s; q[2] = (l[6] + l[9]) / s; }
	else { float s = sqrtf(1 + l[10] - l[0] - l[5]) * 2; q[3] = (l[1] - l[4]) / s; q[0] = (l[8] + l[2]) / s; q[1] = (l[6] + l[9]) / s; q[2] = 0.25f * s; }
}

// Cynex: zincir modeli - orneklenen degerin (ham P0 zinciri) global zincir isaretine gore carpani
// Cynex: hibrit - referans = rest'e hizali C0, tur paritesine gore (dikis isareti) cevrilmis; sonuca gore karar
static float HybridSign(granny_control* c, granny_track_group* g, const char* bone, const float* rest, const float* sample)
{
	for (int t = 0; t < g->TransformTrackCount; ++t)
	{
		if (strcmp(g->TransformTracks[t].Name, bone)) continue;
		const granny_curve_data_header* h = (const granny_curve_data_header*) g->TransformTracks[t].OrientationCurve.CurveData.Object;
		if (!h || h->Format == 0) return 1;
		const float* ctl = NULL; int n = 0;
		if (h->Format == 1) { ctl = ((const granny_curve_data_da_k32f_c32f*) h)->Controls; n = ((const granny_curve_data_da_k32f_c32f*) h)->KnotCount; if (((const granny_curve_data_da_k32f_c32f*) h)->Padding == 1) return 1; }
		else if (h->Format == 5) { ctl = ((const granny_curve_data_d4_constant32f*) h)->Controls; n = 1; }
		if (!ctl || n <= 0) return 1;
		float R[4]; memcpy(R, ctl, 16);
		if (Dot(R, rest) < 0) for (int k = 0; k < 4; ++k) R[k] = -R[k];
		if (n >= 2)
		{
			std::vector<int> s(n);
			s[0] = 1;
			for (int i = 1; i < n; ++i) s[i] = Dot(ctl + 4 * i, ctl + 4 * (i - 1)) < 0 ? -s[i - 1] : s[i - 1];
			int last = n - 2; int ch = Dot(ctl, ctl + 4 * last) < 0 ? -s[last] : s[last];
			int F = ch == s[0] ? 1 : -1;
			int L = GrannyGetControlLoopIndex(c);
			if (F < 0 && (L & 1)) for (int k = 0; k < 4; ++k) R[k] = -R[k];
		}
		return Dot(sample, R) < 0 ? -1.f : 1.f;
	}
	return 1;
}

static float ChainSign(granny_control* c, granny_animation* an, granny_track_group* g, const char* bone, const float* rest)
{
	for (int t = 0; t < g->TransformTrackCount; ++t)
	{
		if (strcmp(g->TransformTracks[t].Name, bone)) continue;
		const granny_curve_data_header* h = (const granny_curve_data_header*) g->TransformTracks[t].OrientationCurve.CurveData.Object;
		if (!h) return 1;
		if (h->Format == 0) return 1;
		const float* ctl = NULL; int n = 0;
		if (h->Format == 1) { ctl = ((const granny_curve_data_da_k32f_c32f*) h)->Controls; n = ((const granny_curve_data_da_k32f_c32f*) h)->KnotCount; }
		else if (h->Format == 5) { ctl = ((const granny_curve_data_d4_constant32f*) h)->Controls; n = 1; }
		if (!ctl || n <= 0) return 1;
		std::vector<int> s(n);
		s[0] = Dot(ctl, rest) < 0 ? -1 : 1;
		for (int i = 1; i < n; ++i) s[i] = Dot(ctl + 4 * i, ctl + 4 * (i - 1)) < 0 ? -s[i - 1] : s[i - 1];
		if (n == 1) return (float) s[0];
		int F = 1;
		{ int last = n - 2; int ch = Dot(ctl, ctl + 4 * last) < 0 ? -s[last] : s[last]; F = ch == s[0] ? 1 : -1; }
		float lc = GrannyGetControlClampedLocalClock(c);
		granny_bool32 u, o; GrannyGetControlLoopState(c, &u, &o);
		int k = GrannyCurveFindKnot(&g->TransformTracks[t].OrientationCurve, lc);
		int i = k - h->Degree, m = n - 1, r, q = 0;
		if (i < 0 && (!u || m <= 0)) r = 0;
		else if (i >= m && (!o || m <= 0)) r = i < n ? i : n - 1;
		else if (i >= 0 && i < m) r = i;
		else { q = i / m; r = i - q * m; if (r < 0) { r += m; --q; } }
		int L = GrannyGetControlLoopIndex(c) + q;
		float ls = (F < 0 && (L & 1)) ? -1.f : 1.f;
		return (float) s[r] * ls;
	}
	return 1;
}

// Cynex: track'in kendi ilk kontrol noktasi
static bool TrackFirst(granny_track_group* g, const char* bone, float* q)
{
	for (int t = 0; t < g->TransformTrackCount; ++t)
	{
		if (strcmp(g->TransformTracks[t].Name, bone)) continue;
		const granny_curve_data_header* h = (const granny_curve_data_header*) g->TransformTracks[t].OrientationCurve.CurveData.Object;
		if (!h) return false;
		const float* c = NULL;
		if (h->Format == 0) c = ((const granny_curve_data_da_keyframes32f*) h)->Controls;
		else if (h->Format == 1) c = ((const granny_curve_data_da_k32f_c32f*) h)->Controls;
		else if (h->Format == 5) c = ((const granny_curve_data_d4_constant32f*) h)->Controls;
		if (!c) return false;
		memcpy(q, c, 16);
		return true;
	}
	return false;
}

int main(int argc, char** argv)
{
	granny_file_info* im = GrannyGetFileInfo(Load(argv[1]));
	granny_animation* A = GrannyGetFileInfo(Load(argv[2]))->Animations[0];
	granny_animation* B = GrannyGetFileInfo(Load(argv[3]))->Animations[0];
	float clk = (float) atof(argv[4]);
	granny_model* mdl = im->Models[0];
	granny_skeleton* sk = mdl->Skeleton;
	granny_track_group* gA = Group(A, mdl->Name);
	granny_track_group* gB = Group(B, mdl->Name);
	// Cynex: aday hesaplari donusturulmus egrilerle (oracle orijinal dosyayla calisir)
	if (getenv("CONV_A") && getenv("CONV_B"))
	{
		gA = Group(GrannyGetFileInfo(Load(getenv("CONV_A")))->Animations[0], mdl->Name);
		gB = Group(GrannyGetFileInfo(Load(getenv("CONV_B")))->Animations[0], mdl->Name);
	}
	granny_model_instance* mi = GrannyInstantiateModel(mdl);
	granny_local_pose* lp = GrannyNewLocalPose(sk->BoneCount);
	auto Env = [](const char* n, float d) { const char* v = getenv(n); return v ? (float) atof(v) : d; };
	const float aStart = Env("A_START", 0.0f), aSpeed = Env("A_SPEED", 1.0f), T = Env("T", 0.8f), blend = Env("BLEND", 0.2f), bSpeed = Env("B_SPEED", 1.3f);
	const int aLoop = (int) Env("A_LOOP", 0.0f), bLoop = (int) Env("B_LOOP", 1.0f);
	granny_control* ca = GrannyPlayControlledAnimation(aStart, A, mi);
	GrannySetControlLoopCount(ca, aLoop);
	GrannySetControlSpeed(ca, aSpeed);
	if (getenv("A_RAWAT"))
	{
		GrannySetModelClock(mi, Env("A_RAWAT", 0.0f));
		GrannySetControlRawLocalClock(ca, GrannyGetControlLocalDuration(ca));
	}
	GrannySetModelClock(mi, T);
	GrannySetControlEaseOutCurve(ca, T, T + blend, 1, 1, 0, 0);
	GrannySetControlEaseIn(ca, false); GrannySetControlEaseOut(ca, true);
	GrannyCompleteControlAt(ca, T + blend);
	granny_control* cb = GrannyPlayControlledAnimation(T, B, mi);
	GrannySetControlSpeed(cb, bSpeed); GrannySetControlLoopCount(cb, bLoop);
	GrannySetControlEaseIn(cb, true); GrannySetControlEaseOut(cb, false);
	GrannySetControlEaseInCurve(cb, T, T + blend, 0, 0, 1, 1);
	GrannySetModelClock(mi, clk);
	GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
	const float wa = GrannyGetControlEffectiveWeight(ca), wb = GrannyGetControlEffectiveWeight(cb);
	const char* names[] = { "restflip", "accumAfirst", "accumBfirst", "firstRest+accum", "direct", "accumRef=rest-if-empty B first", "selfFirst", "selfFirstThenAccum", "bindflip", "p0rest", "chain", "C0rest", "hybrid" };
	const int C = 13;
	int ok[C] = {}, total = 0;
	for (int b = 0; b < sk->BoneCount; ++b)
	{
		float qa[4], qb[4]; bool ka = false, kb = false;
		float pa[16], pb[16];
		pa[15] = pb[15] = 0;
		bool ha = SampleQ(ca, A, gA, sk->Bones[b].Name, qa, &ka, pa), hb = SampleQ(cb, B, gB, sk->Bones[b].Name, qb, &kb, pb);
		if (!ha || !hb) continue;
		const float* rest = sk->Bones[b].LocalTransform.Orientation;
		const float* r = GrannyGetLocalPoseTransform(lp, b)->Orientation;
		if (fabsf(Dot(qa, qb)) > 0.999f) continue;
		++total;
		bool matched[C];
		for (int c = 0; c < C; ++c)
		{
			float acc[4] = {};
			auto add = [&](const float* q, float w, int mode)
			{
				float s = 1;
				bool empty = acc[0] == 0 && acc[1] == 0 && acc[2] == 0 && acc[3] == 0;
				if (mode == 1) s = Dot(q, rest) < 0 ? -1.f : 1.f;
				if (mode == 2 && !empty) s = Dot(q, acc) < 0 ? -1.f : 1.f;
				if (mode == 3) s = (empty ? Dot(q, rest) : Dot(q, acc)) < 0 ? -1.f : 1.f;
				for (int k = 0; k < 4; ++k) acc[k] += s * w * q[k];
			};
			switch (c)
			{
				case 0: add(qa, wa, 1); add(qb, wb, 1); break;
				case 1: add(qa, wa, 2); add(qb, wb, 2); break;
				case 2: add(qb, wb, 2); add(qa, wa, 2); break;
				case 3: add(qa, wa, 3); add(qb, wb, 3); break;
				case 4: add(qa, wa, 0); add(qb, wb, 0); break;
				case 5: add(qb, wb, 3); add(qa, wa, 3); break;
				case 6: { float fa[4], fb[4]; if (TrackFirst(gA, sk->Bones[b].Name, fa) && TrackFirst(gB, sk->Bones[b].Name, fb)) { float sa = Dot(qa, fa) < 0 ? -1.f : 1.f, sb = Dot(qb, fb) < 0 ? -1.f : 1.f; for (int k = 0; k < 4; ++k) acc[k] = sa * wa * qa[k] + sb * wb * qb[k]; } break; }
				case 12: { float sa = HybridSign(ca, gA, sk->Bones[b].Name, rest, qa), sb = HybridSign(cb, gB, sk->Bones[b].Name, rest, qb); for (int k = 0; k < 4; ++k) acc[k] = sa * wa * qa[k] + sb * wb * qb[k]; break; }
				case 11: {
					float fa[4] = {0,0,0,1}, fb[4] = {0,0,0,1};
					TrackFirst(gA, sk->Bones[b].Name, fa); TrackFirst(gB, sk->Bones[b].Name, fb);
					if (Dot(fa, rest) < 0) for (int k = 0; k < 4; ++k) fa[k] = -fa[k];
					if (Dot(fb, rest) < 0) for (int k = 0; k < 4; ++k) fb[k] = -fb[k];
					float sa = (!ka && Dot(qa, fa) < 0) ? -1.f : 1.f, sb = (!kb && Dot(qb, fb) < 0) ? -1.f : 1.f;
					for (int k = 0; k < 4; ++k) acc[k] = sa * wa * qa[k] + sb * wb * qb[k];
					break;
				}
				case 10: { float sa = ChainSign(ca, A, gA, sk->Bones[b].Name, rest), sb = ChainSign(cb, B, gB, sk->Bones[b].Name, rest); for (int k = 0; k < 4; ++k) acc[k] = sa * wa * qa[k] + sb * wb * qb[k]; break; }
				case 9: { float sa = (!ka && Dot(pa, rest) < 0) ? -1.f : 1.f, sb = (!kb && Dot(pb, rest) < 0) ? -1.f : 1.f; for (int k = 0; k < 4; ++k) acc[k] = sa * wa * qa[k] + sb * wb * qb[k]; break; }
				case 8: { float bq[4]; BindLocalQ(sk, b, bq); float sa = Dot(qa, bq) < 0 ? -1.f : 1.f, sb = Dot(qb, bq) < 0 ? -1.f : 1.f; for (int k = 0; k < 4; ++k) acc[k] = sa * wa * qa[k] + sb * wb * qb[k]; break; }
				case 7: { float fa[4]; if (TrackFirst(gA, sk->Bones[b].Name, fa)) { float sa = Dot(qa, fa) < 0 ? -1.f : 1.f; for (int k = 0; k < 4; ++k) acc[k] = sa * wa * qa[k]; add(qb, wb, 2); } break; }
			}
			float l = sqrtf(Dot(acc, acc));
			float d = 0;
			for (int k = 0; k < 4; ++k) d = fmaxf(d, fminf(fabsf(acc[k] / l - r[k]), fabsf(acc[k] / l + r[k])));
			matched[c] = d < 1e-4f;
			ok[c] += matched[c];
		}
		if (argc > 5)
		{
			printf("bone %2d ka%d kb%d da.rest %+.3f db.rest %+.3f dab %+.3f r.rest %+.3f :", b, ka, kb, Dot(qa, rest), Dot(qb, rest), Dot(qa, qb), Dot(r, rest));
			for (int c = 0; c < C; ++c) printf(" %d", matched[c]);
			printf(" | win a:");
			if (pa[15] > 99) for (int j = 0; j <= (int) (pa[15] - 100); ++j) printf(" %+.3f", Dot(pa + 4 * j, rest));
			printf(" win b:");
			if (pb[15] > 99) for (int j = 0; j <= (int) (pb[15] - 100); ++j) printf(" %+.3f", Dot(pb + 4 * j, rest));
			{ float fa[4] = {0,0,0,1}, fb[4] = {0,0,0,1}; TrackFirst(gA, sk->Bones[b].Name, fa); TrackFirst(gB, sk->Bones[b].Name, fb);
			  printf(" | qa.C0a %+.3f qb.C0b %+.3f C0a.rest %+.3f C0b.rest %+.3f C0a(%.3f %.3f %.3f %.3f)", Dot(qa, fa), Dot(qb, fb), Dot(fa, rest), Dot(fb, rest), fa[0], fa[1], fa[2], fa[3]); }
			printf(" | combos:");
			for (int combo = 0; combo < 4; ++combo)
			{
				float sa = (combo & 1) ? -1.f : 1.f, sb = (combo & 2) ? -1.f : 1.f, acc[4];
				for (int k = 0; k < 4; ++k) acc[k] = sa * wa * qa[k] + sb * wb * qb[k];
				float l = sqrtf(Dot(acc, acc)), d = 0;
				for (int k = 0; k < 4; ++k) d = fmaxf(d, fminf(fabsf(acc[k] / l - r[k]), fabsf(acc[k] / l + r[k])));
				printf(" %.2g", d);
			}
			{ float bq[4]; BindLocalQ(sk, b, bq); printf(" rest(%.3f %.3f %.3f %.3f) bind(%.3f %.3f %.3f %.3f)", rest[0], rest[1], rest[2], rest[3], bq[0], bq[1], bq[2], bq[3]); }
			printf(" qa(%.3f %.3f %.3f %.3f) qb(%.3f %.3f %.3f %.3f) r(%.3f %.3f %.3f %.3f)\n", qa[0], qa[1], qa[2], qa[3], qb[0], qb[1], qb[2], qb[3], r[0], r[1], r[2], r[3]);
		}
	}
	printf("clk %g wa %g wb %g bones %d:", clk, wa, wb, total);
	for (int c = 0; c < C; ++c) printf(" %s=%d", names[c], ok[c]);
	printf("\n");
	return 0;
}
