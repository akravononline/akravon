// Cynex: kara kutu deneme - track'i olmayan kemik karisima giriyor mu
#include <granny.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
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
	granny_animation* A = GrannyGetFileInfo(Load("C:/CynexMobilBuild/granny/testdata/warrior/run.gr2"))->Animations[0];
	granny_animation* B = GrannyGetFileInfo(Load("C:/CynexMobilBuild/granny/testdata/warrior/attack.gr2"))->Animations[0];
	granny_model* mdl = im->Models[0];
	granny_skeleton* sk = mdl->Skeleton;
	granny_local_pose* lp = GrannyNewLocalPose(sk->BoneCount);
	granny_model_instance* mi = GrannyInstantiateModel(mdl);
	int bone = -1;
	GrannyFindBoneByName(sk, "Bip01 Head", &bone);
	granny_transform_track* tb = 0;
	for (int t = 0; t < B->TrackGroups[0]->TransformTrackCount; ++t) if (!strcmp(B->TrackGroups[0]->TransformTracks[t].Name, "Bip01 Head")) tb = &B->TrackGroups[0]->TransformTracks[t];
	const char* saved = tb->Name;
	for (int unbind = 0; unbind < 2; ++unbind)
	{
		tb->Name = unbind ? "xx" : saved;
		for (float wB : { 0.5f, 0.15f })
		{
			granny_control* ca = GrannyPlayControlledAnimation(0.0f, A, mi);
			GrannySetControlWeight(ca, 0.1f);
			granny_control* cb = GrannyPlayControlledAnimation(0.0f, B, mi);
			GrannySetControlWeight(cb, wB);
			GrannySetModelClock(mi, 0.3f);
			GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
			const float* q = GrannyGetLocalPoseTransform(lp, bone)->Orientation;
			printf("unbind=%d wA=0.1 wB=%g head q=(%.6f %.6f %.6f %.6f)\n", unbind, wB, q[0], q[1], q[2], q[3]);
			GrannyFreeControl(cb);
			GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
			q = GrannyGetLocalPoseTransform(lp, bone)->Orientation;
			printf("   A only w0.1: q=(%.6f %.6f %.6f %.6f)\n", q[0], q[1], q[2], q[3]);
			GrannySetControlWeight(ca, 1.0f);
			GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
			q = GrannyGetLocalPoseTransform(lp, bone)->Orientation;
			printf("   A only w1: q=(%.6f %.6f %.6f %.6f)\n", q[0], q[1], q[2], q[3]);
			GrannyFreeControl(ca);
		}
	}
	const float* r = sk->Bones[bone].LocalTransform.Orientation;
	printf("rest q=(%.6f %.6f %.6f %.6f)\n", r[0], r[1], r[2], r[3]);
	return 0;
}
