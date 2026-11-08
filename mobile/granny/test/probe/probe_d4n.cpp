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
	granny_skeleton* sk = GrannyGetFileInfo(Load(argv[1]))->Models[0]->Skeleton;
	granny_animation* an = GrannyGetFileInfo(Load(argv[2]))->Animations[0];
	int b = atoi(argv[3]);
	for (int g = 0; g < an->TrackGroupCount; ++g)
		for (int t = 0; t < an->TrackGroups[g]->TransformTrackCount; ++t)
			if (!strcmp(an->TrackGroups[g]->TransformTracks[t].Name, sk->Bones[b].Name))
			{
				granny_curve2* c = &an->TrackGroups[g]->TransformTracks[t].OrientationCurve;
				int n = GrannyCurveGetKnotCount(c);
				std::vector<float> knots(n), ctrls(4 * n);
				GrannyCurveExtractKnotValues(c, 0, n, knots.data(), ctrls.data(), GrannyCurveIdentityOrientation);
				granny_curve2* conv = GrannyCurveConvertToDaK32fC32f(c, GrannyCurveIdentityOrientation);
				granny_curve_data_da_k32f_c32f* d = GrannyCurveGetContentsOfDaK32fC32f(conv);
				int diffSign = 0, diffVal = 0;
				for (int i = 0; i < n; ++i)
				{
					float dot = 0, dd = 0;
					for (int k = 0; k < 4; ++k) { dot += ctrls[4*i+k] * d->Controls[4*i+k]; float e = ctrls[4*i+k] - d->Controls[4*i+k]; dd += e*e; }
					if (dot < 0) ++diffSign;
					if (dd > 1e-10 && dot > 0) ++diffVal;
					if ((i < 3) || (argc > 4 && i >= atoi(argv[4]) && i < atoi(argv[4]) + 8)) printf("  %d knot %g/%g extract(%.4f %.4f %.4f %.4f) conv(%.4f %.4f %.4f %.4f)\n", i, knots[i], d->Knots[i], ctrls[4*i], ctrls[4*i+1], ctrls[4*i+2], ctrls[4*i+3], d->Controls[4*i], d->Controls[4*i+1], d->Controls[4*i+2], d->Controls[4*i+3]);
				}
				printf("n %d diffSign %d diffVal %d\n", n, diffSign, diffVal);
			}
	return 0;
}
