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
	for (int i = 3; i < argc; ++i)
	{
		int b = atoi(argv[i]);
		for (int g = 0; g < an->TrackGroupCount; ++g)
			for (int t = 0; t < an->TrackGroups[g]->TransformTrackCount; ++t)
				if (!strcmp(an->TrackGroups[g]->TransformTracks[t].Name, sk->Bones[b].Name))
				{
					const granny_curve_data_header* h = (const granny_curve_data_header*) an->TrackGroups[g]->TransformTracks[t].OrientationCurve.CurveData.Object;
					printf("bone %d %s ori format %d degree %d knots %d\n", b, sk->Bones[b].Name, h ? h->Format : -1, h ? h->Degree : -1, GrannyCurveGetKnotCount(&an->TrackGroups[g]->TransformTracks[t].OrientationCurve));
				}
	}
	return 0;
}
