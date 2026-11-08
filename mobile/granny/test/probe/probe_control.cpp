// Cynex: kara kutu deneme - control saat/dongu/ease davranisi
#include <granny.h>
#include <cstdio>
#include <cstdlib>
#include <vector>

static granny_file* Load(const char* path)
{
	FILE* fp = fopen(path, "rb");
	if (!fp) { printf("yok: %s\n", path); exit(1); }
	fseek(fp, 0, SEEK_END); long n = ftell(fp); fseek(fp, 0, SEEK_SET);
	std::vector<char> buf(n); fread(buf.data(), 1, n, fp); fclose(fp);
	return GrannyReadEntireFileFromMemory((int)n, buf.data());
}

static void State(const char* tag, granny_model_instance* mi, granny_control* c, float clock)
{
	GrannySetModelClock(mi, clock);
	granny_bool32 u = 0, o = 0;
	GrannyGetControlLoopState(c, &u, &o);
	printf("%-10s clk=%6.3f raw=%8.4f clamp=%8.4f loopIdx=%d u=%d o=%d complete=%d dur=%7.4f left=%7.4f ease=%6.4f w=%6.4f active=%d\n",
		tag, clock, GrannyGetControlRawLocalClock(c), GrannyGetControlClampedLocalClock(c), GrannyGetControlLoopIndex(c), u, o,
		GrannyControlIsComplete(c), GrannyGetControlDuration(c), GrannyGetControlDurationLeft(c),
		GrannyGetControlEaseCurveMultiplier(c), GrannyGetControlEffectiveWeight(c), GrannyControlIsActive(c));
}

int main(int argc, char** argv)
{
	const char* dir = "C:/CynexMobilBuild/granny/testdata/warrior/";
	char p1[512], p2[512];
	sprintf(p1, "%swarrior_novice.gr2", dir);
	sprintf(p2, "%srun.gr2", dir);
	granny_file* fm = Load(p1);
	granny_file* fa = Load(p2);
	granny_file_info* im = GrannyGetFileInfo(fm);
	granny_file_info* ia = GrannyGetFileInfo(fa);
	granny_animation* anim = ia->Animations[0];
	printf("anim %s dur=%g timestep=%g overs=%g loop=%d flags=%d tg=%d\n", anim->Name, anim->Duration, anim->TimeStep, anim->Oversampling, anim->DefaultLoopCount, anim->Flags, anim->TrackGroupCount);
	printf("model %s, trackgroup %s\n", im->Models[0]->Name, anim->TrackGroups[0]->Name);

	granny_model_instance* mi = GrannyInstantiateModel(im->Models[0]);

	{
		printf("-- default (start 1.0)\n");
		granny_control* c = GrannyPlayControlledAnimation(1.0f, anim, mi);
		printf("speed=%g loopcount=%d localdur=%g weight=%g\n", GrannyGetControlSpeed(c), GrannyGetControlLoopCount(c), GrannyGetControlLocalDuration(c), GrannyGetControlWeight(c));
		for (float t = 0.0f; t < 3.0f; t += 0.25f) State("def", mi, c, t);
		GrannyFreeControl(c);
	}
	{
		printf("-- loop 0 speed 1.5\n");
		granny_control* c = GrannyPlayControlledAnimation(0.0f, anim, mi);
		GrannySetControlLoopCount(c, 0);
		GrannySetControlSpeed(c, 1.5f);
		for (float t = 0.0f; t < 3.0f; t += 0.25f) State("l0", mi, c, t);
		GrannyFreeControl(c);
	}
	{
		printf("-- loop 2, set speed mid way\n");
		granny_control* c = GrannyPlayControlledAnimation(0.0f, anim, mi);
		GrannySetControlLoopCount(c, 2);
		for (float t = 0.0f; t < 1.0f; t += 0.25f) State("l2", mi, c, t);
		GrannySetControlSpeed(c, 2.0f);
		for (float t = 1.0f; t < 2.0f; t += 0.125f) State("l2s2", mi, c, t);
		GrannySetControlRawLocalClock(c, 0.1f);
		State("setraw", mi, c, 2.0f);
		State("setraw", mi, c, 2.25f);
		GrannyFreeControl(c);
	}
	{
		printf("-- negative speed\n");
		granny_control* c = GrannyPlayControlledAnimation(0.0f, anim, mi);
		GrannySetControlLoopCount(c, 2);
		GrannySetControlSpeed(c, -1.0f);
		for (float t = 0.0f; t < 2.0f; t += 0.25f) State("neg", mi, c, t);
		GrannyFreeControl(c);
	}
	{
		printf("-- ease in true no curve\n");
		granny_control* c = GrannyPlayControlledAnimation(0.5f, anim, mi);
		GrannySetControlLoopCount(c, 0);
		GrannySetControlEaseIn(c, true);
		for (float t = 0.0f; t < 2.0f; t += 0.125f) State("ein", mi, c, t);
		GrannySetControlEaseOut(c, true);
		for (float t = 0.0f; t < 2.0f; t += 0.125f) State("eio", mi, c, t);
		GrannyFreeControl(c);
	}
	{
		printf("-- ease in curve (game style)\n");
		granny_control* c = GrannyPlayControlledAnimation(1.0f, anim, mi);
		GrannySetControlLoopCount(c, 0);
		GrannySetControlEaseIn(c, true);
		GrannySetControlEaseOut(c, false);
		GrannySetControlEaseInCurve(c, 1.0f, 1.4f, 0.0f, 0.0f, 1.0f, 1.0f);
		for (float t = 0.8f; t < 1.6f; t += 0.05f) State("ecurve", mi, c, t);
		printf("-- ease out curve + complete at\n");
		GrannySetControlEaseOutCurve(c, 2.0f, 2.3f, 1.0f, 1.0f, 0.0f, 0.0f);
		GrannySetControlEaseIn(c, false);
		GrannySetControlEaseOut(c, true);
		GrannyCompleteControlAt(c, 2.3f);
		for (float t = 1.9f; t < 2.5f; t += 0.05f) State("eout", mi, c, t);
		printf("-- odd curve params\n");
		GrannySetControlEaseOut(c, false);
		GrannySetControlEaseIn(c, true);
		GrannySetControlEaseInCurve(c, 3.0f, 4.0f, 0.2f, 0.9f, -0.5f, 0.7f);
		for (float t = 2.8f; t < 4.2f; t += 0.1f) State("odd", mi, c, t);
		GrannySetControlEaseOut(c, true);
		GrannySetControlEaseOutCurve(c, 3.5f, 3.9f, 0.3f, 1.2f, 0.1f, 0.5f);
		for (float t = 2.8f; t < 4.2f; t += 0.1f) State("oddio", mi, c, t);
		GrannyFreeControl(c);
	}
	{
		printf("-- EaseControlIn/Out returns\n");
		granny_control* c = GrannyPlayControlledAnimation(0.0f, anim, mi);
		GrannySetModelClock(mi, 0.3f);
		float r1 = GrannyEaseControlIn(c, 0.5f, false);
		printf("EaseControlIn ret=%g\n", r1);
		for (float t = 0.3f; t < 1.0f; t += 0.1f) State("ecin", mi, c, t);
		float r2 = GrannyEaseControlOut(c, 0.4f);
		printf("EaseControlOut ret=%g\n", r2);
		for (float t = 1.0f; t < 1.6f; t += 0.1f) State("ecout", mi, c, t);
		GrannyFreeControl(c);
	}
	{
		printf("-- complete at, free if complete\n");
		granny_control* c = GrannyPlayControlledAnimation(0.0f, anim, mi);
		GrannySetControlLoopCount(c, 0);
		GrannyCompleteControlAt(c, 0.5f);
		State("cat", mi, c, 0.4f);
		State("cat", mi, c, 0.5f);
		State("cat", mi, c, 0.6f);
		State("back", mi, c, 0.3f);
		GrannyFreeControl(c);
	}
	{
		printf("-- set raw local clock / local duration\n");
		granny_control* c = GrannyPlayControlledAnimation(0.0f, anim, mi);
		GrannySetModelClock(mi, 0.2f);
		GrannySetControlRawLocalClock(c, GrannyGetControlLocalDuration(c));
		State("atend", mi, c, 0.2f);
		State("atend", mi, c, 0.3f);
		GrannySetControlSpeed(c, 2.0f);
		GrannySetControlRawLocalClock(c, 0.1f);
		printf("raw immediately=%g\n", GrannyGetControlRawLocalClock(c));
		State("s2", mi, c, 0.4f);
		GrannyFreeControl(c);
	}
	return 0;
}
