// Cynex: kara kutu deneme - MeshIsRigid kurali
#include <windows.h>
#include <granny.h>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <vector>
namespace fs = std::filesystem;

int wmain(int argc, wchar_t** argv)
{
	std::vector<char> buf;
	int counts[4][2] = {};
	int total = 0, files = 0;
	for (auto& e : fs::recursive_directory_iterator(argv[1]))
	{
		if (!e.is_regular_file() || _wcsicmp(e.path().extension().c_str(), L".gr2")) continue;
		if (++files > 3000) break;
		FILE* fp = _wfopen(e.path().c_str(), L"rb");
		fseek(fp, 0, SEEK_END); long n = ftell(fp); fseek(fp, 0, SEEK_SET);
		buf.resize(n); fread(buf.data(), 1, n, fp); fclose(fp);
		granny_file* f = GrannyReadEntireFileFromMemory((int) n, buf.data());
		if (!f) continue;
		granny_file_info* fi = GrannyGetFileInfo(f);
		for (int m = 0; fi && m < fi->MeshCount; ++m)
		{
			granny_mesh* mesh = fi->Meshes[m];
			bool rigid = GrannyMeshIsRigid(mesh);
			bool hasIdx = false;
			granny_data_type_definition* vt = GrannyGetMeshVertexType(mesh);
			for (granny_data_type_definition* d = vt; d && d->Type != GrannyEndMember; ++d)
				if (!strcmp(d->Name, GrannyVertexBoneIndicesName)) hasIdx = true;
			bool h0 = mesh->BoneBindingCount <= 1;
			bool h1 = !hasIdx;
			bool h2 = h0 || h1;
			bool h3 = mesh->BoneBindingCount == 1;
			bool hs[4] = { h0, h1, h2, h3 };
			for (int h = 0; h < 4; ++h) counts[h][hs[h] == rigid]++;
			++total;
			if (rigid != h2) printf("mismatch h2: %s bb=%d hasIdx=%d rigid=%d\n", mesh->Name, mesh->BoneBindingCount, hasIdx, rigid);
		}
		GrannyFreeFile(f);
	}
	printf("meshes %d\n", total);
	for (int h = 0; h < 4; ++h) printf("h%d: wrong %d right %d\n", h, counts[h][0], counts[h][1]);
	return 0;
}
