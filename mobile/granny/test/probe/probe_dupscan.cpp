// Cynex: kara kutu deneme - ayni isimli track'lerden hangisi baglaniyor (arama kurali)
#include <windows.h>
#include <granny.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>
namespace fs = std::filesystem;

static granny_file* LoadW(const fs::path& p)
{
	FILE* fp = _wfopen(p.c_str(), L"rb");
	if (!fp) return NULL;
	fseek(fp, 0, SEEK_END); long n = ftell(fp); fseek(fp, 0, SEEK_SET);
	std::vector<char> buf(n); fread(buf.data(), 1, n, fp); fclose(fp);
	return GrannyReadEntireFileFromMemory((int)n, buf.data());
}

static int Bis2(granny_track_group* g, const char* key)
{
	int lo = 0, hi = g->TransformTrackCount;
	while (lo < hi)
	{
		int mid = (lo + hi) / 2;
		int r = strcmp(key, g->TransformTracks[mid].Name);
		if (r < 0) hi = mid; else if (r > 0) lo = mid + 1; else return mid;
	}
	return -1;
}

static int Bis1(granny_track_group* g, const char* key)
{
	int lo = 0, hi = g->TransformTrackCount - 1;
	while (lo <= hi)
	{
		int mid = (lo + hi) / 2;
		int r = strcmp(key, g->TransformTracks[mid].Name);
		if (r < 0) hi = mid - 1; else if (r > 0) lo = mid + 1; else return mid;
	}
	return -1;
}

int wmain(int argc, wchar_t** argv)
{
	int stats[5] = {};
	for (auto& e : fs::recursive_directory_iterator(argv[1]))
	{
		if (!e.is_regular_file() || _wcsicmp(e.path().extension().c_str(), L".gr2")) continue;
		std::wstring stem = e.path().stem().wstring();
		if (_wcsicmp(stem.c_str(), e.path().parent_path().filename().c_str())) continue;
		granny_file* fm = LoadW(e.path());
		if (!fm) continue;
		granny_file_info* im = GrannyGetFileInfo(fm);
		if (!im || !im->ModelCount) { GrannyFreeFile(fm); continue; }
		for (auto& a : fs::directory_iterator(e.path().parent_path()))
		{
			if (a.path() == e.path() || _wcsicmp(a.path().extension().c_str(), L".gr2")) continue;
			granny_file* fa = LoadW(a.path());
			if (!fa) continue;
			granny_file_info* ia = GrannyGetFileInfo(fa);
			if (!ia || !ia->AnimationCount) { GrannyFreeFile(fa); continue; }
			granny_model* mdl = im->Models[0];
			granny_skeleton* sk = mdl->Skeleton;
			granny_animation* an = ia->Animations[0];
			granny_int32x gi = -1;
			if (!GrannyFindTrackGroupForModel(an, mdl->Name, &gi)) { GrannyFreeFile(fa); continue; }
			granny_track_group* g = an->TrackGroups[gi];
			granny_model_instance* mi = GrannyInstantiateModel(mdl);
			granny_control* c = GrannyPlayControlledAnimation(0.0f, an, mi);
			GrannySetModelClock(mi, an->Duration * 0.37f);
			granny_local_pose* lp = GrannyNewLocalPose(sk->BoneCount);
			GrannySampleModelAnimations(mi, 0, sk->BoneCount, lp);
			for (int b = 0; b < sk->BoneCount; ++b)
			{
				std::vector<int> cand;
				for (int t = 0; t < g->TransformTrackCount; ++t)
					if (!strcmp(g->TransformTracks[t].Name, sk->Bones[b].Name)) cand.push_back(t);
				if (cand.size() < 2) continue;
				const granny_transform* r = GrannyGetLocalPoseTransform(lp, b);
				int chosen = -1, matches = 0;
				for (int t : cand)
				{
					granny_transform_track& tr = g->TransformTracks[t];
					float p[3], q[4];
					float lc = GrannyGetControlClampedLocalClock(c);
					GrannyEvaluateCurveAtT(3, false, false, &tr.PositionCurve, false, an->Duration, lc, p, GrannyCurveIdentityPosition);
					GrannyEvaluateCurveAtT(4, true, false, &tr.OrientationCurve, false, an->Duration, lc, q, GrannyCurveIdentityOrientation);
					float d = 0;
					for (int k = 0; k < 3; ++k) d = fmaxf(d, fabsf(p[k] - r->Position[k]));
					float dot = q[0]*r->Orientation[0]+q[1]*r->Orientation[1]+q[2]*r->Orientation[2]+q[3]*r->Orientation[3];
					if (d < 1e-3f && fabsf(fabsf(dot) - 1) < 1e-4f) { chosen = t; ++matches; }
				}
				if (matches != 1) { ++stats[4]; continue; }
				int pBis2 = Bis2(g, sk->Bones[b].Name), pBis1 = Bis1(g, sk->Bones[b].Name);
				bool sorted = (g->Flags & 2) != 0;
				stats[0] += chosen == pBis2; stats[1] += chosen == pBis1; stats[2] += chosen == cand.front(); stats[3] += chosen == cand.back();
				printf("%s sorted=%d n=%d cand=[%d..%d] chosen=%d bis2=%d bis1=%d : %ls\n", sk->Bones[b].Name, sorted, g->TransformTrackCount, cand.front(), cand.back(), chosen, pBis2, pBis1, a.path().filename().c_str());
			}
			GrannyFreeLocalPose(lp);
			GrannyFreeModelInstance(mi);
			GrannyFreeFile(fa);
			break;
		}
		GrannyFreeFile(fm);
	}
	printf("bis2 %d bis1 %d first %d last %d ambiguous %d\n", stats[0], stats[1], stats[2], stats[3], stats[4]);
	return 0;
}
