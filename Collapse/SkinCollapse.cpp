#include "SkinCollapse.h"
#include "FBXHelper.h"

namespace SkinCollapse
{
	float ComputeSkinCost(char* vtx1, char* vtx2)
	{
		float cost = 1.0f;
		FBXHelper::FbxBoneWeight** bw1 = (FBXHelper::FbxBoneWeight**)(vtx1 + 36);
		FBXHelper::FbxBoneWeight** bw2 = (FBXHelper::FbxBoneWeight**)(vtx2 + 36);
		if (bw1 == NULL || bw2 == NULL)
			return cost;
		FBXHelper::FbxBoneWeight* pbw1 = *bw1;
		FBXHelper::FbxBoneWeight* pbw2 = *bw2;
		for (int i = 0; i < pbw1->boneName.Count(); ++i)
		{
			for (int j = 0; j < pbw2->boneName.Count(); ++j)
			{
				if (pbw1->boneName[i] == pbw2->boneName[j])
				{
					float weight1 = (float)pbw1->weight[i];
					float weight2 = (float)pbw2->weight[j];
					float w = (weight1 + weight2) * 0.25f;
					cost *= (1.0f - w);
				}
			}
		}
		return cost;
	}
}