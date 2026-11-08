#include <granny.h>
#include <cstdio>
#include <cstring>
int main()
{
	struct { granny_data_type_definition** t; const char* n; } types[] = {
		{ &GrannyCurveDataDaKeyframes32fType, "DaKeyframes32f" }, { &GrannyCurveDataDaK32fC32fType, "DaK32fC32f" },
		{ &GrannyCurveDataDaIdentityType, "DaIdentity" }, { &GrannyCurveDataDaConstant32fType, "DaConstant32f" },
		{ &GrannyCurveDataD3Constant32fType, "D3Constant32f" }, { &GrannyCurveDataD4Constant32fType, "D4Constant32f" },
		{ &GrannyCurveDataDaK16uC16uType, "DaK16uC16u" }, { &GrannyCurveDataDaK8uC8uType, "DaK8uC8u" },
		{ &GrannyCurveDataD4nK16uC15uType, "D4nK16uC15u" }, { &GrannyCurveDataD4nK8uC7uType, "D4nK8uC7u" },
		{ &GrannyCurveDataD3K16uC16uType, "D3K16uC16u" }, { &GrannyCurveDataD3K8uC8uType, "D3K8uC8u" },
		{ &GrannyCurveDataD9I1K16uC16uType, "D9I1K16uC16u" }, { &GrannyCurveDataD9I3K16uC16uType, "D9I3K16uC16u" },
		{ &GrannyCurveDataD9I1K8uC8uType, "D9I1K8uC8u" }, { &GrannyCurveDataD9I3K8uC8uType, "D9I3K8uC8u" },
		{ &GrannyCurveDataD3I1K32fC32fType, "D3I1K32fC32f" }, { &GrannyCurveDataD3I1K16uC16uType, "D3I1K16uC16u" },
		{ &GrannyCurveDataD3I1K8uC8uType, "D3I1K8uC8u" },
	};
	for (auto& t : types)
	{
		granny_curve_builder* b = GrannyBeginCurve(*t.t, 0, 1, 1);
		float knot = 0, ctrl[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
		GrannyPushCurveKnotArray(b, &knot);
		GrannyPushCurveControlArray(b, ctrl);
		granny_curve2* c = GrannyEndCurve(b);
		printf("%-16s format=%d degree=%d\n", t.n, c ? ((granny_curve_data_header*) c->CurveData.Object)->Format : -1, c ? ((granny_curve_data_header*) c->CurveData.Object)->Degree : -1);
		if (c) GrannyFreeCurve(c);
	}
	printf("MV64LE:"); for (int i = 0; i < 8; ++i) printf(" %08x", ((const unsigned*) GrannyGRNFileMV_64Bit_LittleEndian)[i]); printf("\n");
	printf("MV32LE:"); for (int i = 0; i < 8; ++i) printf(" %08x", ((const unsigned*) GrannyGRNFileMV_32Bit_LittleEndian)[i]); printf("\n");
	printf("MV64BE:"); for (int i = 0; i < 8; ++i) printf(" %08x", ((const unsigned*) GrannyGRNFileMV_64Bit_BigEndian)[i]); printf("\n");
	printf("MV32BE:"); for (int i = 0; i < 8; ++i) printf(" %08x", ((const unsigned*) GrannyGRNFileMV_32Bit_BigEndian)[i]); printf("\n");
	printf("MVOld :"); for (int i = 0; i < 8; ++i) printf(" %08x", ((const unsigned*) GrannyGRNFileMV_Old)[i]); printf("\n");
	printf("ident pos %g %g %g ori %g %g %g %g\n", GrannyCurveIdentityPosition[0], GrannyCurveIdentityPosition[1], GrannyCurveIdentityPosition[2], GrannyCurveIdentityOrientation[0], GrannyCurveIdentityOrientation[1], GrannyCurveIdentityOrientation[2], GrannyCurveIdentityOrientation[3]);
	return 0;
}
