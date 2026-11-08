// Cynex: egri ornekleme (gr2conv sonrasi formatlar: DaK32fC32f, DaKeyframes32f, DaIdentity, Da/D3/D4Constant32f)
#pragma once

#include "granny_internal.h"

namespace CynexGranny
{
	enum
	{
		CURVE_MAX_DEGREE = 7,
		CURVE_MAX_DIMENSION = 9,
	};

	struct SCurve
	{
		enum EKind
		{
			KIND_IDENTITY,
			KIND_CONSTANT,
			KIND_BSPLINE,
			KIND_KEYFRAMES,
		};

		const float *	pKnots;
		const float *	pControls;
		int				iKnotCount;		// Cynex: keyframe'de kare sayisi
		uint8_t			eKind;
		uint8_t			iDegree;
		uint8_t			iDimension;
	};

	extern const float c_afIdentityPosition[3];
	extern const float c_afIdentityOrientation[4];
	extern const float c_afIdentityScaleShear[9];

	// Cynex: bilinmeyen format false doner (sonuc birim kabul edilir)
	bool	MakeCurve(const granny_curve2 & c_rCurve, int iDimension, SCurve & rOut);
	bool	IsCurveIdentity(const SCurve & c_rCurve, const float * c_pfIdentity, float fThreshold);

	// Cynex: yonelim karisim referansi: egrinin ilk kontrol noktasi, rest ile ayni yarim kureye alinmis
	void	GetOrientationReference(const SCurve & c_rCurve, const float * c_pfRest, float * pReference);

	// Cynex: keyframe egriler kare indeksiyle: anahtar[t / fTimeStep]
	// Cynex: yonelimde pencere noktalari ilk noktadan zincirlenir (granny EvaluateCurveAtT ile ayni)
	void	EvaluateCurve(const SCurve & c_rCurve, int iDimension, bool isBackwardLoop, bool isForwardLoop, float fDuration, float fTimeStep, float t, const float * c_pfIdentity, float * pResult);
}
