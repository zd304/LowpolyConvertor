#include "FBXHelper.h"
#include <string>

namespace FBXHelper
{
	FbxManager* pFBXSDKManager = NULL;
	FbxScene* pFBXScene = NULL;
	FbxModelList* pMeshList = NULL;
	FbxBoneMap* pSkeleton = NULL;
	FbxAnimationEvaluator* pAnimEvaluator = NULL;
	DWORD mLastTime = 0l;
	float mAnimTime = 0.0f;
	float mDuration = 0.0f;

	FbxModelList::FbxModelList()
	{
		mMeshes.Clear();
		mSkins.Clear();
	}

	FbxModelList::~FbxModelList()
	{
		for (int i = 0; i < mMeshes.Count(); ++i)
		{
			FbxModel* mesh = mMeshes[i];
			if (!mesh) continue;
			if (mesh->pVB)
			{
				delete[] mesh->pVB;
				mesh->pVB = NULL;
			}
			if (mesh->pIB)
			{
				delete[] mesh->pIB;
				mesh->pIB = NULL;
			}
			mesh->nIndexCount = 0;
			mesh->nVertexCount = 0;
			delete mesh;
		}
		mMeshes.Clear();
		for (int i = 0; i < mSkins.Count(); ++i)
		{
			FbxSkinInfo* skin = mSkins[i];
			if (!skin) continue;
			if (skin->weights)
			{
				delete[] skin->weights;
			}
			delete skin;
		}
		mSkins.Clear();
	}

	FbxBoneMap::FbxBoneMap()
	{

	}

	FbxBoneMap::~FbxBoneMap()
	{
		std::map<std::string, FbxBone*>::iterator it;
		for (it = mBones.begin(); it != mBones.end(); ++it)
		{
			FbxBone* bone = it->second;
			if (!bone)
				continue;
			delete bone;
		}
		mBones.clear();
		mBoneList.Clear();
	}

	class AnimationCurve;
	struct LocalCurve;

	class BoneAniamtion
	{
	public:
		BoneAniamtion();
		~BoneAniamtion();
	public:
		typedef std::map<FbxBone*, AnimationCurve*>::iterator IT_BA;
		std::map<FbxBone*, AnimationCurve*> mBoneCurves;
	};

	class AnimationCurve
	{
	public:
		AnimationCurve();
		~AnimationCurve();
	public:
		typedef std::map<std::string, LocalCurve*>::iterator IT_AC;
		std::map<std::string, LocalCurve*> animCurves;
	};

	struct LocalCurve
	{
		FbxAnimCurve* translationX = NULL;
		FbxAnimCurve* translationY = NULL;
		FbxAnimCurve* translationZ = NULL;
		FbxAnimCurve* rotationX = NULL;
		FbxAnimCurve* rotationY = NULL;
		FbxAnimCurve* rotationZ = NULL;
		FbxAnimCurve* scaleX = NULL;
		FbxAnimCurve* scaleY = NULL;
		FbxAnimCurve* scaleZ = NULL;
		FbxNode* node = NULL;
	};

	D3DXMATRIX ToD3DMatrix(const FbxAMatrix& mat)
	{
		D3DXMATRIX mm;
			//= D3DXMATRIX(
			//(float)mat.Get(0, 0), (float)mat.Get(0, 1), (float)mat.Get(0, 2), (float)mat.Get(0, 3),
			//(float)mat.Get(1, 0), (float)mat.Get(1, 1), (float)mat.Get(1, 2), (float)mat.Get(1, 3),
			//(float)mat.Get(2, 0), (float)mat.Get(2, 1), (float)mat.Get(2, 2), (float)mat.Get(2, 3),
			//(float)mat.Get(3, 0), (float)mat.Get(3, 1), (float)mat.Get(3, 2), (float)mat.Get(3, 3));
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				mm(i, j) = (float)mat.Get(i, j);
			}
		}
		return mm;
	}

	FbxAMatrix ToFbxMatrix(const D3DXMATRIX& mat)
	{
		FbxAMatrix mm;
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				mm.mData[i][j] = (float)mat(i, j);
			}
		}
		return mm;
	}

	D3DXMATRIX FbxAnimationEvaluator::matIdentity = D3DXMATRIX(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);

	FbxAnimationEvaluator::FbxAnimationEvaluator()
	{
		mCurveData = new BoneAniamtion();
	}

	FbxAnimationEvaluator::~FbxAnimationEvaluator()
	{
		if (mCurveData)
		{
			BoneAniamtion* p = (BoneAniamtion*)mCurveData;
			delete p;
			mCurveData = NULL;
		}
	}

	D3DXMATRIX FbxAnimationEvaluator::Evaluator(FbxBone* bone, float second)
	{
		FbxTime t;
		t.SetSecondDouble((double)second);
		FbxAMatrix mat = bone->node->EvaluateGlobalTransform(t);
		return ToD3DMatrix(mat);
	}

	BoneAniamtion::BoneAniamtion()
	{
		mBoneCurves.clear();
	}

	BoneAniamtion::~BoneAniamtion()
	{
		std::map<FbxBone*, AnimationCurve*>::iterator it;
		for (it = mBoneCurves.begin(); it != mBoneCurves.end(); ++it)
		{
			AnimationCurve* anim = it->second;
			if (anim)
			{
				delete anim;
			}
		}
		mBoneCurves.clear();
	}

	AnimationCurve::AnimationCurve()
	{
		animCurves.clear();
	}

	AnimationCurve::~AnimationCurve()
	{
		std::map<std::string, LocalCurve*>::iterator it;
		for (it = animCurves.begin(); it != animCurves.end(); ++it)
		{
			LocalCurve* curves = it->second;
			if (curves)
			{
				delete curves;
			}
		}
		animCurves.clear();
	}

	void ProcessNode(FbxNode* pNode, FbxNode* pParent = NULL, int mask = -1);

	bool BeginFBXHelper(const char* fileName)
	{
		InitializeSdkObjects(pFBXSDKManager, pFBXScene);

		bool rst = LoadScene(pFBXSDKManager, pFBXScene, fileName);

		if (rst == false)
		{
			FBXSDK_printf("\n\nAn error occurred while loading the scene...");
			return false;
		}

		FbxAxisSystem::DirectX.ConvertScene(pFBXScene);
		FbxAxisSystem fbxAxisSystem = pFBXScene->GetGlobalSettings().GetAxisSystem();
		FbxAxisSystem::ECoordSystem coordSystem = fbxAxisSystem.GetCoorSystem();
		if (coordSystem == FbxAxisSystem::ECoordSystem::eLeftHanded)
		{
			printf("Left-Hand!\n");
		}
		else
		{
			printf("Right-Hand!\n");
		}

		FbxGeometryConverter converter(pFBXSDKManager);
		converter.Triangulate(pFBXScene, true);

		pMeshList = new FbxModelList();

		int numStacks = pFBXScene->GetSrcObjectCount<fbxsdk_2015_1::FbxAnimStack>();
		if (numStacks > 0)
		{
			pAnimEvaluator = new FbxAnimationEvaluator();
		}

		mLastTime = timeGetTime();
		mAnimTime = 0.0f;
		mDuration = 0.0f;

		ProcessNode(pFBXScene->GetRootNode(), NULL, FbxNodeAttribute::eSkeleton);
		ProcessNode(pFBXScene->GetRootNode(), NULL, FbxNodeAttribute::eMesh);

		return rst;
	}

	FbxSkinInfo* ProcessSkin(FbxMesh* pMesh, int vtxCount)
	{
		int skinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);
		if (skinCount == 0)
			return NULL;
		FbxSkinInfo* skinInfo = new FbxSkinInfo();
		pMeshList->mSkins.Add(skinInfo);
		// スキンの数を取得;
		skinInfo->weights = new FbxBoneWeight[vtxCount];
		skinInfo->size = vtxCount;
		for (int s = 0; s < skinCount; ++s)
		{
			// s番目のスキンを取得;
			FbxSkin* skinDeformer = (FbxSkin*)pMesh->GetDeformer(s, FbxDeformer::eSkin);
			int cpic = skinDeformer->GetControlPointIndicesCount();

			int clusterCount = skinDeformer->GetClusterCount();
			for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
			{
				FbxCluster* cluster = skinDeformer->GetCluster(clusterIndex);
				FbxNode* link = cluster->GetLink();
				if (!link)
					continue;
				const char* boneName = link->GetName();

				FbxBoneMap::IT_BM itbm = pSkeleton->mBones.find(boneName);
				if (itbm != pSkeleton->mBones.end())
				{
					FbxBone* bone = itbm->second;
					FbxAMatrix transformLinkMatrix, transformMatrix, matBindPose;

					std::string txt;
					for (int ll = 0; ll < bone->layer; ++ll)
					{
						txt += "  ";
					}
					txt += bone->name;
					txt += "\n";
					printf(txt.c_str());

					// ボーンの初期姿勢を取得;
					cluster->GetTransformLinkMatrix(transformLinkMatrix);

					cluster->GetTransformMatrix(transformMatrix);
					FbxVector4 vt = pMesh->GetNode()->GetGeometricTranslation(FbxNode::eSourcePivot);
					FbxVector4 vr = pMesh->GetNode()->GetGeometricRotation(FbxNode::eSourcePivot);
					FbxVector4 vs = pMesh->GetNode()->GetGeometricScaling(FbxNode::eSourcePivot);
					FbxAMatrix geometry(vt, vr, vs);
					transformMatrix *= geometry;

					matBindPose = transformMatrix.Inverse() * transformLinkMatrix;
					bone->bindPose = ToD3DMatrix(matBindPose);
				}
				else
				{
					printf("Not Find %s\n", boneName);
				}

				int* indices = cluster->GetControlPointIndices();
				double* weights = cluster->GetControlPointWeights();
				int indexCount = cluster->GetControlPointIndicesCount();
				for (int pi = 0; pi < indexCount; ++pi)
				{
					int vtxIndex = indices[pi];
					double weight = weights[pi];
					FbxBoneWeight& bw = skinInfo->weights[vtxIndex];
					bw.boneName.Add(boneName);
					bw.weight.Add(weight);
				}
			}
		}
		return skinInfo;
	}

	void ProcessAnimation(FbxNode* pNode, FbxBone* bone)
	{
		if (!pAnimEvaluator)
			return;
		BoneAniamtion* boneAnim = (BoneAniamtion*)pAnimEvaluator->mCurveData;
		AnimationCurve* animCurve = NULL;
		BoneAniamtion::IT_BA itba = boneAnim->mBoneCurves.find(bone);
		if (itba == boneAnim->mBoneCurves.end())
		{
			animCurve = new AnimationCurve();
			boneAnim->mBoneCurves[bone] = animCurve;
		}
		else
		{
			animCurve = boneAnim->mBoneCurves[bone];
		}
		if (animCurve == NULL)
			return;

		int numStacks = pFBXScene->GetSrcObjectCount<fbxsdk_2015_1::FbxAnimStack>();
		for (int i = 0; i < numStacks; ++i)
		{
			fbxsdk_2015_1::FbxAnimStack* pAnimStack = pFBXScene->GetSrcObject<fbxsdk_2015_1::FbxAnimStack>(i);
			std::string animName = pAnimStack->GetName();

			int numAnimLayers = pAnimStack->GetMemberCount<FbxAnimLayer>();
			//for (int j = 0; j < numAnimLayers; ++j)
			//{
			//	FbxAnimLayer* pLayer = pAnimStack->GetMember<FbxAnimLayer>(j);
			//}
			// 暂不考虑Blend;
			if (numAnimLayers > 0)
			{
				FbxAnimLayer* pLayer = pAnimStack->GetMember<FbxAnimLayer>(0);
				if (!pLayer)
					return;
				LocalCurve* curves = new LocalCurve();
				curves->translationX = pNode->LclTranslation.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_X);
				curves->translationY = pNode->LclTranslation.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_Y);
				curves->translationZ = pNode->LclTranslation.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_Z);
				curves->rotationX = pNode->LclRotation.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_X);
				curves->rotationY = pNode->LclRotation.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_Y);
				curves->rotationZ = pNode->LclRotation.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_Z);
				curves->scaleX = pNode->LclScaling.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_X);
				curves->scaleY = pNode->LclScaling.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_Y);
				curves->scaleZ = pNode->LclScaling.GetCurve(pLayer, FBXSDK_CURVENODE_COMPONENT_Z);
				curves->node = pNode;
				animCurve->animCurves[animName] = curves;
			}
		}
	}

	void ProcessSkeleton(FbxNode* pNode, FbxNode* pParent)
	{
		FbxNode* parent = NULL;
		if (pParent)
		{
			FbxNodeAttribute* attributeType = pParent->GetNodeAttribute();
			if (attributeType && attributeType->GetAttributeType() == FbxNodeAttribute::eSkeleton)
			{
				parent = pParent;
			}
		}
		if (!pSkeleton)
			pSkeleton = new FbxBoneMap();

		FbxBone* bone = new FbxBone();
		bone->id = pSkeleton->mBones.size();
		bone->name = pNode->GetName();
		bone->layer = 0;
		bone->node = pNode;

		if (parent)
		{
			FbxBone* parentBone = pSkeleton->mBones[parent->GetName()];
			bone->parent = parentBone;
			bone->layer = parentBone->layer + 1;
			parentBone->children.push_back(bone);
		}

		D3DXMatrixIdentity(&bone->bindPose);
		pSkeleton->mBones[bone->name] = bone;
		pSkeleton->mBoneList.Add(bone);

		ProcessAnimation(pNode, bone);
	}

	void ProcessMesh(FbxNode* pNode)
	{
		FbxMesh* pMesh = pNode->GetMesh();
		if (pMesh == NULL)
		{
			return;
		}
		FbxModel* meshData = new FbxModel();

		bool allByControlPoint = true;

		int triangleCount = pMesh->GetPolygonCount();
		bool hasNormal = pMesh->GetElementNormalCount() > 0;
		bool hasUV = pMesh->GetElementUVCount() > 0;
		FbxGeometryElement::EMappingMode normalMappingMode = FbxGeometryElement::eNone;
		FbxGeometryElement::EMappingMode uvMappingMode = FbxGeometryElement::eNone;
		if (hasNormal)
		{
			normalMappingMode = pMesh->GetElementNormal(0)->GetMappingMode();
			if (normalMappingMode == FbxGeometryElement::eNone)
			{
				hasNormal = false;
			}
			if (hasNormal && normalMappingMode != FbxGeometryElement::eByControlPoint)
			{
				allByControlPoint = false;
			}
		}
		if (hasUV)
		{
			uvMappingMode = pMesh->GetElementUV(0)->GetMappingMode();
			if (uvMappingMode == FbxGeometryElement::eNone)
			{
				hasUV = false;
			}
			if (hasUV && uvMappingMode != FbxGeometryElement::eByControlPoint)
			{
				allByControlPoint = false;
			}
		}

		meshData->nVertexCount = pMesh->GetControlPointsCount();
		if (!allByControlPoint)
		{
			//meshData->nVertexCount = triangleCount * 3;
		}
		meshData->nIndexCount = triangleCount * 3;

		meshData->pVB = new FbxMeshVertex_Tmp[meshData->nVertexCount];
		meshData->pIB = new unsigned int[meshData->nIndexCount];

		const char* uvName = NULL;
		FbxStringList listUVNames;
		pMesh->GetUVSetNames(listUVNames);
		if (hasUV && listUVNames.GetCount())
		{
			uvName = listUVNames[0];
		}

		const FbxVector4* controlPoints = pMesh->GetControlPoints();
		FbxVector4 currentVertex;
		FbxVector4 currentNormal;
		FbxVector2 currentUV;
		if (allByControlPoint)
		{
			const FbxGeometryElementNormal * normalElement = NULL;
			const FbxGeometryElementUV * uvElement = NULL;
			if (hasNormal)
			{
				normalElement = pMesh->GetElementNormal(0);
			}
			if (hasUV)
			{
				uvElement = pMesh->GetElementUV(0);
			}
			for (int i = 0; i < meshData->nVertexCount; ++i)
			{
				currentVertex = controlPoints[i];
				FbxMeshVertex_Tmp* vertex = &(meshData->pVB[i]);
				vertex->pos.x = (float)currentVertex[0];
				vertex->pos.y = (float)currentVertex[1];
				vertex->pos.z = (float)currentVertex[2];

				if (hasNormal)
				{
					int normalIndex = i;
					if (normalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
					{
						normalIndex = normalElement->GetIndexArray().GetAt(i);
					}
					currentNormal = normalElement->GetDirectArray().GetAt(normalIndex);
					vertex->normal.x = (float)currentNormal[0];
					vertex->normal.y = (float)currentNormal[1];
					vertex->normal.z = (float)currentNormal[2];
				}
				else
				{
					vertex->normal.x = 0.0f;
					vertex->normal.y = 0.0f;
					vertex->normal.z = 0.0f;
				}
				if (hasUV)
				{
					int uvIndex = i;
					if (uvElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
					{
						uvIndex = uvElement->GetIndexArray().GetAt(i);
					}
					currentUV = uvElement->GetDirectArray().GetAt(uvIndex);
					vertex->uv.x = (float)currentUV[0];
					vertex->uv.y = (float)currentUV[1];
				}
				else
				{
					vertex->uv.x = 0.0f;
					vertex->uv.y = 0.0f;
				}
				vertex->color = 0xffffffff;
			}
		}

		int vertexIndex = 0;
		for (int polygonIndex = 0; polygonIndex < triangleCount; ++polygonIndex)
		{
			for (int i = 0; i < 3; ++i)
			{
				const int controlPointIndex = pMesh->GetPolygonVertex(polygonIndex, i);

				//if (allByControlPoint)
				{
					meshData->pIB[polygonIndex * 3 + i] = (unsigned int)controlPointIndex;
				}
				//else
				{
					//meshData->pIB[polygonIndex * 3 + i] = static_cast<unsigned int>(vertexIndex);

					FbxMeshVertex_Tmp* vertex = &(meshData->pVB[controlPointIndex]);// &(meshData->pVB[vertexIndex]);
					currentVertex = controlPoints[controlPointIndex];
					vertex->pos.x = (float)currentVertex[0];
					vertex->pos.y = (float)currentVertex[1];
					vertex->pos.z = (float)currentVertex[2];

					if (hasNormal)
					{
						pMesh->GetPolygonVertexNormal(polygonIndex, i, currentNormal);
						vertex->normal.x = (float)currentNormal[0];
						vertex->normal.y = (float)currentNormal[1];
						vertex->normal.z = (float)currentNormal[2];
					}
					else
					{
						vertex->normal.x = 0.0f;
						vertex->normal.y = 0.0f;
						vertex->normal.z = 0.0f;
					}
					if (hasUV)
					{
						bool unmappedUV;
						pMesh->GetPolygonVertexUV(polygonIndex, i, uvName, currentUV, unmappedUV);
						vertex->uv.x = (float)currentUV[0];
						vertex->uv.y = (float)currentUV[1];
					}
					else
					{
						vertex->uv.x = 0.0f;
						vertex->uv.y = 0.0f;
					}
					vertex->color = 0xffffffff;
				}
				++vertexIndex;
			}
		}
		pMeshList->mMeshes.Add(meshData);
		FbxSkinInfo* skinInfo = ProcessSkin(pMesh, meshData->nVertexCount);
	}

	void ProcessNode(FbxNode* pNode, FbxNode* pParent, int mask)
	{
		FbxNodeAttribute* attributeType = pNode->GetNodeAttribute();
		if (attributeType)
		{
			switch (attributeType->GetAttributeType())
			{
			case FbxNodeAttribute::eMesh:
				if (mask == -1 || mask == FbxNodeAttribute::eMesh)
					ProcessMesh(pNode);
				break;
			case FbxNodeAttribute::eSkeleton:
				if (mask == -1 || mask == FbxNodeAttribute::eSkeleton)
					ProcessSkeleton(pNode, pParent);
				break;
			default:
				break;
			}
		}
		for (int i = 0; i < pNode->GetChildCount(); ++i)
		{
			ProcessNode(pNode->GetChild(i), pNode, mask);
		}
	}

	FbxAnimationEvaluator* GetAnimationEvaluator()
	{
		return pAnimEvaluator;
	}

	FbxBoneMap* GetBoneMap()
	{
		return pSkeleton;
	}

	FbxModelList* GetModelList()
	{
		return pMeshList;
	}

	bool SetCurrentAnimation(const char* animName)
	{
		int numStacks = pFBXScene->GetSrcObjectCount<FbxAnimStack>();
		for (int i = 0; i < numStacks; ++i)
		{
			FbxAnimStack* pAnimStack = pFBXScene->GetSrcObject<FbxAnimStack>(i);
			if (!pAnimStack) continue;
			std::string name = pAnimStack->GetName();
			if (name == animName)
			{
				pFBXScene->SetCurrentAnimationStack(pAnimStack);
				FbxTimeSpan span = pAnimStack->GetLocalTimeSpan();
				mDuration = (float)span.GetDuration().GetSecondDouble();
				return true;
			}
		}
		return false;
	}

	void UpdateSkeleton()
	{
		if (!pSkeleton || !pAnimEvaluator)
			return;
		DWORD curTime = timeGetTime();
		DWORD timeDelta = curTime - mLastTime;
		mLastTime = curTime;

		float dt = (float)timeDelta * 0.001f;
		mAnimTime += dt;
		if (mAnimTime > mDuration)
			mAnimTime = 0.0f;

		FbxBoneMap* bonemap = pSkeleton;
		FBXHelper::FbxAnimationEvaluator* animEvaluator = pAnimEvaluator;

		for (int i = 0; i < bonemap->mBoneList.Count(); ++i)
		{
			FBXHelper::FbxBone* bone = bonemap->mBoneList[i];
			bone->offset = animEvaluator->Evaluator(bone, mAnimTime);
		}
	}

	bool EndFBXHelper()
	{
		bool rst = true;
		mLastTime = 0l;
		mAnimTime = 0.0f;
		mDuration = 0.0f;
		DestroySdkObjects(pFBXSDKManager, rst);
		pFBXSDKManager = NULL;
		pFBXScene = NULL;

		if (pMeshList)
		{
			delete pMeshList;
			pMeshList = NULL;
		}
		if (pSkeleton)
		{
			delete pSkeleton;
			pSkeleton = NULL;
		}
		if (pAnimEvaluator)
		{
			delete pAnimEvaluator;
			pAnimEvaluator = NULL;
		}

		return true;
	}
}