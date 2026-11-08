// Cynex: B-spline egri degerlendirme
#include "granny_curve.h"

#include <cmath>

namespace CynexGranny
{
	const float c_afIdentityPosition[3] = { 0.0f, 0.0f, 0.0f };
	const float c_afIdentityOrientation[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	const float c_afIdentityScaleShear[9] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	namespace
	{
		enum ECurveFormat
		{
			FORMAT_DA_KEYFRAMES32F = 0,
			FORMAT_DA_K32F_C32F = 1,
			FORMAT_DA_IDENTITY = 2,
			FORMAT_DA_CONSTANT32F = 3,
			FORMAT_D3_CONSTANT32F = 4,
			FORMAT_D4_CONSTANT32F = 5,
		};

		// Cynex: dongu kurallari granny ile ayni - ileri dongude son dugum yerine sonraki turun ilk dugumu
		// Cynex: r = kontrol indeksi, q = tur farki (-1 onceki tur, +1 sonraki tur)
		CYNEX_FORCEINLINE void WindowIndex(const SCurve & c_rCurve, int i, bool isBackwardLoop, bool isForwardLoop, int & r, int & q)
		{
			const int n = c_rCurve.iKnotCount;
			const int m = n - 1;
			q = 0;
			if (i < 0)
			{
				if (!isBackwardLoop || m <= 0)
				{
					r = 0;
					return;
				}
			}
			else if (i >= m)
			{
				if (!isForwardLoop || m <= 0)
				{
					r = i < n ? i : n - 1;
					return;
				}
			}
			else
			{
				r = i;
				return;
			}

			q = i / m;
			r = i - q * m;
			if (r < 0)
			{
				r += m;
				--q;
			}
		}

		CYNEX_FORCEINLINE float Dot4(const float * a, const float * b)
		{
			return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
		}

		CYNEX_FORCEINLINE void AlignQuaternion(float * q, const float * c_pfReference)
		{
			if (Dot4(q, c_pfReference) < 0.0f)
			{
				q[0] = -q[0];
				q[1] = -q[1];
				q[2] = -q[2];
				q[3] = -q[3];
			}
		}

		CYNEX_FORCEINLINE int FindKnot(const float * c_pKnots, int n, float t)
		{
			int lo = 0;
			int hi = n;
			while (lo < hi)
			{
				const int mid = (lo + hi) >> 1;
				if (c_pKnots[mid] > t)
					hi = mid;
				else
					lo = mid + 1;
			}
			return lo < n - 1 ? lo : n - 1;
		}
	}

	bool MakeCurve(const granny_curve2 & c_rCurve, int iDimension, SCurve & rOut)
	{
		rOut.pKnots = NULL;
		rOut.pControls = NULL;
		rOut.iKnotCount = 0;
		rOut.eKind = SCurve::KIND_IDENTITY;
		rOut.iDegree = 0;
		rOut.iDimension = (uint8_t) iDimension;

		const void * pObject = c_rCurve.CurveData.Object;
		if (!pObject)
			return true;

		granny_curve_data_header kHeader;
		memcpy(&kHeader, pObject, sizeof(kHeader));

		switch (kHeader.Format)
		{
			case FORMAT_DA_IDENTITY:
				return true;

			case FORMAT_DA_KEYFRAMES32F:
			{
				// Cynex: eski dosyalarda Dimension 0 olabiliyor, track turunun boyutu esas
				const granny_curve_data_da_keyframes32f * pData = static_cast<const granny_curve_data_da_keyframes32f *>(pObject);
				const int frames = pData->Controls ? pData->ControlCount / iDimension : 0;
				if (frames <= 0)
					return true;
				rOut.eKind = SCurve::KIND_KEYFRAMES;
				rOut.pControls = pData->Controls;
				rOut.iKnotCount = frames;
				return true;
			}

			case FORMAT_DA_CONSTANT32F:
			{
				const granny_curve_data_da_constant32f * pData = static_cast<const granny_curve_data_da_constant32f *>(pObject);
				if (pData->ControlCount < iDimension || !pData->Controls)
					break;
				rOut.eKind = SCurve::KIND_CONSTANT;
				rOut.pControls = pData->Controls;
				return true;
			}

			case FORMAT_D3_CONSTANT32F:
				if (iDimension != 3)
					break;
				rOut.eKind = SCurve::KIND_CONSTANT;
				rOut.pControls = static_cast<const granny_curve_data_d3_constant32f *>(pObject)->Controls;
				return true;

			case FORMAT_D4_CONSTANT32F:
				if (iDimension != 4)
					break;
				rOut.eKind = SCurve::KIND_CONSTANT;
				rOut.pControls = static_cast<const granny_curve_data_d4_constant32f *>(pObject)->Controls;
				return true;

			case FORMAT_DA_K32F_C32F:
			{
				const granny_curve_data_da_k32f_c32f * pData = static_cast<const granny_curve_data_da_k32f_c32f *>(pObject);
				if (pData->KnotCount == 0)
					return true;
				if (pData->KnotCount < 0 || !pData->Knots || !pData->Controls || pData->ControlCount != pData->KnotCount * iDimension)
					break;
				if (pData->KnotCount == 1)
				{
					rOut.eKind = SCurve::KIND_CONSTANT;
					rOut.pControls = pData->Controls;
					return true;
				}
				if (kHeader.Degree > CURVE_MAX_DEGREE)
					break;
				rOut.eKind = SCurve::KIND_BSPLINE;
				rOut.pKnots = pData->Knots;
				rOut.pControls = pData->Controls;
				rOut.iKnotCount = pData->KnotCount;
				rOut.iDegree = kHeader.Degree;
				return true;
			}

			default:
				break;
		}

		CYNEX_LOG(GrannyErrorLogMessage, GrannyCurveLogMessage, "Unsupported curve format %d (dimension %d), convert the file with gr2conv", (int) kHeader.Format, iDimension);
		return false;
	}

	void GetOrientationReference(const SCurve & c_rCurve, const float * c_pfRest, float * pReference)
	{
		if (c_rCurve.eKind == SCurve::KIND_IDENTITY || !c_rCurve.pControls)
			memcpy(pReference, c_afIdentityOrientation, sizeof(c_afIdentityOrientation));
		else
			memcpy(pReference, c_rCurve.pControls, 4 * sizeof(float));
		AlignQuaternion(pReference, c_pfRest);
	}

	bool IsCurveIdentity(const SCurve & c_rCurve, const float * c_pfIdentity, float fThreshold)
	{
		if (c_rCurve.eKind == SCurve::KIND_IDENTITY)
			return true;
		if (c_rCurve.eKind != SCurve::KIND_CONSTANT)
			return false;
		for (int i = 0; i < c_rCurve.iDimension; ++i)
			if (fabsf(c_rCurve.pControls[i] - c_pfIdentity[i]) > fThreshold)
				return false;
		return true;
	}

	void EvaluateCurve(const SCurve & c_rCurve, int iDimension, bool isBackwardLoop, bool isForwardLoop, float fDuration, float fTimeStep, float t, const float * c_pfIdentity, float * pResult)
	{
		if (c_rCurve.eKind == SCurve::KIND_IDENTITY)
		{
			memcpy(pResult, c_pfIdentity, iDimension * sizeof(float));
			return;
		}
		if (c_rCurve.eKind == SCurve::KIND_CONSTANT)
		{
			memcpy(pResult, c_rCurve.pControls, iDimension * sizeof(float));
			return;
		}

		if (c_rCurve.eKind == SCurve::KIND_KEYFRAMES)
		{
			int frame = fTimeStep > 0.0f ? (int) (t / fTimeStep) : 0;
			if (frame >= c_rCurve.iKnotCount)
				frame = c_rCurve.iKnotCount - 1;
			if (frame < 0)
				frame = 0;
			memcpy(pResult, c_rCurve.pControls + frame * iDimension, iDimension * sizeof(float));
			return;
		}

		const int degree = c_rCurve.iDegree;
		const int k = FindKnot(c_rCurve.pKnots, c_rCurve.iKnotCount, t);

		if (degree == 0)
		{
			int r, q;
			WindowIndex(c_rCurve, k, isBackwardLoop, isForwardLoop, r, q);
			memcpy(pResult, c_rCurve.pControls + r * iDimension, iDimension * sizeof(float));
			return;
		}

		// Cynex: ti[-d..d-1], pi[-d..0] penceresi + de Boor
		float afKnots[2 * CURVE_MAX_DEGREE];
		float afPoints[CURVE_MAX_DEGREE + 1][CURVE_MAX_DIMENSION];
		for (int j = -degree; j < degree; ++j)
		{
			int r, q;
			WindowIndex(c_rCurve, k + j, isBackwardLoop, isForwardLoop, r, q);
			afKnots[j + degree] = c_rCurve.pKnots[r] + (float) q * fDuration;
			if (j <= 0)
				memcpy(afPoints[j + degree], c_rCurve.pControls + r * iDimension, iDimension * sizeof(float));
		}

		if (iDimension == 4)
			for (int j = 1; j <= degree; ++j)
				AlignQuaternion(afPoints[j], afPoints[j - 1]);

		const float * ti = afKnots + degree;
		for (int r = 1; r <= degree; ++r)
		{
			for (int j = degree; j >= r; --j)
			{
				const float fLeft = ti[j - degree - 1];
				const float fSpan = ti[j - r] - fLeft;
				const float a = fSpan != 0.0f ? (t - fLeft) / fSpan : 0.0f;
				const float b = 1.0f - a;
				float * pDst = afPoints[j];
				const float * pPrev = afPoints[j - 1];
				for (int c = 0; c < iDimension; ++c)
					pDst[c] = b * pPrev[c] + a * pDst[c];
			}
		}
		memcpy(pResult, afPoints[degree], iDimension * sizeof(float));
	}
}
