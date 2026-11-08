// Cynex: kara kutu deneme - ease egrisi
#include <granny.h>
#include <cstdio>
#include <cstdlib>
#include <vector>

static granny_file* Load(const char* path)
{
	FILE* fp = fopen(path, "rb");
	fseek(fp, 0, SEEK_END); long n = ftell(fp); fseek(fp, 0, SEEK_SET);
	std::vector<char> buf(n); fread(buf.data(), 1, n, fp); fclose(fp);
	return GrannyReadEntireFileFromMemory((int)n, buf.data());
}

int main()
{
	granny_file_info* im = GrannyGetFileInfo(Load("C:/CynexMobilBuild/granny/testdata/warrior/warrior_novice.gr2"));
	granny_file_info* ia = GrannyGetFileInfo(Load("C:/CynexMobilBuild/granny/testdata/warrior/run.gr2"));
	granny_animation* anim = ia->Animations[0];
	granny_model_instance* mi = GrannyInstantiateModel(im->Models[0]);
	printf("localease default: ");
	{
		granny_control* c = GrannyPlayControlledAnimation(0.0f, anim, mi);
		printf("%d\n", GrannyGetControlLocalEase(c));
		GrannyFreeControl(c);
	}
	float basis[4][4] = { {1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1} };
	for (int io = 0; io < 2; ++io)
	{
		for (int b = 0; b < 4; ++b)
		{
			granny_control* c = GrannyPlayControlledAnimation(0.0f, anim, mi);
			GrannySetControlLoopCount(c, 0);
			if (io == 0) { GrannySetControlEaseIn(c, true); GrannySetControlEaseInCurve(c, 10.0f, 12.0f, basis[b][0], basis[b][1], basis[b][2], basis[b][3]); }
			else { GrannySetControlEaseOut(c, true); GrannySetControlEaseOutCurve(c, 10.0f, 12.0f, basis[b][0], basis[b][1], basis[b][2], basis[b][3]); }
			printf("%s basis %d:", io ? "out" : "in ", b);
			for (float t = 9.5f; t <= 12.6f; t += 0.25f) { GrannySetModelClock(mi, t); printf(" %.4f", GrannyGetControlEaseCurveMultiplier(c)); }
			printf("\n");
			GrannyFreeControl(c);
		}
	}
	// Cynex: varsayilan egriler
	const char* names[4] = { "none", "in", "out", "both" };
	float starts[3] = { 0.0f, 5.0f, 100.0f };
	for (int s = 0; s < 3; ++s)
		for (int m = 1; m < 4; ++m)
		{
			granny_control* c = GrannyPlayControlledAnimation(starts[s], anim, mi);
			GrannySetControlLoopCount(c, 0);
			GrannySetControlEaseIn(c, (m & 1) != 0);
			GrannySetControlEaseOut(c, (m & 2) != 0);
			printf("default %s start=%g:", names[m], starts[s]);
			float ts[] = { -10, -1, -0.01f, 0, 0.01f, 0.1f, 0.5f, 1, 4.9f, 5, 5.1f, 99, 100, 100.5f, 101, 1000 };
			for (float t : ts) { GrannySetModelClock(mi, t); printf(" %g:%.4f", t, GrannyGetControlEaseCurveMultiplier(c)); }
			printf("\n");
			GrannyFreeControl(c);
		}
	// Cynex: oyun sirasi - once ease in egrisi, sonra ease out egrisi
	{
		granny_control* c = GrannyPlayControlledAnimation(1.0f, anim, mi);
		GrannySetControlLoopCount(c, 0);
		GrannySetControlEaseIn(c, true); GrannySetControlEaseOut(c, false);
		GrannySetControlEaseInCurve(c, 1.0f, 1.2f, 0, 0, 1, 1);
		printf("game seq:");
		for (float t = 0.9f; t <= 1.3f; t += 0.05f) { GrannySetModelClock(mi, t); printf(" %.4f", GrannyGetControlEaseCurveMultiplier(c)); }
		GrannySetControlEaseOutCurve(c, 2.0f, 2.2f, 1, 1, 0, 0);
		GrannySetControlEaseIn(c, false); GrannySetControlEaseOut(c, true);
		printf(" | ");
		for (float t = 0.9f; t <= 2.4f; t += 0.1f) { GrannySetModelClock(mi, t); printf(" %.4f", GrannyGetControlEaseCurveMultiplier(c)); }
		printf("\n");
		// Cynex: ikisi acik, egriler cakisiyor
		GrannySetControlEaseIn(c, true);
		GrannySetControlEaseInCurve(c, 1.9f, 2.1f, 0, 0, 1, 1);
		printf("both overlap:");
		for (float t = 1.8f; t <= 2.4f; t += 0.05f) { GrannySetModelClock(mi, t); printf(" %.4f", GrannyGetControlEaseCurveMultiplier(c)); }
		printf("\n");
		GrannyFreeControl(c);
	}
	// Cynex: zero-length ve ters egri
	{
		granny_control* c = GrannyPlayControlledAnimation(0.0f, anim, mi);
		GrannySetControlEaseIn(c, true);
		GrannySetControlEaseInCurve(c, 1.0f, 1.0f, 0, 0, 1, 1);
		printf("zero len in:");
		for (float t = 0.9f; t <= 1.1f; t += 0.05f) { GrannySetModelClock(mi, t); printf(" %.4f", GrannyGetControlEaseCurveMultiplier(c)); }
		GrannySetControlEaseInCurve(c, 1.0f, 0.5f, 0, 0, 1, 1);
		printf(" | reversed:");
		for (float t = 0.4f; t <= 1.1f; t += 0.1f) { GrannySetModelClock(mi, t); printf(" %.4f", GrannyGetControlEaseCurveMultiplier(c)); }
		printf("\n");
		GrannySetControlWeight(c, 0.5f);
		GrannySetModelClock(mi, 2.0f);
		printf("weight 0.5 effective=%g\n", GrannyGetControlEffectiveWeight(c));
		GrannyFreeControl(c);
	}
	return 0;
}
