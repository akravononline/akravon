// Cynex: kara kutu deneme - track group / track eslesme kurallari
#include <granny.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
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

static void LogCb(granny_log_message_type Type, granny_log_message_origin Origin, char const* File, granny_int32x Line, char const* Message, void* UserData)
{
	printf("  [log type=%d origin=%d] %s\n", Type, Origin, Message);
}

int main()
{
	setvbuf(stdout, NULL, _IONBF, 0);
	granny_log_callback cb = { LogCb, 0 };
	GrannySetLogCallback(&cb);
	granny_file_info* im = GrannyGetFileInfo(Load("C:/CynexMobilBuild/granny/testdata/warrior/warrior_novice.gr2"));
	granny_file_info* ia = GrannyGetFileInfo(Load("C:/CynexMobilBuild/granny/testdata/warrior/run.gr2"));
	granny_model* mdl = im->Models[0];
	granny_animation* an = ia->Animations[0];
	granny_skeleton* sk = mdl->Skeleton;
	granny_local_pose* lp = GrannyNewLocalPose(sk->BoneCount);
	const char* saved = mdl->Name;

	const char* names[] = { "Bip01", "bip01", "BIP01", "Nope" };
	for (const char* n : names)
	{
		mdl->Name = n;
		granny_int32x idx = -7;
		bool ok = GrannyFindTrackGroupForModel(an, n, &idx);
		granny_model_instance* mi = GrannyInstantiateModel(mdl);
		granny_control* c = GrannyPlayControlledAnimation(0.0f, an, mi);
		printf("model name '%s': find=%d idx=%d control=%p\n", n, ok, idx, (void*) c);
		if (c)
		{
			GrannySetModelClock(mi, 0.2f);
			GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
			printf("   b1 p=(%g %g %g) rest p=(%g %g %g)\n", GrannyGetLocalPoseTransform(lp, 1)->Position[0], GrannyGetLocalPoseTransform(lp, 1)->Position[1], GrannyGetLocalPoseTransform(lp, 1)->Position[2],
				sk->Bones[1].LocalTransform.Position[0], sk->Bones[1].LocalTransform.Position[1], sk->Bones[1].LocalTransform.Position[2]);
			GrannyFreeControl(c);
		}
		GrannyFreeModelInstance(mi);
	}
	mdl->Name = saved;

	// Cynex: track adini kucuk harfe cevir, kemik eslesiyor mu
	{
		granny_transform_track& tr = an->TrackGroups[0]->TransformTracks[2];
		printf("track2 '%s'\n", tr.Name);
		int bone = -1;
		GrannyFindBoneByName(sk, tr.Name, &bone);
		char lower[128];
		strcpy(lower, tr.Name);
		for (char* p = lower; *p; ++p) *p = (char) tolower(*p);
		const char* savedTrack = tr.Name;
		tr.Name = lower;
		granny_model_instance* mi = GrannyInstantiateModel(mdl);
		granny_control* c = GrannyPlayControlledAnimation(0.0f, an, mi);
		GrannySetModelClock(mi, 0.2f);
		GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
		const float* q = GrannyGetLocalPoseTransform(lp, bone)->Orientation;
		const float* r = sk->Bones[bone].LocalTransform.Orientation;
		printf("lowercase track: bone %d q=(%g %g %g %g) rest=(%g %g %g %g)\n", bone, q[0], q[1], q[2], q[3], r[0], r[1], r[2], r[3]);
		GrannyFreeControl(c);
		tr.Name = savedTrack;
		c = GrannyPlayControlledAnimation(0.0f, an, mi);
		GrannySetModelClock(mi, 0.2f);
		GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
		q = GrannyGetLocalPoseTransform(lp, bone)->Orientation;
		printf("normal track:    bone %d q=(%g %g %g %g)\n", bone, q[0], q[1], q[2], q[3]);
		GrannyFreeControl(c);
		GrannyFreeModelInstance(mi);
	}
	// Cynex: model instance serbest birakilirken kontroller
	{
		granny_model_instance* mi = GrannyInstantiateModel(mdl);
		granny_control* c = GrannyPlayControlledAnimation(0.0f, an, mi);
		GrannyFreeControlOnceUnused(c);
		GrannyCompleteControlAt(c, 0.5f);
		GrannySetModelClock(mi, 0.6f);
		printf("before FreeCompleted: complete=%d\n", GrannyControlIsComplete(c));
		GrannyFreeCompletedModelControls(mi);
		granny_model_control_binding* b = GrannyModelControlsBegin(mi);
		printf("after FreeCompleted: first binding=%p end=%p\n", (void*) b, (void*) GrannyModelControlsEnd(mi));
		granny_control* c2 = GrannyPlayControlledAnimation(0.0f, an, mi);
		GrannyFreeModelInstance(mi);
		printf("freed instance with live control\n");
		(void) c2;
	}
	return 0;
}
