#ifndef __FBX_HELPER_H__
#define __FBX_HELPER_H__

#include "FBXCommon.h"
#include "inc.h"

namespace FBXHelper
{
	struct FbxMeshVertex_Tmp
	{
		D3DXVECTOR3 pos;
		D3DXVECTOR3 normal;
		unsigned int color;
		D3DXVECTOR2 uv;
	};

	struct FbxModel
	{
		FbxMeshVertex_Tmp* pVB = NULL;
		int nVertexCount = 0;
		unsigned int* pIB = NULL;
		int nIndexCount = 0;
	};

	struct FbxBoneWeight
	{
		List<std::string> boneName;
		List<double> weight;
	};

	struct FbxSkinInfo
	{
		FbxBoneWeight* weights = NULL;
		unsigned int size = 0;
	};

	class FbxModelList
	{
	public:
		FbxModelList();
		~FbxModelList();
	public:
		List<FbxModel*> mMeshes;
		List<FbxSkinInfo*> mSkins;
	};

	struct FbxBone
	{
		unsigned int id;
		std::string name;
		D3DXMATRIX bindPose;
		D3DXMATRIX offset;
		int layer = 0;
		FbxNode* node = NULL;
		FbxBone* parent = NULL;
		std::vector<FbxBone*> children;
	};

	class FbxBoneMap
	{
	public:
		FbxBoneMap();
		~FbxBoneMap();
	public:
		typedef std::map<std::string, FbxBone*>::iterator IT_BM;
		std::map<std::string, FbxBone*> mBones;
		List<FbxBone*> mBoneList;
	};

	class FbxAnimationEvaluator
	{
	public:
		FbxAnimationEvaluator();
		~FbxAnimationEvaluator();

		D3DXMATRIX Evaluator(FbxBone* bone, float second);
	};

	bool BeginFBXHelper(const char* fileName);

	FbxBoneMap* GetBoneMap();

	FbxModelList* GetModelList();

	void GetBox(D3DXVECTOR3& max, D3DXVECTOR3& min);

	D3DXMATRIX ToD3DMatrix(const FbxAMatrix& mat);

	bool SetCurrentAnimation(const char* animName);

	bool GetAniamtionNames(List<const char*>& names);

	void UpdateSkeleton();

	bool IsWorking();

	FbxManager* GetFbxManager();

	FbxScene* GetFbxScene();

	bool EndFBXHelper();
}

#endif
