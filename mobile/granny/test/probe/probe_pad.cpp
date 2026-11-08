#include <granny.h>
#include <cstdio>
#include <cstdlib>
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
	int counts[2] = {};
	for (int g = 0; g < an->TrackGroupCount; ++g)
		for (int t = 0; t < an->TrackGroups[g]->TransformTrackCount; ++t)
		{
			const granny_curve_data_header* h = (const granny_curve_data_header*) an->TrackGroups[g]->TransformTracks[t].OrientationCurve.CurveData.Object;
			if (h && h->Format == 1) counts[((const granny_curve_data_da_k32f_c32f*) h)->Padding == 1]++;
			if (h && t < 3) printf("track %d format %d\n", t, h->Format);
		}
	printf("pad0 %d pad1 %d\n", counts[0], counts[1]);
	return 0;
}
