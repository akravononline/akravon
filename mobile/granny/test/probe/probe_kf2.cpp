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
	granny_file_info* ia = GrannyGetFileInfo(Load(argv[1]));
	for (int a = 0; a < ia->AnimationCount; ++a)
	{
		granny_animation* an = ia->Animations[a];
		printf("anim timestep %g dur %g\n", an->TimeStep, an->Duration);
		for (int g = 0; g < an->TrackGroupCount; ++g)
			for (int t = 0; t < an->TrackGroups[g]->TransformTrackCount; ++t)
			{
				granny_transform_track& tr = an->TrackGroups[g]->TransformTracks[t];
				const granny_curve2* cs[3] = { &tr.PositionCurve, &tr.OrientationCurve, &tr.ScaleShearCurve };
				for (int c = 0; c < 3; ++c)
				{
					const granny_curve_data_header* h = (const granny_curve_data_header*) cs[c]->CurveData.Object;
					if (h && h->Format == 0)
					{
						const granny_curve_data_da_keyframes32f* k = (const granny_curve_data_da_keyframes32f*) h;
						if (k->Dimension <= 0 || k->ControlCount < k->Dimension || !k->Controls)
							printf("  bad keyframe track %s c%d dim %d count %d ptr %p\n", tr.Name, c, k->Dimension, k->ControlCount, (void*) k->Controls);
					}
				}
			}
		for (int g = 0; g < an->TrackGroupCount; ++g)
			for (int v = 0; v < an->TrackGroups[g]->VectorTrackCount; ++v)
			{
				granny_vector_track& tr = an->TrackGroups[g]->VectorTracks[v];
				const granny_curve_data_header* h = (const granny_curve_data_header*) tr.ValueCurve.CurveData.Object;
				if (h && h->Format == 0)
				{
					const granny_curve_data_da_keyframes32f* k = (const granny_curve_data_da_keyframes32f*) h;
					printf("  vector keyframe %s dim %d count %d\n", tr.Name, k->Dimension, k->ControlCount);
				}
			}
	}
	return 0;
}
