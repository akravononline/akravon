// Cynex: kara kutu deneme - track group secimi ve periyodik kok hareketi
#include <granny.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>

static granny_file* Load(const char* path)
{
	FILE* fp = fopen(path, "rb");
	if (!fp) { printf("yok %s\n", path); exit(1); }
	fseek(fp, 0, SEEK_END); long n = ftell(fp); fseek(fp, 0, SEEK_SET);
	std::vector<char> buf(n); fread(buf.data(), 1, n, fp); fclose(fp);
	return GrannyReadEntireFileFromMemory((int)n, buf.data());
}

int main(int argc, char** argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	granny_file_info* im = GrannyGetFileInfo(Load(argv[1]));
	granny_file_info* ia = GrannyGetFileInfo(Load(argv[2]));
	granny_animation* an = ia->Animations[0];
	if (!im->ModelCount)
	{
		for (int g = 0; g < an->TrackGroupCount; ++g)
			if (an->TrackGroups[g]->PeriodicLoop)
				printf("R=%g dA=%g dZ=%g dur=%g RdA=%g\n", an->TrackGroups[g]->PeriodicLoop->Radius, an->TrackGroups[g]->PeriodicLoop->dAngle, an->TrackGroups[g]->PeriodicLoop->dZ, an->Duration, an->TrackGroups[g]->PeriodicLoop->Radius * an->TrackGroups[g]->PeriodicLoop->dAngle);
		return 0;
	}
	granny_model* mdl = im->Models[0];
	printf("model '%s' skel '%s' bones %d ; anim dur %g groups %d\n", mdl->Name, mdl->Skeleton->Name, mdl->Skeleton->BoneCount, an->Duration, an->TrackGroupCount);
	for (int g = 0; g < an->TrackGroupCount; ++g)
	{
		granny_track_group* tg = an->TrackGroups[g];
		printf(" group %d '%s' flags 0x%X tracks %d looptrans (%g %g %g)", g, tg->Name, tg->Flags, tg->TransformTrackCount, tg->LoopTranslation[0], tg->LoopTranslation[1], tg->LoopTranslation[2]);
		if (tg->PeriodicLoop)
		{
			granny_periodic_loop* p = tg->PeriodicLoop;
			printf(" periodic R=%g dA=%g dZ=%g X=(%g %g %g) Y=(%g %g %g) Axis=(%g %g %g)", p->Radius, p->dAngle, p->dZ,
				p->BasisX[0], p->BasisX[1], p->BasisX[2], p->BasisY[0], p->BasisY[1], p->BasisY[2], p->Axis[0], p->Axis[1], p->Axis[2]);
		}
		printf(" first track '%s'\n", tg->TransformTrackCount ? tg->TransformTracks[0].Name : "-");
	}
	granny_int32x found = -1;
	bool ok = GrannyFindTrackGroupForModel(an, mdl->Name, &found);
	printf("FindTrackGroupForModel -> %d idx %d\n", ok, found);
	granny_model_instance* mi = GrannyInstantiateModel(mdl);
	granny_control* c = GrannyPlayControlledAnimation(0.0f, an, mi);
	printf("control=%p\n", (void*) c);
	if (!c)
		return 0;
	GrannySetControlLoopCount(c, 0);
	for (float clk = 0.0f; clk <= 2.0f * an->Duration + 0.01f; clk += an->Duration / 8.0f)
	{
		GrannySetModelClock(mi, clk);
		float tr[3], rot[3];
		GrannyGetRootMotionVectors(mi, an->Duration / 8.0f, tr, rot, false);
		printf("clk %.4f raw %.4f: tr=(%.5f %.5f %.5f) rot=(%.6f %.6f %.6f)\n", clk, GrannyGetControlRawLocalClock(c), tr[0], tr[1], tr[2], rot[0], rot[1], rot[2]);
	}
	GrannySetModelClock(mi, an->Duration * 0.3f);
	for (float dt : { 0.01f, 0.1f, an->Duration, 2.0f * an->Duration })
	{
		float tr[3], rot[3];
		GrannyGetRootMotionVectors(mi, dt, tr, rot, false);
		float m[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 }, out[16];
		GrannyUpdateModelMatrix(mi, dt, m, out, false);
		printf("dt %.4f: tr=(%.5f %.5f %.5f) rot=(%.6f %.6f %.6f)\n  mat:", dt, tr[0], tr[1], tr[2], rot[0], rot[1], rot[2]);
		for (int i = 0; i < 16; ++i) printf(" %.5f", out[i]);
		printf("\n");
		float m2[16] = { 0,1,0,0, -1,0,0,0, 0,0,1,0, 10,20,30,1 };
		GrannyUpdateModelMatrix(mi, dt, m2, out, false);
		printf("  mat2:"); for (int i = 0; i < 16; ++i) printf(" %.5f", out[i]); printf("\n");
		GrannyUpdateModelMatrix(mi, dt, m2, out, true);
		printf("  inv2:"); for (int i = 0; i < 16; ++i) printf(" %.5f", out[i]); printf("\n");
	}
	return 0;
}
