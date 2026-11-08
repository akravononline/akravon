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
	granny_file_info* fi = GrannyGetFileInfo(Load(argv[1]));
	for (int m = 0; m < fi->ModelCount; ++m)
		for (int b = 0; b < fi->Models[m]->MeshBindingCount; ++b)
		{
			granny_mesh* mesh = fi->Models[m]->MeshBindings[b].Mesh;
			granny_data_type_definition* t = GrannyGetMeshVertexType(mesh);
			printf("mesh %s vd=%p type=%p (PNT332=%p) count=%d verts=%p idx=%d topo=%p\n", mesh->Name, (void*) mesh->PrimaryVertexData, (void*) t, (void*) GrannyPNT332VertexType, GrannyGetMeshVertexCount(mesh), GrannyGetMeshVertices(mesh), GrannyGetMeshIndexCount(mesh), (void*) mesh->PrimaryTopology);
			if (t) for (granny_data_type_definition* d = t; d->Type != GrannyEndMember; ++d) printf("   %s %d %d\n", d->Name, d->Type, d->ArrayWidth);
		}
	return 0;
}
