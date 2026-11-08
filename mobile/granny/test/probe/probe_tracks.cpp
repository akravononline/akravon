// Cynex: kara kutu deneme - model iskeleti ile animasyon track'lerinin eslesmesi
#include <granny.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
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
	granny_file_info* im = GrannyGetFileInfo(Load(argv[1]));
	granny_file_info* ia = GrannyGetFileInfo(Load(argv[2]));
	granny_skeleton* sk = im->Models[0]->Skeleton;
	granny_animation* an = ia->Animations[0];
	granny_int32x gi = 0;
	GrannyFindTrackGroupForModel(an, im->Models[0]->Name, &gi);
	granny_track_group* g = an->TrackGroups[gi];
	printf("model '%s' bones %d, group '%s' tracks %d flags 0x%X\n", im->Models[0]->Name, sk->BoneCount, g->Name, g->TransformTrackCount, g->Flags);
	int unbound = 0;
	for (int b = 0; b < sk->BoneCount; ++b)
	{
		int found = -1, foundCase = -1;
		for (int t = 0; t < g->TransformTrackCount; ++t)
		{
			if (!strcmp(g->TransformTracks[t].Name, sk->Bones[b].Name)) found = t;
			if (!_stricmp(g->TransformTracks[t].Name, sk->Bones[b].Name)) foundCase = t;
		}
		if (found < 0 || b < 3 || (argc > 3))
			printf("  bone %d '%s' parent %d -> track %d (nocase %d)\n", b, sk->Bones[b].Name, sk->Bones[b].ParentIndex, found, foundCase);
		if (found < 0) ++unbound;
	}
	printf("unbound bones %d\n", unbound);
	for (int t = 0; t < g->TransformTrackCount; ++t)
	{
		int b = -1;
		GrannyFindBoneByName(sk, g->TransformTracks[t].Name, &b);
		if (b >= sk->BoneCount || argc > 3)
			printf("  track %d '%s' -> bone %d\n", t, g->TransformTracks[t].Name, b < sk->BoneCount ? b : -1);
	}
	// Cynex: ayni isimde iki kemik/track var mi
	for (int b = 0; b < sk->BoneCount; ++b)
		for (int c = b + 1; c < sk->BoneCount; ++c)
			if (!strcmp(sk->Bones[b].Name, sk->Bones[c].Name)) printf("  DUP bone '%s' %d %d\n", sk->Bones[b].Name, b, c);
	for (int t = 0; t < g->TransformTrackCount; ++t)
		for (int u = t + 1; u < g->TransformTrackCount; ++u)
			if (!strcmp(g->TransformTracks[t].Name, g->TransformTracks[u].Name)) printf("  DUP track '%s' %d %d\n", g->TransformTracks[t].Name, t, u);
	return 0;
}
