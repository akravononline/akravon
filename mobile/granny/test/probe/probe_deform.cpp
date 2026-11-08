// Cynex: kara kutu deneme - deform, vertex kopyalama, tip boyutu, extended data
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

granny_data_type_definition PNT3322[5] =
{
	{GrannyReal32Member, GrannyVertexPositionName, 0, 3},
	{GrannyReal32Member, GrannyVertexNormalName, 0, 3},
	{GrannyReal32Member, GrannyVertexTextureCoordinatesName "0", 0, 2},
	{GrannyReal32Member, GrannyVertexTextureCoordinatesName "1", 0, 2},
	{GrannyEndMember}
};

int main(int argc, char** argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	const char* path = argc > 1 ? argv[1] : "C:/CynexMobilBuild/granny/testdata/warrior/warrior_novice.gr2";
	granny_file_info* fi = GrannyGetFileInfo(Load(path));
	granny_model* mdl = fi->Models[0];
	granny_skeleton* sk = mdl->Skeleton;
	granny_local_pose* lp = GrannyNewLocalPose(sk->BoneCount);
	GrannyBuildRestLocalPose(sk, 0, sk->BoneCount, lp);
	for (int b = 0; b < sk->BoneCount; ++b)
	{
		granny_transform* t = GrannyGetLocalPoseTransform(lp, b);
		t->Orientation[0] += 0.05f * (b % 5); t->Orientation[2] -= 0.03f * (b % 3);
		float l = sqrtf(t->Orientation[0]*t->Orientation[0]+t->Orientation[1]*t->Orientation[1]+t->Orientation[2]*t->Orientation[2]+t->Orientation[3]*t->Orientation[3]);
		for (int k = 0; k < 4; ++k) t->Orientation[k] /= l;
	}
	granny_world_pose* wp = GrannyNewWorldPose(sk->BoneCount);
	GrannyBuildWorldPose(sk, 0, sk->BoneCount, lp, NULL, wp);
	const float* comp = (const float*) GrannyGetWorldPoseComposite4x4Array(wp);

	printf("sizeof(data_type_definition)=%d PNT332 total=%d PNT3322 total=%d\n", (int) sizeof(granny_data_type_definition), GrannyGetTotalTypeSize(GrannyPNT332VertexType), GrannyGetTotalTypeSize(PNT3322));
	for (int mb = 0; mb < mdl->MeshBindingCount; ++mb)
	{
		granny_mesh* mesh = mdl->MeshBindings[mb].Mesh;
		granny_data_type_definition* vt = GrannyGetMeshVertexType(mesh);
		int n = GrannyGetMeshVertexCount(mesh);
		printf("mesh %s rigid=%d verts=%d idx=%d typesize=%d groups=%d\n", mesh->Name, GrannyMeshIsRigid(mesh), n, GrannyGetMeshIndexCount(mesh), GrannyGetTotalTypeSize(vt), GrannyGetMeshTriangleGroupCount(mesh));
		std::vector<granny_pnt332_vertex> v332(n);
		GrannyCopyMeshVertices(mesh, GrannyPNT332VertexType, v332.data());
		std::vector<float> v3322(n * 10, -777.0f);
		GrannyCopyMeshVertices(mesh, PNT3322, v3322.data());
		printf("  v0 PNT332 p=(%g %g %g) n=(%g %g %g) uv=(%g %g)\n", v332[0].Position[0], v332[0].Position[1], v332[0].Position[2], v332[0].Normal[0], v332[0].Normal[1], v332[0].Normal[2], v332[0].UV[0], v332[0].UV[1]);
		printf("  v0 PNT3322 uv1=(%g %g)\n", v3322[8], v3322[9]);
		const granny_uint8* raw = (const granny_uint8*) GrannyGetMeshVertices(mesh);
		int off = 0;
		for (granny_data_type_definition* d = vt; d->Type != GrannyEndMember; ++d)
		{
			printf("  member %s type %d width %d offset %d\n", d->Name, d->Type, d->ArrayWidth, off);
			off += GrannyGetMemberTypeSize(d);
		}
		int ic = GrannyGetMeshIndexCount(mesh);
		std::vector<granny_uint16> i16(ic);
		GrannyCopyMeshIndices(mesh, 2, i16.data());
		std::vector<granny_uint32> i32(ic);
		GrannyCopyMeshIndices(mesh, 4, i32.data());
		printf("  idx16[0..2]=%d %d %d idx32=%u %u %u\n", i16[0], i16[1], i16[2], i32[0], i32[1], i32[2]);
		if (GrannyMeshIsRigid(mesh))
			continue;
		granny_mesh_binding* bind = GrannyNewMeshBinding(mesh, sk, sk);
		const granny_int32x* toBone = GrannyGetMeshBindingToBoneIndices(bind);
		for (int dontallow = 0; dontallow < 2; ++dontallow)
		{
			granny_mesh_deformer* def = GrannyNewMeshDeformer(vt, GrannyPNT332VertexType, GrannyDeformPositionNormal, dontallow ? GrannyDontAllowUncopiedTail : GrannyAllowUncopiedTail);
			std::vector<granny_pnt332_vertex> out(n);
			memset(out.data(), 0xCD, n * sizeof(granny_pnt332_vertex));
			GrannyDeformVertices(def, toBone, comp, n, raw, out.data());
			float ep = 0, en = 0, ept = 0, enr = 0;
			int stride = GrannyGetTotalObjectSize(vt);
			for (int i = 0; i < n; ++i)
			{
				const granny_uint8* v = raw + i * stride;
				const float* pos = (const float*) v;
				const granny_uint8* w = v + 12;
				const granny_uint8* bi = v + 16;
				const float* nrm = (const float*) (v + 20);
				float rp[3] = {}, rn[3] = {}, rp2[3] = {}, rn2[3] = {};
				int wsum = w[0] + w[1] + w[2] + w[3];
				for (int k = 0; k < 4; ++k)
				{
					if (!w[k]) continue;
					float wf = w[k] / 255.0f;
					float wf2 = w[k] / (float) wsum;
					const float* M = comp + 16 * toBone[bi[k]];
					for (int j = 0; j < 3; ++j)
					{
						float tp = pos[0]*M[0*4+j] + pos[1]*M[1*4+j] + pos[2]*M[2*4+j] + M[12+j];
						float tn = nrm[0]*M[0*4+j] + nrm[1]*M[1*4+j] + nrm[2]*M[2*4+j];
						rp[j] += wf * tp; rn[j] += wf * tn; rp2[j] += wf2 * tp; rn2[j] += wf2 * tn;
					}
				}
				float ln = sqrtf(rn[0]*rn[0]+rn[1]*rn[1]+rn[2]*rn[2]);
				for (int j = 0; j < 3; ++j)
				{
					ep = fmaxf(ep, fabsf(rp[j] - out[i].Position[j]));
					en = fmaxf(en, fabsf(rn[j] - out[i].Normal[j]));
					ept = fmaxf(ept, fabsf(rp2[j] - out[i].Position[j]));
					enr = fmaxf(enr, fabsf(rn[j]/ln - out[i].Normal[j]));
				}
				if (i == 0) printf("  deform v0 w=(%d %d %d %d) bi=(%d %d %d %d) out p=(%g %g %g) n=(%g %g %g) uv=(%g %g) wsum=%d\n", w[0], w[1], w[2], w[3], bi[0], bi[1], bi[2], bi[3],
					out[i].Position[0], out[i].Position[1], out[i].Position[2], out[i].Normal[0], out[i].Normal[1], out[i].Normal[2], out[i].UV[0], out[i].UV[1], wsum);
			}
			printf("  deform(dontallow=%d) err pos/255 %g nrm/255 %g pos/sum %g nrmnormalized %g\n", dontallow, ep, en, ept, enr);
			GrannyFreeMeshDeformer(def);
		}
		GrannyFreeMeshBinding(bind);
	}
	for (int m = 0; m < fi->MaterialCount; ++m)
	{
		granny_material* mat = fi->Materials[m];
		granny_variant res; memset(&res, 0xAB, sizeof(res));
		bool ok = GrannyFindMatchingMember(mat->ExtendedData.Type, mat->ExtendedData.Object, "Two-sided", &res);
		granny_int32 two = -5;
		granny_data_type_definition def[] = { {GrannyInt32Member, "Two-sided"}, {GrannyEndMember} };
		if (res.Type && ok) GrannyConvertSingleObject(res.Type, res.Object, def, &two, 0);
		printf("material %s maps=%d ok=%d type=%p obj=%p two=%d\n", mat->Name, mat->MapCount, ok, (void*) res.Type, res.Object, two);
		if (res.Type && ok) printf("  res type member: %d %s width %d\n", res.Type->Type, res.Type->Name ? res.Type->Name : "?", res.Type->ArrayWidth);
		granny_texture* tex = GrannyGetMaterialTextureByType(mat, GrannyDiffuseColorTexture);
		granny_texture* opa = GrannyGetMaterialTextureByType(mat, GrannyOpacityTexture);
		printf("  diffuse=%s opacity=%s tex=%p\n", tex ? tex->FromFileName : "-", opa ? opa->FromFileName : "-", (void*) mat->Texture);
		for (int k = 0; k < mat->MapCount; ++k) printf("  map %d usage=%s mat=%s tex=%p\n", k, mat->Maps[k].Usage, mat->Maps[k].Material ? mat->Maps[k].Material->Name : "-", mat->Maps[k].Material ? (void*) mat->Maps[k].Material->Texture : 0);
	}
	{
		granny_variant res; memset(&res, 0xAB, sizeof(res));
		bool ok = GrannyFindMatchingMember(NULL, NULL, "Two-sided", &res);
		printf("null type: ok=%d type=%p obj=%p\n", ok, (void*) res.Type, res.Object);
		memset(&res, 0xAB, sizeof(res));
		ok = GrannyFindMatchingMember(fi->Materials[0]->ExtendedData.Type, fi->Materials[0]->ExtendedData.Object, "YokBoyle", &res);
		printf("missing: ok=%d type=%p obj=%p\n", ok, (void*) res.Type, res.Object);
		memset(&res, 0xAB, sizeof(res));
		ok = GrannyFindMatchingMember(fi->Materials[0]->ExtendedData.Type, fi->Materials[0]->ExtendedData.Object, "two-SIDED", &res);
		printf("case: ok=%d type=%p obj=%p\n", ok, (void*) res.Type, res.Object);
	}
	return 0;
}
