// Cynex: kara kutu deneme - transform bileskesi, world pose, mesh binding, deform
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

static void QuatToMat(const float* q, float m[3][3])
{
	float x = q[0], y = q[1], z = q[2], w = q[3];
	// Cynex: satir vektor duzeni aday 1
	m[0][0] = 1 - 2*(y*y + z*z); m[0][1] = 2*(x*y + z*w);     m[0][2] = 2*(x*z - y*w);
	m[1][0] = 2*(x*y - z*w);     m[1][1] = 1 - 2*(x*x + z*z); m[1][2] = 2*(y*z + x*w);
	m[2][0] = 2*(x*z + y*w);     m[2][1] = 2*(y*z - x*w);     m[2][2] = 1 - 2*(x*x + y*y);
}

int main()
{
	// Cynex: 1) bileske transform
	granny_transform t;
	t.Flags = 7;
	t.Position[0] = 1; t.Position[1] = 2; t.Position[2] = 3;
	float q[4] = { 0.2f, -0.3f, 0.4f, 0.0f }; q[3] = sqrtf(1 - q[0]*q[0] - q[1]*q[1] - q[2]*q[2]);
	memcpy(t.Orientation, q, 16);
	float ss[3][3] = { { 1.5f, 0.1f, 0.2f }, { -0.3f, 0.8f, 0.05f }, { 0.4f, 0.0f, 1.2f } };
	memcpy(t.ScaleShear, ss, 36);
	float m[16];
	GrannyBuildCompositeTransform4x4(&t, m);
	printf("composite:\n"); for (int r = 0; r < 4; ++r) printf("  %9.5f %9.5f %9.5f %9.5f\n", m[r*4], m[r*4+1], m[r*4+2], m[r*4+3]);
	float R[3][3]; QuatToMat(q, R);
	float c1[3][3], c2[3][3], c3[3][3], c4[3][3];
	for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j)
	{
		c1[i][j] = c2[i][j] = c3[i][j] = c4[i][j] = 0;
		for (int k = 0; k < 3; ++k)
		{
			c1[i][j] += ss[i][k] * R[k][j];        // SS * R
			c2[i][j] += R[i][k] * ss[k][j];        // R * SS
			c3[i][j] += ss[k][i] * R[k][j];        // SS^T * R
			c4[i][j] += ss[i][k] * R[j][k];        // SS * R^T
		}
	}
	float (*cs[4])[3] = { c1, c2, c3, c4 };
	for (int c = 0; c < 4; ++c)
	{
		float e = 0;
		for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) e = fmaxf(e, fabsf(cs[c][i][j] - m[i*4+j]));
		printf("cand %d err %g\n", c, e);
	}
	for (unsigned f = 0; f < 8; ++f)
	{
		t.Flags = f;
		GrannyBuildCompositeTransform4x4(&t, m);
		printf("flags %u: row0 %g %g %g row3 %g %g %g\n", f, m[0], m[1], m[2], m[12], m[13], m[14]);
	}

	// Cynex: 2) world pose ve composite
	const char* dir = "C:/CynexMobilBuild/granny/testdata/warrior/";
	char p[512];
	sprintf(p, "%swarrior_novice.gr2", dir); granny_file_info* im = GrannyGetFileInfo(Load(p));
	sprintf(p, "%shair_10_1.gr2", dir); granny_file_info* ih = GrannyGetFileInfo(Load(p));
	granny_model* body = im->Models[0];
	granny_skeleton* sk = body->Skeleton;
	granny_local_pose* lp = GrannyNewLocalPose(sk->BoneCount);
	GrannyBuildRestLocalPose(sk, 0, sk->BoneCount, lp);
	// Cynex: biraz bozalim ki olcek/rotasyon olsun
	for (int b = 0; b < sk->BoneCount; ++b)
	{
		granny_transform* lt = GrannyGetLocalPoseTransform(lp, b);
		lt->Position[0] += 0.1f * b;
	}
	granny_world_pose* wp = GrannyNewWorldPose(sk->BoneCount);
	float offset[16] = { 0,1,0,0, -1,0,0,0, 0,0,1,0, 5,6,7,1 };
	GrannyBuildWorldPose(sk, 0, sk->BoneCount, lp, offset, wp);
	float maxw = 0, maxc = 0;
	std::vector<float> W(16 * sk->BoneCount);
	for (int b = 0; b < sk->BoneCount; ++b)
	{
		float L[16];
		GrannyBuildCompositeTransform4x4(GrannyGetLocalPoseTransform(lp, b), L);
		const float* P = sk->Bones[b].ParentIndex >= 0 ? &W[16 * sk->Bones[b].ParentIndex] : offset;
		float* Wb = &W[16 * b];
		for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) { float s = 0; for (int k = 0; k < 4; ++k) s += L[i*4+k] * P[k*4+j]; Wb[i*4+j] = s; }
		const float* g = GrannyGetWorldPose4x4(wp, b);
		for (int k = 0; k < 16; ++k) maxw = fmaxf(maxw, fabsf(g[k] - Wb[k]));
		float C[16];
		const float* IW = &sk->Bones[b].InverseWorld4x4[0][0];
		for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) { float s = 0; for (int k = 0; k < 4; ++k) s += IW[i*4+k] * Wb[k*4+j]; C[i*4+j] = s; }
		const float* gc = GrannyGetWorldPoseComposite4x4(wp, b);
		for (int k = 0; k < 16; ++k) maxc = fmaxf(maxc, fabsf(gc[k] - C[k]));
	}
	printf("world err %g composite err %g\n", maxw, maxc);
	GrannyBuildWorldPose(sk, 0, sk->BoneCount, lp, NULL, wp);
	printf("null offset: w0 row3 %g %g %g\n", GrannyGetWorldPose4x4(wp, 0)[12], GrannyGetWorldPose4x4(wp, 0)[13], GrannyGetWorldPose4x4(wp, 0)[14]);
	printf("GetWorldPose4x4 out of range: %p %p\n", (void*) GrannyGetWorldPose4x4(wp, sk->BoneCount), (void*) GrannyGetWorldPose4x4(wp, -1));

	// Cynex: 3) mesh binding
	for (granny_file_info* fi : { im, ih })
	{
		granny_model* mdl = fi->Models[0];
		printf("model %s skel %s bones %d meshes %d\n", mdl->Name, mdl->Skeleton->Name, mdl->Skeleton->BoneCount, mdl->MeshBindingCount);
		for (int mb = 0; mb < mdl->MeshBindingCount; ++mb)
		{
			granny_mesh* mesh = mdl->MeshBindings[mb].Mesh;
			granny_mesh_binding* bind = GrannyNewMeshBinding(mesh, mdl->Skeleton, sk);
			printf(" mesh %s rigid=%d bonebindings=%d binding bonecount=%d transferred=%d\n", mesh->Name, GrannyMeshIsRigid(mesh), mesh->BoneBindingCount,
				GrannyGetMeshBindingBoneCount(bind), GrannyMeshBindingIsTransferred(bind));
			const granny_int32x* to = GrannyGetMeshBindingToBoneIndices(bind);
			const granny_int32x* from = GrannyGetMeshBindingFromBoneIndices(bind);
			for (int i = 0; i < mesh->BoneBindingCount && i < 6; ++i)
			{
				int expect = -1;
				GrannyFindBoneByName(sk, mesh->BoneBindings[i].BoneName, &expect);
				printf("   %s -> to %d (byname %d) from %d\n", mesh->BoneBindings[i].BoneName, to[i], expect, from[i]);
			}
			GrannyFreeMeshBinding(bind);
		}
	}
	// Cynex: eksik kemik
	{
		granny_mesh* mesh = ih->Models[0]->MeshBindings[0].Mesh;
		granny_bone_binding saved = mesh->BoneBindings[0];
		mesh->BoneBindings[0].BoneName = "YokBoyleKemik";
		granny_mesh_binding* bind = GrannyNewMeshBinding(mesh, ih->Models[0]->Skeleton, sk);
		printf("missing bone binding=%p", (void*) bind);
		if (bind) printf(" to0=%d", GrannyGetMeshBindingToBoneIndices(bind)[0]);
		printf("\n");
		if (bind) GrannyFreeMeshBinding(bind);
		mesh->BoneBindings[0] = saved;
	}
	// Cynex: 4) FindBoneByName harf duyarliligi
	{
		int idx = -1;
		bool r1 = GrannyFindBoneByName(sk, "bip01 head", &idx); printf("lower: %d %d\n", r1, idx);
		bool r2 = GrannyFindBoneByName(sk, "Bip01 Head", &idx); printf("exact: %d %d\n", r2, idx);
		bool r3 = GrannyFindBoneByName(sk, "BIP01 HEAD", &idx); printf("upper: %d %d\n", r3, idx);
		idx = 1234; bool r4 = GrannyFindBoneByName(sk, "yok", &idx); printf("missing: %d %d\n", r4, idx);
	}
	return 0;
}
