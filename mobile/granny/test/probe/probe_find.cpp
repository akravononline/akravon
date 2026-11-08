// Cynex: kara kutu deneme - nadir ozellikleri olan dosyalari listele
#include <windows.h>
#include <granny.h>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

int wmain(int argc, wchar_t** argv)
{
	if (argc < 2)
		return 1;
	std::vector<char> buf;
	for (auto& e : fs::recursive_directory_iterator(argv[1]))
	{
		if (!e.is_regular_file() || _wcsicmp(e.path().extension().c_str(), L".gr2"))
			continue;
		FILE* fp = _wfopen(e.path().c_str(), L"rb");
		if (!fp) continue;
		fseek(fp, 0, SEEK_END); long n = ftell(fp); fseek(fp, 0, SEEK_SET);
		buf.resize(n); fread(buf.data(), 1, n, fp); fclose(fp);
		granny_file* f = GrannyReadEntireFileFromMemory((int) n, buf.data());
		if (!f) continue;
		granny_file_info* fi = GrannyGetFileInfo(f);
		std::string name = e.path().u8string();
		for (int a = 0; fi && a < fi->AnimationCount; ++a)
		{
			granny_animation* an = fi->Animations[a];
			for (int g = 0; g < an->TrackGroupCount; ++g)
			{
				granny_track_group* tg = an->TrackGroups[g];
				if ((tg->Flags & 4) || tg->PeriodicLoop)
					printf("VDA/PERIODIC flags=0x%X periodic=%p tg=%s : %s\n", tg->Flags, (void*) tg->PeriodicLoop, tg->Name, name.c_str());
			}
			if (an->TrackGroupCount > 1 && argc > 2)
			{
				printf("MULTI %d:", an->TrackGroupCount);
				for (int g = 0; g < an->TrackGroupCount && g < 4; ++g) printf(" [%s]", an->TrackGroups[g]->Name);
				printf(" : %s\n", name.c_str());
			}
		}
		for (int m = 0; fi && m < fi->MeshCount; ++m)
		{
			granny_mesh* mesh = fi->Meshes[m];
			if (!mesh->PrimaryVertexData || !mesh->PrimaryTopology)
				printf("NOVERTEX mesh=%s vd=%p topo=%p : %s\n", mesh->Name, (void*) mesh->PrimaryVertexData, (void*) mesh->PrimaryTopology, name.c_str());
			else if (mesh->PrimaryTopology->IndexCount == 0 && mesh->PrimaryTopology->Index16Count == 0 && argc > 3)
				printf("NOINDEX mesh=%s : %s\n", mesh->Name, name.c_str());
		}
		GrannyFreeFile(f);
	}
	return 0;
}
