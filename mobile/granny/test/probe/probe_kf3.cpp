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
	for (int g = 0; g < an->TrackGroupCount; ++g)
		for (int t = 0; t < an->TrackGroups[g]->TransformTrackCount; ++t)
		{
			granny_transform_track& tr = an->TrackGroups[g]->TransformTracks[t];
			if (strcmp(tr.Name, argv[2])) continue;
			const granny_curve_data_header* h = (const granny_curve_data_header*) tr.OrientationCurve.CurveData.Object;
			printf("group %d format %d dur %g step %g\n", g, h->Format, an->Duration, an->TimeStep);
			if (h->Format != 0) continue;
			const granny_curve_data_da_keyframes32f* k = (const granny_curve_data_da_keyframes32f*) h;
			int frames = k->ControlCount / 4;
			printf("frames %d dim %d\n", frames, k->Dimension);
			for (int f = 0; f < frames && f < 20; ++f) printf("  key %d (%.4f %.4f %.4f %.4f)\n", f, k->Controls[4*f], k->Controls[4*f+1], k->Controls[4*f+2], k->Controls[4*f+3]);
		}
	return 0;
}
