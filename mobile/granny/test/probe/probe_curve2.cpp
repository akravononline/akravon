// Cynex: kara kutu deneme - gercek egride dongu penceresi
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

int main(int argc, char** argv)
{
	granny_animation* an = GrannyGetFileInfo(Load(argv[1]))->Animations[0];
	const char* track = argv[2];
	for (int g = 0; g < an->TrackGroupCount; ++g)
		for (int t = 0; t < an->TrackGroups[g]->TransformTrackCount; ++t)
		{
			granny_transform_track& tr = an->TrackGroups[g]->TransformTracks[t];
			if (strcmp(tr.Name, track)) continue;
			const granny_curve_data_da_k32f_c32f* d = (const granny_curve_data_da_k32f_c32f*) tr.OrientationCurve.CurveData.Object;
			printf("group %d track %d dur %g format %d degree %d knots %d\n", g, t, an->Duration, d->CurveDataHeader.Format, d->CurveDataHeader.Degree, d->KnotCount);
			for (int k = 0; k < d->KnotCount; ++k) printf("  k%d %.6f  (%.4f %.4f %.4f %.4f)\n", k, d->Knots[k], d->Controls[4*k], d->Controls[4*k+1], d->Controls[4*k+2], d->Controls[4*k+3]);
			for (int k = 0; k <= d->KnotCount; k += (k == 1 ? d->KnotCount - 3 : 1))
			{
				float ti[64], pi[256]; float* tip = 0; float* pip = 0;
				GrannyConstructBSplineBuffers(4, &tr.OrientationCurve, &tr.OrientationCurve, &tr.OrientationCurve, an->Duration, an->Duration, an->Duration, k, ti, pi, &tip, &pip, GrannyCurveIdentityOrientation);
				printf("  buffers k=%d ti:", k);
				for (int i = -2; i <= 1; ++i) printf(" %.5f", tip[i]);
				printf(" pi:");
				for (int i = -2; i <= 0; ++i) printf(" (%.4f %.4f %.4f %.4f)", pip[4*i], pip[4*i+1], pip[4*i+2], pip[4*i+3]);
				printf("\n");
				if (k >= d->KnotCount) break;
			}
			for (float tt : { 0.0f, 0.005f, 0.79f, 0.8f })
			{
				float r[4];
				GrannyEvaluateCurveAtT(4, true, true, &tr.OrientationCurve, true, an->Duration, tt, r, GrannyCurveIdentityOrientation);
				printf("  eval loop t=%g (%.5f %.5f %.5f %.5f)\n", tt, r[0], r[1], r[2], r[3]);
			}
		}
	return 0;
}
