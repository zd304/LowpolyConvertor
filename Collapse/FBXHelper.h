#ifndef __FBX_HELPER_H__
#define __FBX_HELPER_H__

#include "FBXCommon.h"
#include <d3dx9.h>
#include "list.h"
#include <string>
#include <map>
#include <vector>

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
		FbxMesh* meshNode = NULL;
		int layer = 0;
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

		D3DXMATRIX Evaluator(FbxBone* bone, const char* animName, float second);
	public:
		void* mCurveData;
		static D3DXMATRIX matIdentity;
	};

	bool BeginFBXHelper(const char* fileName);

	void GetMesh(void** ppVB, int& v_stride, int& v_count, void** ppIB, int& i_stride, int& i_count);

	FbxAnimationEvaluator* GetAnimationEvaluator();

	FbxBoneMap* GetBoneMap();

	FbxModelList* GetModelList();
	D3DXMATRIX ToD3DMatrix(const FbxAMatrix& mat);

	bool EndFBXHelper();
}

#endif
