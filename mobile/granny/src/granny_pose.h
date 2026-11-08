// Cynex: local/world pose ic yapilari
#pragma once

#include "granny_internal.h"

namespace CynexGranny
{
	struct SLocalBone
	{
		granny_transform	kTransform;
		float				fWeight;
		int					iCount;
	};
}

namespace granny
{
	struct local_pose
	{
		int							BoneCount;
		float						FillThreshold;
		CynexGranny::SLocalBone *	Bones;
	};

	struct world_pose
	{
		int					BoneCount;
		granny_matrix_4x4 *	World;
		granny_matrix_4x4 *	Composite;
	};
}

namespace CynexGranny
{
	void BeginLocalPoseAccumulation(granny_local_pose * pPose, int iFirstBone, int iBoneCount);
	void EndLocalPoseAccumulation(granny_local_pose * pPose, int iFirstBone, int iBoneCount, const granny_skeleton * c_pSkeleton);
	void BuildWorldPose(const granny_skeleton * c_pSkeleton, int iFirstBone, int iBoneCount, const granny_local_pose * c_pPose, const float * c_pOffset4x4, granny_world_pose * pResult);
}
