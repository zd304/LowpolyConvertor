#include "FBXHelper.h"
#include <string>

namespace FBXHelper
{
	FbxManager* pFBXSDKManager = NULL;
	FbxScene* pFBXScene = NULL;
	FbxModelList* pMeshList = NULL;
	FbxBoneMap* pSkeleton = NULL;
	FbxAnimationEvaluator* pAnimEvaluator = NULL;

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

	D3DXMATRIX FbxAnimationEvaluator::Evaluator(FbxBone* bone, const char* animName, float second)
	{
		BoneAniamtion* p = (BoneAniamtion*)mCurveData;
		BoneAniamtion::IT_BA itba = p->mBoneCurves.find(bone);
		if (itba == p->mBoneCurves.end())
			return matIdentity;
		AnimationCurve* curve = itba->second;
		AnimationCurve::IT_AC itac = curve->animCurves.find(animName);
		if (itac == curve->animCurves.end())
			return matIdentity;
		LocalCurve* lclCurve = itac->second;

		FbxTime t;
		t.SetSecondDouble((double)second);

//#define __LOCAL_EVALUATE__
#ifdef __LOCAL_EVALUATE__
		FbxVector4 translation(0.0, 0.0, 0.0);
		FbxVector4 rotation(0.0, 0.0, 0.0);
		FbxVector4 scaling(1.0f, 1.0, 1.0, 1.0);
		if (lclCurve->translationX) translation.mData[0] = lclCurve->translationX->Evaluate(t);
		if (lclCurve->translationY) translation.mData[1] = lclCurve->translationY->Evaluate(t);
		if (lclCurve->translationZ) translation.mData[2] = lclCurve->translationZ->Evaluate(t);
		//if (lclCurve->rotationX) rotation.mData[0] = lclCurve->rotationX->Evaluate(t);
		//if (lclCurve->rotationY) rotation.mData[1] = lclCurve->rotationY->Evaluate(t);
		//if (lclCurve->rotationZ) rotation.mData[2] = lclCurve->rotationZ->Evaluate(t);
		//if (lclCurve->scaleX) scaling.mData[0] = lclCurve->scaleX->Evaluate(t);
		//if (lclCurve->scaleY) scaling.mData[1] = lclCurve->scaleY->Evaluate(t);
		//if (lclCurve->scaleZ) scaling.mData[2] = lclCurve->scaleZ->Evaluate(t);
		FbxAMatrix mat = FbxAMatrix(translation, rotation, scaling);
#else
		FbxAMatrix mat = lclCurve->node->EvaluateGlobalTransform(t);
#endif
		if (bone->meshNode)
		{
			FbxNode* mn = bone->meshNode->GetNode();
			FbxAMatrix matMesh =  mn->EvaluateGlobalTransform(t);
			mat = matMesh.Inverse() * mat;
		}
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

	void ProcessNode(FbxNode* pNode, FbxNode* pParent = NULL);

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

		pMeshList = new FbxModelList();

		int numStacks = pFBXScene->GetSrcObjectCount<fbxsdk_2015_1::FbxAnimStack>();
		if (numStacks > 0)
		{
			pAnimEvaluator = new FbxAnimationEvaluator();
		}

		ProcessNode(pFBXScene->GetRootNode());

		return rst;
	}

	void ProcessSkin(FbxMesh* pMesh, int vtxCount)
	{
		FbxSkinInfo* skinInfo = new FbxSkinInfo();
		pMeshList->mSkins.Add(skinInfo);
		// スキンの数を取得;
		int skinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);
		if (skinCount == 0)
			return;
		skinInfo->weights = new FbxBoneWeight[vtxCount];
		skinInfo->size = vtxCount;
		for (int s = 0; s < skinCount; ++s)
		{
			// s番目のスキンを取得;
			FbxSkin* skinDeformer = (FbxSkin*)pMesh->GetDeformer(s, FbxDeformer::eSkin);

			for (int i = 0; i < skinDeformer->GetClusterCount(); ++i)
			{
				FbxCluster* cluster = skinDeformer->GetCluster(i);
				FbxNode* link = cluster->GetLink();
				if (!link) continue;
				const char* boneName = link->GetName();

				FbxBoneMap::IT_BM itbm = pSkeleton->mBones.find(boneName);
				if (itbm != pSkeleton->mBones.end())
				{
					FbxBone* bone = itbm->second;
					FbxAMatrix transformLinkMatrix, transformMatrix, matBindPose;

					// ボーンの初期姿勢を取得;
					cluster->GetTransformLinkMatrix(transformLinkMatrix);

					cluster->GetTransformMatrix(transformMatrix);
					FbxVector4 vt = pMesh->GetNode()->GetGeometricTranslation(FbxNode::eSourcePivot);
					FbxVector4 vr = pMesh->GetNode()->GetGeometricRotation(FbxNode::eSourcePivot);
					FbxVector4 vs = pMesh->GetNode()->GetGeometricScaling(FbxNode::eSourcePivot);
					FbxAMatrix geometry(vt, vr, vs);
					transformMatrix *= geometry;

					bone->meshNode = pMesh;

					matBindPose = transformMatrix.Inverse() * transformLinkMatrix;
					bone->bindPose = ToD3DMatrix(matBindPose);
				}
				else
				{
					printf("Not Find %s\n", boneName);
				}

				int* indices = cluster->GetControlPointIndices();
				double* weights = cluster->GetControlPointWeights();
				for (int j = 0; j < cluster->GetControlPointIndicesCount(); ++j)
				{
					int vtxIndex = indices[j];
					double weight = weights[j];
					FbxBoneWeight& bw = skinInfo->weights[vtxIndex];
					bw.boneName.Add(boneName);
					bw.weight.Add(weight);
				}
			}
		}
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

		FbxAMatrix matBindPose;// = pNode->EvaluateGlobalTransform();
		FbxVector4 vt(pNode->LclTranslation.Get());
		FbxVector4 vr(pNode->LclRotation.Get());
		FbxVector4 vs(pNode->LclScaling.Get());
		matBindPose.SetTRS(vt, vr, vs);

		if (parent)
		{
			FbxBone* parentBone = pSkeleton->mBones[parent->GetName()];
			bone->parent = parentBone;
			parentBone->children.push_back(bone);
			matBindPose = ToFbxMatrix(parentBone->bindPose) * matBindPose;
		}

		matBindPose.SetS(FbxVector4(1, 1, 1, 1));
		bone->bindPose = ToD3DMatrix(matBindPose);
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
			meshData->nVertexCount = triangleCount * 3;
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

				if (allByControlPoint)
				{
					meshData->pIB[polygonIndex * 3 + i] = (unsigned int)controlPointIndex;
				}
				else
				{
					meshData->pIB[polygonIndex * 3 + i] = static_cast<unsigned int>(vertexIndex);

					FbxMeshVertex_Tmp* vertex = &(meshData->pVB[vertexIndex]);
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
		ProcessSkin(pMesh, meshData->nVertexCount);
	}

	void ProcessNode(FbxNode* pNode, FbxNode* pParent)
	{
		FbxNodeAttribute* attributeType = pNode->GetNodeAttribute();
		if (attributeType)
		{
			switch (attributeType->GetAttributeType())
			{
			case FbxNodeAttribute::eMesh:
				ProcessMesh(pNode);
				break;
			case FbxNodeAttribute::eSkeleton:
				ProcessSkeleton(pNode, pParent);
				break;
			default:
				break;
			}
		}
		for (int i = 0; i < pNode->GetChildCount(); ++i)
		{
			ProcessNode(pNode->GetChild(i), pNode);
		}
	}

	void GetMesh(void** ppVB, int& v_stride, int& v_count, void** ppIB, int& i_stride, int& i_count)
	{
		if (!pMeshList) return;
		if (pMeshList->mMeshes.Count() == 0)
			return;
		FbxModel* mesh = pMeshList->mMeshes[0];
		(*ppVB) = mesh->pVB;
		v_stride = sizeof(FbxMeshVertex_Tmp);
		v_count = mesh->nVertexCount;
		(*ppIB) = mesh->pIB;
		i_stride = sizeof(unsigned int);
		i_count = mesh->nIndexCount;
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

	bool EndFBXHelper()
	{
		bool rst = true;
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