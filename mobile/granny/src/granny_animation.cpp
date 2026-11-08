// Cynex: model instance, kontrol (saat/dongu/ease), animasyon ornekleme ve kok hareketi
#include "granny_curve.h"
#include "granny_pose.h"

#include <cctype>
#include <cfloat>
#include <cmath>
#include <mutex>
#include <unordered_map>

using namespace CynexGranny;

namespace CynexGranny
{
	struct SBoundTrack
	{
		int				iBone;
		granny_uint32	uFlags;
		bool			isDirectOrientation;	// Cynex: keyframe yonelim komsuluk olmadan karisir
		float			afReference[4];			// Cynex: komsuluk referansi (ilk kontrol noktasi, rest yarim kuresinde)
		SCurve			kPosition;
		SCurve			kOrientation;
		SCurve			kScaleShear;
	};

	// Cynex: (track group, iskelet) basina bir kez kurulur, kontroller paylasir
	struct SAnimationBinding
	{
		const granny_track_group *	pTrackGroup;
		const granny_skeleton *		pSkeleton;
		int							iRefCount;
		bool						isCached;
		int							iTrackCount;
		SBoundTrack *				pTracks;
		SAnimationBinding *			pPrevUnused;
		SAnimationBinding *			pNextUnused;
	};
}

namespace granny
{
	struct control
	{
		model_instance *					Owner;
		control *							Prev;
		control *							Next;
		const granny_animation *			Animation;
		const granny_track_group *			TrackGroup;
		CynexGranny::SAnimationBinding *	Binding;
		bool								HasAccumulation;
		bool								EaseIn;
		bool								EaseOut;
		bool								FreeOnceUnused;
		float								CurrentClock;
		float								LocalClock;
		float								LocalDuration;
		float								Speed;
		float								Weight;
		float								CompletionClock;
		int									LoopIndex;
		int									LoopCount;
		float								EaseInStart;
		float								EaseInEnd;
		float								EaseInValues[4];
		float								EaseOutStart;
		float								EaseOutEnd;
		float								EaseOutValues[4];
	};

	struct model_instance
	{
		const granny_model *	Model;
		control *				FirstControl;
		control *				LastControl;
	};
}

namespace
{
	/////////////////////////////////////////////////////////////////////////
	// Cynex: baglama onbellegi

	struct SBindingKey
	{
		const void *	pGroup;
		const void *	pSkeleton;

		bool operator==(const SBindingKey & rhs) const
		{
			return pGroup == rhs.pGroup && pSkeleton == rhs.pSkeleton;
		}
	};

	struct SBindingKeyHash
	{
		size_t operator()(const SBindingKey & k) const
		{
			const size_t a = reinterpret_cast<uintptr_t>(k.pGroup);
			const size_t b = reinterpret_cast<uintptr_t>(k.pSkeleton);
			return a ^ (b + (size_t) 0x9E3779B9u + (a << 6) + (a >> 2));
		}
	};

	enum
	{
		MAX_UNUSED_BINDINGS = 1024,
	};

	std::mutex															gs_kBindingLock;
	std::unordered_map<SBindingKey, SAnimationBinding *, SBindingKeyHash>	gs_kBindings;
	SAnimationBinding *													gs_pUnusedHead = NULL;
	SAnimationBinding *													gs_pUnusedTail = NULL;
	int																	gs_iUnusedCount = 0;

	void UnusedUnlink(SAnimationBinding * p)
	{
		if (p->pPrevUnused)
			p->pPrevUnused->pNextUnused = p->pNextUnused;
		else
			gs_pUnusedHead = p->pNextUnused;
		if (p->pNextUnused)
			p->pNextUnused->pPrevUnused = p->pPrevUnused;
		else
			gs_pUnusedTail = p->pPrevUnused;
		p->pPrevUnused = p->pNextUnused = NULL;
		--gs_iUnusedCount;
	}

	void UnusedPushBack(SAnimationBinding * p)
	{
		p->pNextUnused = NULL;
		p->pPrevUnused = gs_pUnusedTail;
		if (gs_pUnusedTail)
			gs_pUnusedTail->pNextUnused = p;
		else
			gs_pUnusedHead = p;
		gs_pUnusedTail = p;
		++gs_iUnusedCount;
	}

	void DestroyBinding(SAnimationBinding * p)
	{
		AlignedFree(p->pTracks);
		AlignedFree(p);
	}

	granny_uint32 TrackFlags(const SBoundTrack & r)
	{
		granny_uint32 flags = 0;
		if (!IsCurveIdentity(r.kPosition, c_afIdentityPosition, GrannyPositionIdentityThreshold))
			flags |= GrannyHasPosition;
		if (!IsCurveIdentity(r.kOrientation, c_afIdentityOrientation, GrannyOrientationIdentityThreshold))
			flags |= GrannyHasOrientation;
		if (!IsCurveIdentity(r.kScaleShear, c_afIdentityScaleShear, GrannyScaleShearIdentityThreshold))
			flags |= GrannyHasScaleShear;
		return flags;
	}

	int FindTrackLinear(const granny_track_group * c_pGroup, const char * c_szName)
	{
		for (int t = 0; t < c_pGroup->TransformTrackCount; ++t)
		{
			const char * c_szTrack = c_pGroup->TransformTracks[t].Name;
			if (c_szTrack && strcmp(c_szTrack, c_szName) == 0)
				return t;
		}
		return -1;
	}

	int FindTrackSorted(const granny_track_group * c_pGroup, const char * c_szName)
	{
		int lo = 0;
		int hi = c_pGroup->TransformTrackCount;
		while (lo < hi)
		{
			const int mid = (lo + hi) >> 1;
			const char * c_szTrack = c_pGroup->TransformTracks[mid].Name;
			const int cmp = strcmp(c_szName, c_szTrack ? c_szTrack : "");
			if (cmp < 0)
				hi = mid;
			else if (cmp > 0)
				lo = mid + 1;
			else
				return mid;
		}
		return -1;
	}

	SAnimationBinding * CreateBinding(const granny_track_group * c_pGroup, const granny_skeleton * c_pSkeleton)
	{
		SAnimationBinding * p = static_cast<SAnimationBinding *>(AlignedAlloc(sizeof(SAnimationBinding), 16));
		memset(p, 0, sizeof(*p));
		p->pTrackGroup = c_pGroup;
		p->pSkeleton = c_pSkeleton;

		const int boneCount = c_pSkeleton->BoneCount;
		p->pTracks = AllocArray<SBoundTrack>(boneCount > 0 ? (size_t) boneCount : 1);

		// Cynex: kemik - track eslesmesi isimle, harf duyarli; sirali grupta granny gibi ikili arama (ayni isimde hangisi secilecegi buna bagli)
		const bool isSorted = (c_pGroup->Flags & GrannyTrackGroupIsSorted) != 0;
		for (int b = 0; b < boneCount; ++b)
		{
			const char * c_szBone = c_pSkeleton->Bones[b].Name;
			if (!c_szBone)
				continue;
			const int t = isSorted ? FindTrackSorted(c_pGroup, c_szBone) : FindTrackLinear(c_pGroup, c_szBone);
			if (t < 0)
				continue;
			const granny_transform_track & rTrack = c_pGroup->TransformTracks[t];
			SBoundTrack & rBound = p->pTracks[p->iTrackCount++];
			rBound.iBone = b;
			MakeCurve(rTrack.PositionCurve, 3, rBound.kPosition);
			MakeCurve(rTrack.OrientationCurve, 4, rBound.kOrientation);
			MakeCurve(rTrack.ScaleShearCurve, 9, rBound.kScaleShear);
			rBound.uFlags = TrackFlags(rBound);
			rBound.isDirectOrientation = rBound.kOrientation.eKind == SCurve::KIND_KEYFRAMES;
			GetOrientationReference(rBound.kOrientation, c_pSkeleton->Bones[b].LocalTransform.Orientation, rBound.afReference);
		}

		return p;
	}

	SAnimationBinding * AcquireBinding(const granny_track_group * c_pGroup, const granny_skeleton * c_pSkeleton)
	{
		std::lock_guard<std::mutex> kGuard(gs_kBindingLock);
		const SBindingKey kKey = { c_pGroup, c_pSkeleton };
		auto it = gs_kBindings.find(kKey);
		if (it != gs_kBindings.end())
		{
			SAnimationBinding * p = it->second;
			if (p->iRefCount++ == 0)
				UnusedUnlink(p);
			return p;
		}

		SAnimationBinding * p = CreateBinding(c_pGroup, c_pSkeleton);
		p->iRefCount = 1;
		p->isCached = true;
		gs_kBindings.emplace(kKey, p);
		return p;
	}

	void ReleaseBinding(SAnimationBinding * p)
	{
		if (!p)
			return;
		std::lock_guard<std::mutex> kGuard(gs_kBindingLock);
		if (--p->iRefCount > 0)
			return;

		if (!p->isCached)
		{
			DestroyBinding(p);
			return;
		}

		UnusedPushBack(p);
		while (gs_iUnusedCount > MAX_UNUSED_BINDINGS)
		{
			SAnimationBinding * pOld = gs_pUnusedHead;
			UnusedUnlink(pOld);
			const SBindingKey kKey = { pOld->pTrackGroup, pOld->pSkeleton };
			gs_kBindings.erase(kKey);
			DestroyBinding(pOld);
		}
	}

	/////////////////////////////////////////////////////////////////////////
	// Cynex: kontrol yardimcilari

	bool NamesEqualNoCase(const char * a, const char * b)
	{
		if (!a || !b)
			return false;
		for (;; ++a, ++b)
		{
			const int ca = tolower((unsigned char) *a);
			const int cb = tolower((unsigned char) *b);
			if (ca != cb)
				return false;
			if (!ca)
				return true;
		}
	}

	void NormalizeLoop(granny_control * c)
	{
		const float fDuration = c->LocalDuration;
		if (!(fDuration > 0.0f))
			return;

		if (c->LoopCount <= 0)
		{
			if (c->LocalClock >= fDuration || c->LocalClock < 0.0f)
			{
				const float fLoops = floorf(c->LocalClock / fDuration);
				c->LocalClock -= fLoops * fDuration;
				c->LoopIndex += (int) fLoops;
				if (c->LocalClock >= fDuration)
				{
					c->LocalClock -= fDuration;
					++c->LoopIndex;
				}
				else if (c->LocalClock < 0.0f)
				{
					c->LocalClock += fDuration;
					--c->LoopIndex;
				}
			}
			return;
		}

		const int lastLoop = c->LoopCount - 1;
		if (c->LocalClock >= fDuration && c->LoopIndex < lastLoop)
		{
			int loops = (int) floorf(c->LocalClock / fDuration);
			if (loops > lastLoop - c->LoopIndex)
				loops = lastLoop - c->LoopIndex;
			c->LocalClock -= (float) loops * fDuration;
			c->LoopIndex += loops;
		}
		else if (c->LocalClock < 0.0f && c->LoopIndex > 0)
		{
			int loops = (int) ceilf(-c->LocalClock / fDuration);
			if (loops > c->LoopIndex)
				loops = c->LoopIndex;
			c->LocalClock += (float) loops * fDuration;
			c->LoopIndex -= loops;
		}
	}

	void SetControlClock(granny_control * c, float fClock)
	{
		c->LocalClock += (fClock - c->CurrentClock) * c->Speed;
		c->CurrentClock = fClock;
		NormalizeLoop(c);
	}

	float ClampedLocalClock(const granny_control * c)
	{
		if (c->LocalClock < 0.0f)
			return 0.0f;
		if (c->LocalClock > c->LocalDuration)
			return c->LocalDuration;
		return c->LocalClock;
	}

	void LoopState(const granny_control * c, bool & risUnderflow, bool & risOverflow)
	{
		const bool isInfinite = c->LoopCount <= 0;
		risUnderflow = isInfinite || c->LoopIndex > 0;
		risOverflow = isInfinite || c->LoopIndex < c->LoopCount - 1;
	}

	float Bezier(const float * p, float u)
	{
		const float v = 1.0f - u;
		return v * v * v * p[0] + 3.0f * u * v * v * p[1] + 3.0f * u * u * v * p[2] + u * u * u * p[3];
	}

	// Cynex: ease in bitince 1, ease out bitince son deger; ikisi aciksa ease in bitene kadar o kullanilir
	float EaseMultiplier(const granny_control * c)
	{
		const float t = c->CurrentClock;
		if (c->EaseIn && t <= c->EaseInEnd)
		{
			if (t < c->EaseInStart)
				return c->EaseInValues[0];
			if (c->EaseInEnd <= c->EaseInStart)
				return 1.0f;
			return Bezier(c->EaseInValues, (t - c->EaseInStart) / (c->EaseInEnd - c->EaseInStart));
		}
		if (c->EaseOut)
		{
			if (t < c->EaseOutStart)
				return 1.0f;
			if (t >= c->EaseOutEnd)
				return c->EaseOutValues[3];
			return Bezier(c->EaseOutValues, (t - c->EaseOutStart) / (c->EaseOutEnd - c->EaseOutStart));
		}
		return 1.0f;
	}

	float EffectiveWeight(const granny_control * c)
	{
		return c->Weight * EaseMultiplier(c);
	}

	void UnlinkControl(granny_control * c)
	{
		granny_model_instance * pInstance = c->Owner;
		if (!pInstance)
			return;
		if (c->Prev)
			c->Prev->Next = c->Next;
		else
			pInstance->FirstControl = c->Next;
		if (c->Next)
			c->Next->Prev = c->Prev;
		else
			pInstance->LastControl = c->Prev;
		c->Prev = c->Next = NULL;
		c->Owner = NULL;
	}

	void DestroyControl(granny_control * c)
	{
		UnlinkControl(c);
		ReleaseBinding(c->Binding);
		AlignedFree(c);
	}

	bool IsComplete(const granny_control * c)
	{
		return c->CurrentClock >= c->CompletionClock;
	}

	/////////////////////////////////////////////////////////////////////////
	// Cynex: ornekleme

	CYNEX_FORCEINLINE void NormalizeQuaternion(float * q)
	{
		const float fLengthSq = q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3];
		if (fLengthSq > 0.0f)
		{
			const float fScale = 1.0f / sqrtf(fLengthSq);
			q[0] *= fScale;
			q[1] *= fScale;
			q[2] *= fScale;
			q[3] *= fScale;
		}
	}

	void AccumulateSample(SLocalBone & rBone, const SBoundTrack & c_rTrack, float w, const float * p, const float * q, const float * ss)
	{
		// Cynex: yarim kure secimi egri degerlendirmede yapildi (rest'e gore pencere), burada dogrudan toplanir
		granny_transform & rT = rBone.kTransform;
		rT.Flags |= c_rTrack.uFlags;
		rT.Position[0] += w * p[0];
		rT.Position[1] += w * p[1];
		rT.Position[2] += w * p[2];
		rT.Orientation[0] += w * q[0];
		rT.Orientation[1] += w * q[1];
		rT.Orientation[2] += w * q[2];
		rT.Orientation[3] += w * q[3];
		float * pSS = &rT.ScaleShear[0][0];
		for (int k = 0; k < 9; ++k)
			pSS[k] += w * ss[k];
		rBone.fWeight += w;
		++rBone.iCount;
	}

	void SampleInstance(const granny_model_instance * c_pInstance, int iFirstBone, int iBoneCount, granny_local_pose * pPose)
	{
		const granny_skeleton * c_pSkeleton = c_pInstance->Model->Skeleton;
		BeginLocalPoseAccumulation(pPose, iFirstBone, iBoneCount);

		const int iEndBone = iFirstBone + iBoneCount;
		for (const granny_control * c = c_pInstance->FirstControl; c; c = c->Next)
		{
			const float w = EffectiveWeight(c);
			if (!(w > GrannyTrackWeightEpsilon) || !c->Binding)
				continue;

			const float t = ClampedLocalClock(c);
			const float fDuration = c->Animation->Duration;
			const float fTimeStep = c->Animation->TimeStep;
			bool isUnderflow, isOverflow;
			LoopState(c, isUnderflow, isOverflow);

			const SAnimationBinding * pBinding = c->Binding;
			for (int i = 0; i < pBinding->iTrackCount; ++i)
			{
				const SBoundTrack & rTrack = pBinding->pTracks[i];
				if (rTrack.iBone < iFirstBone || rTrack.iBone >= iEndBone)
					continue;

				float p[3], q[4], ss[9];
				EvaluateCurve(rTrack.kPosition, 3, isUnderflow, isOverflow, fDuration, fTimeStep, t, c_afIdentityPosition, p);
				EvaluateCurve(rTrack.kOrientation, 4, isUnderflow, isOverflow, fDuration, fTimeStep, t, c_afIdentityOrientation, q);
				NormalizeQuaternion(q);
				if (!rTrack.isDirectOrientation && q[0] * rTrack.afReference[0] + q[1] * rTrack.afReference[1] + q[2] * rTrack.afReference[2] + q[3] * rTrack.afReference[3] < 0.0f)
				{
					q[0] = -q[0];
					q[1] = -q[1];
					q[2] = -q[2];
					q[3] = -q[3];
				}
				EvaluateCurve(rTrack.kScaleShear, 9, isUnderflow, isOverflow, fDuration, fTimeStep, t, c_afIdentityScaleShear, ss);
				AccumulateSample(pPose->Bones[rTrack.iBone], rTrack, w, p, q, ss);
			}
		}

		EndLocalPoseAccumulation(pPose, iFirstBone, iBoneCount, c_pSkeleton);
	}

	int ClampBoneRange(const granny_model_instance * c_pInstance, int iFirstBone, int iBoneCount, int iPoseBones)
	{
		const int skeletonBones = c_pInstance->Model->Skeleton ? c_pInstance->Model->Skeleton->BoneCount : 0;
		int end = iFirstBone + iBoneCount;
		if (end > skeletonBones)
			end = skeletonBones;
		if (end > iPoseBones)
			end = iPoseBones;
		return end - iFirstBone;
	}

	/////////////////////////////////////////////////////////////////////////
	// Cynex: kok hareketi (sabit cikarim: LoopTranslation veya periyodik dongu)

	void RootMotionVectors(const granny_model_instance * c_pInstance, float fSecondsElapsed, float * pTranslation, float * pRotation, bool isInverse)
	{
		float afTranslation[3] = { 0.0f, 0.0f, 0.0f };
		float afRotation[3] = { 0.0f, 0.0f, 0.0f };
		float fTotalWeight = 0.0f;

		for (const granny_control * c = c_pInstance->FirstControl; c; c = c->Next)
		{
			if (!c->HasAccumulation)
				continue;
			const float w = EffectiveWeight(c);
			if (!(w > GrannyTrackWeightEpsilon))
				continue;

			const float fDuration = c->LocalDuration;
			// Cynex: aralik icinde bulunulan turun yerel saatinde; sadece ilk turda bastan, son turda sondan kirpilir (granny ile ayni)
			float fNow = c->LocalClock;
			float fThen = fNow - c->Speed * fSecondsElapsed;
			if (c->LoopCount > 0)
			{
				if (c->LoopIndex <= 0)
				{
					fNow = fNow < 0.0f ? 0.0f : fNow;
					fThen = fThen < 0.0f ? 0.0f : fThen;
				}
				if (c->LoopIndex >= c->LoopCount - 1)
				{
					fNow = fNow > fDuration ? fDuration : fNow;
					fThen = fThen > fDuration ? fDuration : fThen;
				}
			}
			const float dt = fNow - fThen;
			// Cynex: bu aralikta yerel saati ilerlemeyen kontrol (bitmis) ortalamaya girmez
			if (dt == 0.0f)
				continue;

			const granny_track_group * c_pGroup = c->TrackGroup;
			float t[3] = { 0.0f, 0.0f, 0.0f };
			float r[3] = { 0.0f, 0.0f, 0.0f };
			if (c_pGroup->PeriodicLoop)
			{
				const granny_periodic_loop & rLoop = *c_pGroup->PeriodicLoop;
				const float fAngle = rLoop.dAngle * dt;
				const float fCos = cosf(fAngle) - 1.0f;
				const float fSin = sinf(fAngle);
				for (int k = 0; k < 3; ++k)
				{
					t[k] = rLoop.Radius * fCos * rLoop.BasisX[k] + rLoop.Radius * fSin * rLoop.BasisY[k] + rLoop.dZ * dt * rLoop.Axis[k];
					r[k] = rLoop.Axis[k] * fAngle;
				}
			}
			else if (fDuration > 0.0f)
			{
				const float s = dt / fDuration;
				for (int k = 0; k < 3; ++k)
					t[k] = c_pGroup->LoopTranslation[k] * s;
			}

			for (int k = 0; k < 3; ++k)
			{
				afTranslation[k] += w * t[k];
				afRotation[k] += w * r[k];
			}
			fTotalWeight += w;
		}

		const float s = fTotalWeight > 0.0f ? (isInverse ? -1.0f : 1.0f) / fTotalWeight : 0.0f;
		for (int k = 0; k < 3; ++k)
		{
			pTranslation[k] = afTranslation[k] * s;
			pRotation[k] = afRotation[k] * s;
		}
	}

	// Cynex: sonuc = [R(rot) | tr] * model (satir vektor)
	void ApplyRootMotion(const float * c_pModel, const float * c_pTranslation, const float * c_pRotation, float * pDest)
	{
		float delta[16];
		memcpy(delta, c_afIdentity4x4, sizeof(delta));

		const float fAngle = sqrtf(c_pRotation[0] * c_pRotation[0] + c_pRotation[1] * c_pRotation[1] + c_pRotation[2] * c_pRotation[2]);
		if (fAngle > 0.0f)
		{
			const float x = c_pRotation[0] / fAngle;
			const float y = c_pRotation[1] / fAngle;
			const float z = c_pRotation[2] / fAngle;
			const float c = cosf(fAngle);
			const float s = sinf(fAngle);
			const float v = 1.0f - c;
			delta[0] = c + v * x * x;		delta[1] = v * x * y + s * z;	delta[2] = v * x * z - s * y;
			delta[4] = v * x * y - s * z;	delta[5] = c + v * y * y;		delta[6] = v * y * z + s * x;
			delta[8] = v * x * z + s * y;	delta[9] = v * y * z - s * x;	delta[10] = c + v * z * z;
		}
		delta[12] = c_pTranslation[0];
		delta[13] = c_pTranslation[1];
		delta[14] = c_pTranslation[2];

		float result[16];
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				result[i * 4 + j] = delta[i * 4 + 0] * c_pModel[0 * 4 + j] + delta[i * 4 + 1] * c_pModel[1 * 4 + j]
					+ delta[i * 4 + 2] * c_pModel[2 * 4 + j] + delta[i * 4 + 3] * c_pModel[3 * 4 + j];
		memcpy(pDest, result, sizeof(result));
	}
}

namespace CynexGranny
{
	// Cynex: dosya serbest birakilinca o bellekteki track group / iskeletlere ait baglamalar atilir
	void OnMemoryRangeFreed(const void * c_pvBegin, size_t size)
	{
		if (!c_pvBegin || !size)
			return;
		const uintptr_t begin = reinterpret_cast<uintptr_t>(c_pvBegin);
		const uintptr_t end = begin + size;

		std::lock_guard<std::mutex> kGuard(gs_kBindingLock);
		for (auto it = gs_kBindings.begin(); it != gs_kBindings.end();)
		{
			const uintptr_t group = reinterpret_cast<uintptr_t>(it->first.pGroup);
			const uintptr_t skeleton = reinterpret_cast<uintptr_t>(it->first.pSkeleton);
			if ((group >= begin && group < end) || (skeleton >= begin && skeleton < end))
			{
				SAnimationBinding * p = it->second;
				it = gs_kBindings.erase(it);
				p->isCached = false;
				if (p->iRefCount == 0)
				{
					UnusedUnlink(p);
					DestroyBinding(p);
				}
			}
			else
				++it;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Cynex: model instance

GRANNY_DYNLINK(granny_model_instance *) GrannyInstantiateModel(granny_model const * Model)
{
	if (!Model || !Model->Skeleton)
		return NULL;
	granny_model_instance * pInstance = static_cast<granny_model_instance *>(AlignedAlloc(sizeof(granny_model_instance), 16));
	pInstance->Model = Model;
	pInstance->FirstControl = NULL;
	pInstance->LastControl = NULL;
	return pInstance;
}

GRANNY_DYNLINK(void) GrannyFreeModelInstance(granny_model_instance * ModelInstance)
{
	if (!ModelInstance)
		return;
	while (ModelInstance->FirstControl)
		DestroyControl(ModelInstance->FirstControl);
	AlignedFree(ModelInstance);
}

GRANNY_DYNLINK(granny_skeleton *) GrannyGetSourceSkeleton(granny_model_instance const * Model)
{
	return Model ? Model->Model->Skeleton : NULL;
}

GRANNY_DYNLINK(void) GrannySetModelClock(granny_model_instance const * ModelInstance, granny_real32 NewClock)
{
	if (!ModelInstance)
		return;
	for (granny_control * c = ModelInstance->FirstControl; c; c = c->Next)
		SetControlClock(c, NewClock);
}

GRANNY_DYNLINK(void) GrannyFreeCompletedModelControls(granny_model_instance const * ModelInstance)
{
	if (!ModelInstance)
		return;
	granny_control * c = ModelInstance->FirstControl;
	while (c)
	{
		granny_control * pNext = c->Next;
		if (c->FreeOnceUnused && IsComplete(c))
			DestroyControl(c);
		c = pNext;
	}
}

GRANNY_DYNLINK(void) GrannySampleModelAnimations(granny_model_instance const * ModelInstance, granny_int32x FirstBone, granny_int32x BoneCount, granny_local_pose * Result)
{
	if (!ModelInstance || !Result || FirstBone < 0)
		return;
	BoneCount = ClampBoneRange(ModelInstance, FirstBone, BoneCount, Result->BoneCount);
	if (BoneCount <= 0)
		return;
	SampleInstance(ModelInstance, FirstBone, BoneCount, Result);
}

GRANNY_DYNLINK(void) GrannySampleModelAnimationsAccelerated(granny_model_instance const * ModelInstance, granny_int32x BoneCount, granny_real32 const * Offset4x4, granny_local_pose * Scratch, granny_world_pose * Result)
{
	if (!ModelInstance || !Scratch || !Result)
		return;
	BoneCount = ClampBoneRange(ModelInstance, 0, BoneCount, Scratch->BoneCount < Result->BoneCount ? Scratch->BoneCount : Result->BoneCount);
	if (BoneCount <= 0)
		return;
	SampleInstance(ModelInstance, 0, BoneCount, Scratch);
	BuildWorldPose(ModelInstance->Model->Skeleton, 0, BoneCount, Scratch, Offset4x4, Result);
}

GRANNY_DYNLINK(void) GrannyGetRootMotionVectors(granny_model_instance const * ModelInstance, granny_real32 SecondsElapsed, granny_real32 * ResultTranslation3, granny_real32 * ResultRotation3, bool Inverse)
{
	float afTranslation[3], afRotation[3];
	if (ModelInstance)
		RootMotionVectors(ModelInstance, SecondsElapsed, afTranslation, afRotation, Inverse);
	else
		afTranslation[0] = afTranslation[1] = afTranslation[2] = afRotation[0] = afRotation[1] = afRotation[2] = 0.0f;
	if (ResultTranslation3)
		memcpy(ResultTranslation3, afTranslation, sizeof(afTranslation));
	if (ResultRotation3)
		memcpy(ResultRotation3, afRotation, sizeof(afRotation));
}

GRANNY_DYNLINK(void) GrannyUpdateModelMatrix(granny_model_instance const * ModelInstance, granny_real32 SecondsElapsed, granny_real32 const * ModelMatrix4x4, granny_real32 * DestMatrix4x4, bool Inverse)
{
	if (!ModelMatrix4x4 || !DestMatrix4x4)
		return;
	float afTranslation[3] = { 0.0f, 0.0f, 0.0f };
	float afRotation[3] = { 0.0f, 0.0f, 0.0f };
	if (ModelInstance)
		RootMotionVectors(ModelInstance, SecondsElapsed, afTranslation, afRotation, Inverse);
	ApplyRootMotion(ModelMatrix4x4, afTranslation, afRotation, DestMatrix4x4);
}

/////////////////////////////////////////////////////////////////////////////
// Cynex: kontroller

GRANNY_DYNLINK(granny_control *) GrannyPlayControlledAnimation(granny_real32 StartTime, granny_animation const * Animation, granny_model_instance * Model)
{
	if (!Animation || !Model)
		return NULL;

	const granny_model * c_pModel = Model->Model;
	const granny_track_group * c_pGroup = NULL;
	for (int g = 0; g < Animation->TrackGroupCount; ++g)
	{
		const granny_track_group * p = Animation->TrackGroups[g];
		if (p && NamesEqualNoCase(p->Name, c_pModel->Name))
		{
			c_pGroup = p;
			break;
		}
	}
	if (!c_pGroup)
	{
		CYNEX_LOG(GrannyWarningLogMessage, GrannyControlLogMessage, "Unable to find matching track_group for Model: \"%s\" in Animation: \"%s\"",
			c_pModel->Name ? c_pModel->Name : "", Animation->Name ? Animation->Name : "");
		return NULL;
	}

	granny_control * c = static_cast<granny_control *>(AlignedAlloc(sizeof(granny_control), 16));
	memset(c, 0, sizeof(*c));
	c->Animation = Animation;
	c->TrackGroup = c_pGroup;
	c->Binding = AcquireBinding(c_pGroup, c_pModel->Skeleton);
	// Cynex: kok hareketi sadece iskeletin 0. kemigi animasyona bagliysa (granny ile ayni)
	c->HasAccumulation = (c_pGroup->Flags & GrannyAccumulationExtracted) && !(c_pGroup->Flags & GrannyAccumulationIsVDA)
		&& c->Binding->iTrackCount > 0 && c->Binding->pTracks[0].iBone == 0;
	c->CurrentClock = StartTime;
	c->LocalClock = 0.0f;
	c->LocalDuration = Animation->Duration;
	c->Speed = 1.0f;
	c->Weight = 1.0f;
	c->CompletionClock = FLT_MAX;
	c->LoopIndex = 0;
	c->LoopCount = (Animation->Flags & GrannyAnimationDefaultLoopCountValid) ? Animation->DefaultLoopCount : 1;
	c->EaseInValues[2] = c->EaseInValues[3] = 1.0f;
	c->EaseOutValues[0] = c->EaseOutValues[1] = 1.0f;

	c->Owner = Model;
	c->Prev = Model->LastControl;
	if (Model->LastControl)
		Model->LastControl->Next = c;
	else
		Model->FirstControl = c;
	Model->LastControl = c;
	return c;
}

GRANNY_DYNLINK(void) GrannyFreeControl(granny_control * Control)
{
	if (Control)
		DestroyControl(Control);
}

GRANNY_DYNLINK(void) GrannyFreeControlOnceUnused(granny_control * Control)
{
	if (Control)
		Control->FreeOnceUnused = true;
}

GRANNY_DYNLINK(void) GrannyCompleteControlAt(granny_control * Control, granny_real32 AtSeconds)
{
	if (Control)
		Control->CompletionClock = AtSeconds;
}

GRANNY_DYNLINK(bool) GrannyControlIsComplete(granny_control const * Control)
{
	return Control ? IsComplete(Control) : false;
}

GRANNY_DYNLINK(bool) GrannyFreeControlIfComplete(granny_control * Control)
{
	if (!Control || !IsComplete(Control))
		return false;
	DestroyControl(Control);
	return true;
}

GRANNY_DYNLINK(granny_int32x) GrannyGetControlLoopCount(granny_control const * Control)
{
	return Control ? Control->LoopCount : 0;
}

GRANNY_DYNLINK(void) GrannySetControlLoopCount(granny_control * Control, granny_int32x LoopCount)
{
	if (Control)
		Control->LoopCount = LoopCount;
}

GRANNY_DYNLINK(granny_real32) GrannyGetControlSpeed(granny_control const * Control)
{
	return Control ? Control->Speed : 0.0f;
}

GRANNY_DYNLINK(void) GrannySetControlSpeed(granny_control * Control, granny_real32 Speed)
{
	if (Control)
		Control->Speed = Speed;
}

GRANNY_DYNLINK(granny_real32) GrannyGetControlLocalDuration(granny_control const * Control)
{
	return Control ? Control->LocalDuration : 0.0f;
}

GRANNY_DYNLINK(granny_real32) GrannyGetControlRawLocalClock(granny_control * Control)
{
	return Control ? Control->LocalClock : 0.0f;
}

GRANNY_DYNLINK(void) GrannySetControlRawLocalClock(granny_control * Control, granny_real32 LocalClock)
{
	if (Control)
		Control->LocalClock = LocalClock;
}

GRANNY_DYNLINK(granny_real32) GrannyGetControlDurationLeft(granny_control * Control)
{
	if (!Control)
		return 0.0f;
	if (Control->LoopCount <= 0 || Control->Speed == 0.0f)
		return FLT_MAX;
	const float fElapsed = (float) Control->LoopIndex * Control->LocalDuration + Control->LocalClock;
	if (Control->Speed > 0.0f)
		return ((float) Control->LoopCount * Control->LocalDuration - fElapsed) / Control->Speed;
	return fElapsed / -Control->Speed;
}

GRANNY_DYNLINK(void) GrannySetControlEaseIn(granny_control * Control, bool EaseIn)
{
	if (Control)
		Control->EaseIn = EaseIn;
}

GRANNY_DYNLINK(void) GrannySetControlEaseOut(granny_control * Control, bool EaseOut)
{
	if (Control)
		Control->EaseOut = EaseOut;
}

GRANNY_DYNLINK(void) GrannySetControlEaseInCurve(granny_control * Control, granny_real32 StartSeconds, granny_real32 EndSeconds, granny_real32 StartValue, granny_real32 StartTangent, granny_real32 EndTangent, granny_real32 EndValue)
{
	if (!Control)
		return;
	Control->EaseInStart = StartSeconds;
	Control->EaseInEnd = EndSeconds;
	Control->EaseInValues[0] = StartValue;
	Control->EaseInValues[1] = StartTangent;
	Control->EaseInValues[2] = EndTangent;
	Control->EaseInValues[3] = EndValue;
}

GRANNY_DYNLINK(void) GrannySetControlEaseOutCurve(granny_control * Control, granny_real32 StartSeconds, granny_real32 EndSeconds, granny_real32 StartValue, granny_real32 StartTangent, granny_real32 EndTangent, granny_real32 EndValue)
{
	if (!Control)
		return;
	Control->EaseOutStart = StartSeconds;
	Control->EaseOutEnd = EndSeconds;
	Control->EaseOutValues[0] = StartValue;
	Control->EaseOutValues[1] = StartTangent;
	Control->EaseOutValues[2] = EndTangent;
	Control->EaseOutValues[3] = EndValue;
}

GRANNY_DYNLINK(granny_real32) GrannyEaseControlIn(granny_control * Control, granny_real32 Duration, bool FromCurrent)
{
	if (!Control)
		return 0.0f;
	const float fStart = Control->CurrentClock;
	const float fValue = FromCurrent ? EaseMultiplier(Control) : 0.0f;
	GrannySetControlEaseInCurve(Control, fStart, fStart + Duration, fValue, fValue, 1.0f, 1.0f);
	Control->EaseIn = true;
	return fStart + Duration;
}

GRANNY_DYNLINK(granny_real32) GrannyEaseControlOut(granny_control * Control, granny_real32 Duration)
{
	if (!Control)
		return 0.0f;
	const float fStart = Control->CurrentClock;
	GrannySetControlEaseOutCurve(Control, fStart, fStart + Duration, 1.0f, 1.0f, 0.0f, 0.0f);
	Control->EaseOut = true;
	return fStart + Duration;
}
