// Cynex: iskelet, local pose ve world pose
#include "granny_pose.h"

#include <cmath>

using namespace CynexGranny;

namespace CynexGranny
{
	void BeginLocalPoseAccumulation(granny_local_pose * pPose, int iFirstBone, int iBoneCount)
	{
		for (int b = iFirstBone; b < iFirstBone + iBoneCount; ++b)
		{
			SLocalBone & rBone = pPose->Bones[b];
			memset(&rBone.kTransform, 0, sizeof(rBone.kTransform));
			rBone.fWeight = 0.0f;
			rBone.iCount = 0;
		}
	}

	// Cynex: agirlik dolum esiginin altindaysa kalan kisim rest pose ile doldurulur
	void EndLocalPoseAccumulation(granny_local_pose * pPose, int iFirstBone, int iBoneCount, const granny_skeleton * c_pSkeleton)
	{
		const float fFill = pPose->FillThreshold;
		for (int b = iFirstBone; b < iFirstBone + iBoneCount; ++b)
		{
			SLocalBone & rBone = pPose->Bones[b];
			granny_transform & rT = rBone.kTransform;
			const granny_transform & rRest = c_pSkeleton->Bones[b].LocalTransform;

			if (rBone.fWeight < fFill)
			{
				const float w = fFill - rBone.fWeight;
				const float fDot = rT.Orientation[0] * rRest.Orientation[0] + rT.Orientation[1] * rRest.Orientation[1]
					+ rT.Orientation[2] * rRest.Orientation[2] + rT.Orientation[3] * rRest.Orientation[3];
				const float wq = fDot < 0.0f ? -w : w;
				rT.Flags |= rRest.Flags;
				for (int k = 0; k < 3; ++k)
					rT.Position[k] += w * rRest.Position[k];
				for (int k = 0; k < 4; ++k)
					rT.Orientation[k] += wq * rRest.Orientation[k];
				for (int i = 0; i < 3; ++i)
					for (int j = 0; j < 3; ++j)
						rT.ScaleShear[i][j] += w * rRest.ScaleShear[i][j];
				rBone.fWeight = fFill;
			}

			if (rBone.fWeight <= 0.0f)
			{
				rT = rRest;
				continue;
			}

			const float fInvWeight = 1.0f / rBone.fWeight;
			for (int k = 0; k < 3; ++k)
				rT.Position[k] *= fInvWeight;
			for (int i = 0; i < 3; ++i)
				for (int j = 0; j < 3; ++j)
					rT.ScaleShear[i][j] *= fInvWeight;

			const float fLengthSq = rT.Orientation[0] * rT.Orientation[0] + rT.Orientation[1] * rT.Orientation[1]
				+ rT.Orientation[2] * rT.Orientation[2] + rT.Orientation[3] * rT.Orientation[3];
			if (fLengthSq > 0.0f)
			{
				const float fScale = 1.0f / sqrtf(fLengthSq);
				for (int k = 0; k < 4; ++k)
					rT.Orientation[k] *= fScale;
			}
			else
			{
				rT.Orientation[0] = rT.Orientation[1] = rT.Orientation[2] = 0.0f;
				rT.Orientation[3] = 1.0f;
			}
		}
	}

	void BuildWorldPose(const granny_skeleton * c_pSkeleton, int iFirstBone, int iBoneCount, const granny_local_pose * c_pPose, const float * c_pOffset4x4, granny_world_pose * pResult)
	{
		const float * pRoot = c_pOffset4x4 ? c_pOffset4x4 : c_afIdentity4x4;
		for (int b = iFirstBone; b < iFirstBone + iBoneCount; ++b)
		{
			const granny_bone & rBone = c_pSkeleton->Bones[b];
			float local[16];
			BuildCompositeMatrix(c_pPose->Bones[b].kTransform, local);

			const float * pParent = (rBone.ParentIndex >= 0 && rBone.ParentIndex < b) ? &pResult->World[rBone.ParentIndex][0][0] : pRoot;
			float * pWorld = &pResult->World[b][0][0];
			MultiplyAffine4x4(pWorld, local, pParent);
			if (pResult->Composite)
				MultiplyAffine4x4(&pResult->Composite[b][0][0], &rBone.InverseWorld4x4[0][0], pWorld);
		}
	}
}

GRANNY_DYNLINK(bool) GrannyFindBoneByName(granny_skeleton const * Skeleton, char const * BoneName, granny_int32x * BoneIndex)
{
	if (!Skeleton || !BoneName)
		return false;

	int b = 0;
	for (; b < Skeleton->BoneCount; ++b)
	{
		const char * c_szName = Skeleton->Bones[b].Name;
		if (c_szName && strcmp(c_szName, BoneName) == 0)
		{
			if (BoneIndex)
				*BoneIndex = b;
			return true;
		}
	}
	if (BoneIndex)
		*BoneIndex = b;
	return false;
}

GRANNY_DYNLINK(granny_local_pose *) GrannyNewLocalPose(granny_int32x BoneCount)
{
	if (BoneCount < 0)
		BoneCount = 0;
	granny_local_pose * pPose = static_cast<granny_local_pose *>(AlignedAlloc(sizeof(granny_local_pose), 16));
	pPose->BoneCount = BoneCount;
	pPose->FillThreshold = GrannyDefaultLocalPoseFillThreshold;
	pPose->Bones = AllocArray<SLocalBone>((size_t) BoneCount);
	for (int b = 0; b < BoneCount; ++b)
	{
		SLocalBone & rBone = pPose->Bones[b];
		memset(&rBone, 0, sizeof(rBone));
		rBone.kTransform.Orientation[3] = 1.0f;
		rBone.kTransform.ScaleShear[0][0] = rBone.kTransform.ScaleShear[1][1] = rBone.kTransform.ScaleShear[2][2] = 1.0f;
	}
	return pPose;
}

GRANNY_DYNLINK(void) GrannyFreeLocalPose(granny_local_pose * LocalPose)
{
	if (!LocalPose)
		return;
	AlignedFree(LocalPose->Bones);
	AlignedFree(LocalPose);
}

GRANNY_DYNLINK(granny_world_pose *) GrannyNewWorldPose(granny_int32x BoneCount)
{
	if (BoneCount < 0)
		BoneCount = 0;
	granny_world_pose * pPose = static_cast<granny_world_pose *>(AlignedAlloc(sizeof(granny_world_pose), 16));
	pPose->BoneCount = BoneCount;
	pPose->World = AllocArray<granny_matrix_4x4>((size_t) BoneCount);
	pPose->Composite = AllocArray<granny_matrix_4x4>((size_t) BoneCount);
	for (int b = 0; b < BoneCount; ++b)
	{
		memcpy(pPose->World[b], c_afIdentity4x4, sizeof(granny_matrix_4x4));
		memcpy(pPose->Composite[b], c_afIdentity4x4, sizeof(granny_matrix_4x4));
	}
	return pPose;
}

GRANNY_DYNLINK(void) GrannyFreeWorldPose(granny_world_pose * WorldPose)
{
	if (!WorldPose)
		return;
	AlignedFree(WorldPose->World);
	AlignedFree(WorldPose->Composite);
	AlignedFree(WorldPose);
}

// Cynex: granny de BoneCount disini NULL donduruyor (EterGrnLib buna bakiyor)
GRANNY_DYNLINK(granny_real32 *) GrannyGetWorldPose4x4(granny_world_pose const * WorldPose, granny_int32x BoneIndex)
{
	if (!WorldPose || BoneIndex < 0 || BoneIndex >= WorldPose->BoneCount)
		return NULL;
	return &WorldPose->World[BoneIndex][0][0];
}

GRANNY_DYNLINK(granny_real32 *) GrannyGetWorldPoseComposite4x4(granny_world_pose const * WorldPose, granny_int32x BoneIndex)
{
	if (!WorldPose || !WorldPose->Composite || BoneIndex < 0 || BoneIndex >= WorldPose->BoneCount)
		return NULL;
	return &WorldPose->Composite[BoneIndex][0][0];
}

GRANNY_DYNLINK(granny_matrix_4x4 *) GrannyGetWorldPoseComposite4x4Array(granny_world_pose const * WorldPose)
{
	return WorldPose ? WorldPose->Composite : NULL;
}

GRANNY_DYNLINK(void) GrannyBuildWorldPose(granny_skeleton const * Skeleton, granny_int32x FirstBone, granny_int32x BoneCount, granny_local_pose const * LocalPose, granny_real32 const * Offset4x4, granny_world_pose * Result)
{
	if (!Skeleton || !LocalPose || !Result || FirstBone < 0 || BoneCount <= 0)
		return;
	if (FirstBone + BoneCount > Skeleton->BoneCount)
		BoneCount = Skeleton->BoneCount - FirstBone;
	if (FirstBone + BoneCount > LocalPose->BoneCount)
		BoneCount = LocalPose->BoneCount - FirstBone;
	if (FirstBone + BoneCount > Result->BoneCount)
		BoneCount = Result->BoneCount - FirstBone;
	if (BoneCount <= 0)
		return;
	BuildWorldPose(Skeleton, FirstBone, BoneCount, LocalPose, Offset4x4, Result);
}
