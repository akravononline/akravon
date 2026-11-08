// Cynex: kara kutu deneme - GrannyGetMaterialTextureByType kullanim eslemesi
#include <granny.h>
#include <cstdio>
#include <cstring>

int main()
{
	const char* names[] = {
		"color", "specularColor", "normalCamera", "incandescence", "Ambient Color", "Diffuse Color", "RenderMaterial",
		"Specular Color", "Specular Level", "Self-Illumination", "Opacity", "Bump", "Reflection", "Refraction", "Displacement",
		"Umgebungsfarbe", "Streufarben", "Relief", "3D-Verschiebung", "Glanzfarben", "Reflexion", "Refraktion", "BaseColor",
		"Specularity", "Luminosity", "Transparency", "ReflectionImage", "RefractionImage", "ambience", "ambient", "diffuse",
		"specular", "radiance", "transparency", "reflectivity", "index_of_refraction", "normspec",
		"diffuse color", "DIFFUSE COLOR", "Diffuse", "opacity", "Diffuse Color ", "Filter Color", "Glossiness", "transparent",
		"bump", "Color", "cosinePower", "Unknown"
	};
	granny_texture tex;
	memset(&tex, 0, sizeof(tex));
	tex.FromFileName = "x.dds";
	granny_material sub;
	memset(&sub, 0, sizeof(sub));
	sub.Name = "sub";
	sub.Texture = &tex;
	for (const char* n : names)
	{
		granny_material_map map = { n, &sub };
		granny_material mat;
		memset(&mat, 0, sizeof(mat));
		mat.Name = "m";
		mat.MapCount = 1;
		mat.Maps = &map;
		printf("%-22s:", n);
		for (int t = 0; t < GrannyOnePastLastMaterialTextureType; ++t)
			printf(" %d", GrannyGetMaterialTextureByType(&mat, (granny_material_texture_type) t) ? 1 : 0);
		printf("\n");
	}
	// Cynex: ozyineleme - alt materyalin kendi haritasi
	{
		granny_material_map inner = { "Diffuse Color", &sub };
		granny_material mid;
		memset(&mid, 0, sizeof(mid));
		mid.Name = "mid"; mid.MapCount = 1; mid.Maps = &inner;
		granny_material_map outer = { "Diffuse Color", &mid };
		granny_material mat;
		memset(&mat, 0, sizeof(mat));
		mat.Name = "m"; mat.MapCount = 1; mat.Maps = &outer;
		printf("recursive: %p (tex=%p)\n", (void*) GrannyGetMaterialTextureByType(&mat, GrannyDiffuseColorTexture), (void*) &tex);
		granny_material_map outer2 = { "Opacity", &mid };
		mat.Maps = &outer2;
		printf("recursive opacity->mid(diffuse): %p\n", (void*) GrannyGetMaterialTextureByType(&mat, GrannyOpacityTexture));
		mat.Texture = &tex;
		mat.MapCount = 0;
		printf("direct texture no maps: diffuse=%p unknown=%p\n", (void*) GrannyGetMaterialTextureByType(&mat, GrannyDiffuseColorTexture), (void*) GrannyGetMaterialTextureByType(&mat, GrannyUnknownTextureType));
		printf("null material: %p\n", (void*) GrannyGetMaterialTextureByType(NULL, GrannyDiffuseColorTexture));
		granny_material_map two[2] = { { "Diffuse Color", NULL }, { "Diffuse Color", &sub } };
		mat.MapCount = 2; mat.Maps = two; mat.Texture = NULL;
		printf("null map material then valid: %p\n", (void*) GrannyGetMaterialTextureByType(&mat, GrannyDiffuseColorTexture));
		granny_material empty;
		memset(&empty, 0, sizeof(empty));
		granny_material_map two2[2] = { { "Diffuse Color", &empty }, { "Diffuse Color", &sub } };
		mat.Maps = two2;
		printf("textureless map material then valid: %p\n", (void*) GrannyGetMaterialTextureByType(&mat, GrannyDiffuseColorTexture));
	}
	return 0;
}
