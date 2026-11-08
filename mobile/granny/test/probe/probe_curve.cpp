// Cynex: kara kutu deneme - egri tamponlari ve dugum arama
#include <granny.h>
#include <cstdio>
#include <cstring>

static void Dump(int degree, bool loopPrev, bool loopNext)
{
	const int n = 5;
	float knots[n] = { 0.0f, 0.5f, 1.0f, 1.5f, 2.0f };
	float ctrls[n] = { 10, 20, 30, 40, 50 };
	granny_curve2 curve;
	granny_curve_data_da_k32f_c32f data;
	GrannyCurveMakeStaticDaK32fC32f(&curve, &data, n, degree, 1, knots, ctrls);
	printf("== degree %d prev=%d next=%d\n", degree, loopPrev, loopNext);
	for (float t = -0.25f; t <= 2.6f; t += 0.25f)
		printf("  FindKnot(%g)=%d\n", t, GrannyCurveFindKnot(&curve, t));
	for (int k = 0; k <= n; ++k)
	{
		float ti[64], pi[64];
		for (int i = 0; i < 64; ++i) { ti[i] = -999; pi[i] = -999; }
		float* tip = 0; float* pip = 0;
		float ident = 0;
		bool ok = GrannyConstructBSplineBuffers(1, loopPrev ? &curve : 0, &curve, loopNext ? &curve : 0, 2.0f, 2.0f, 2.0f, k, ti, pi, &tip, &pip, &ident);
		int ot = tip ? (int)(tip - ti) : -1, op = pip ? (int)(pip - pi) : -1;
		printf("  k=%d ok=%d tiOff=%d piOff=%d  ti:", k, ok, ot, op);
		if (tip) for (int i = -degree - 1; i <= degree; ++i) printf(" %g", tip[i]);
		printf("  pi:");
		if (pip) for (int i = -degree - 1; i <= 0; ++i) printf(" %g", pip[i]);
		printf("\n");
	}
	for (float t = -0.25f; t <= 2.3f; t += 0.125f)
	{
		float r, ident = 0;
		GrannyEvaluateCurveAtT(1, false, loopPrev, &curve, loopNext, 2.0f, t, &r, &ident);
		printf("  eval(%g)=%g\n", t, r);
	}
}

int main()
{
	for (int d = 0; d <= 3; ++d)
	{
		Dump(d, false, false);
		Dump(d, true, true);
	}
	return 0;
}
