// Cynex: kara kutu deneme - granny2.dll bspline davranisi
#include <granny.h>
#include <cstdio>
#include <cmath>
#include <limits>

int main()
{
	const float NaN = std::numeric_limits<float>::quiet_NaN();
	for (int degree = 0; degree <= 3; ++degree)
	{
		printf("degree %d\n", degree);
		// Cynex: hangi ti/pi elemanlari okunuyor
		for (int probe = -6; probe <= 6; ++probe)
		{
			float ti[16], pi[16];
			for (int i = 0; i < 16; ++i) { ti[i] = (float)(i - 8); pi[i] = (float)(i * i) * 0.1f; }
			float tiN[16], piN[16];
			for (int i = 0; i < 16; ++i) { tiN[i] = ti[i]; piN[i] = pi[i]; }
			tiN[8 + probe] = NaN;
			float r0, r1;
			GrannySampleBSpline(degree, 1, false, ti + 8, pi + 8, -0.5f, &r0);
			GrannySampleBSpline(degree, 1, false, tiN + 8, pi + 8, -0.5f, &r1);
			float r2;
			piN[8 + probe] = NaN;
			GrannySampleBSpline(degree, 1, false, ti + 8, piN + 8, -0.5f, &r2);
			printf("  off %+d: ti %s pi %s  (r=%g)\n", probe, std::isnan(r1) ? "READ" : "-", std::isnan(r2) ? "READ" : "-", r0);
		}
	}
	return 0;
}
