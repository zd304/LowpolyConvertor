#include "FBXHelper.h"

namespace FBXHelper
{
	static FbxManager* pFBXSDKManager = NULL;
	static FbxScene* pFBXScene = NULL;
	static FbxModelList* pMeshList = NULL;
	static FbxBoneMap* pSkeleton = NULL;
	static FbxAnimationEvaluator* pAnimEvaluator = NULL;
	static DWORD mLastTime = 0l;
	static float mAnimTime = 0.0f;
	static float mDuration = 0.0f;
	static D3DXVECTOR3 boxMax;
	static D3DXVECTOR3 boxMin;
	static bool isWorking = false;

	static D3DXVECTOR3 Vec3One(1.0f, 1.0f, 1.0f);
	static D3DXVECTOR3 Vec3Zero(0.0f, 0.0f, 0.0f);
	static D3DXMATRIX MatIdentity = D3DXMATRIX(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		);

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

	D3DXMATRIX ToD3DMatrix(const FbxAMatrix& mat)
	{
		D3DXMATRIX mm;
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

	FbxAnimationEvaluator::FbxAnimationEvaluator()
	{
	}

	FbxAnimationEvaluator::~FbxAnimationEvaluator()
	{
	}

	D3DXMATRIX FbxAnimationEvaluator::Evaluator(FbxBone* bone, float second)
	{
		FbxTime t;
		t.SetSecondDouble((double)second);
		FbxAMatrix mat = bone->node->EvaluateGlobalTransform(t);
		return ToD3DMatrix(mat);
	}

	void UpdateBox(const D3DXVECTOR3& v)
	{
		if (boxMax.x < v.x)
			boxMax.x = v.x;
		if (boxMax.y < v.y)
			boxMax.y = v.y;
		if (boxMax.z < v.z)
			boxMax.z = v.z;

		if (boxMin.x > v.x)
			boxMin.x = v.x;
		if (boxMin.y > v.y)
			boxMin.y = v.y;
		if (boxMin.z > v.z)
			boxMin.z = v.z;
	}

	void ProcessNode(FbxNode* pNode, FbxNode* pParent = NULL, int mask = -1);

	bool BeginFBXHelper(const char* fileName)
	{
		isWorking = true;

		InitializeSdkObjects(pFBXSDKManager, pFBXScene);

		bool rst = LoadScene(pFBXSDKManager, pFBXScene, fileName);

		boxMax.x = FLT_MIN;
		boxMax.y = FLT_MIN;
		boxMax.z = FLT_MIN;
		boxMin.x = FLT_MAX;
		boxMin.y = FLT_MAX;
		boxMin.z = FLT_MAX;

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

		int numStacks = pFBXScene->GetSrcObjectCount<FbxAnimStack>();
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
		{
			pMeshList->mSkins.Add(NULL);
			return NULL;
		}
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
				UpdateBox(vertex->pos);

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
					UpdateBox(vertex->pos);

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

	FbxBoneMap* GetBoneMap()
	{
		return pSkeleton;
	}

	FbxModelList* GetModelList()
	{
		return pMeshList;
	}

	void GetBox(D3DXVECTOR3& max, D3DXVECTOR3& min)
	{
		max = boxMax;
		min = boxMin;
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

	bool GetAniamtionNames(List<const char*>& names)
	{
		bool rst = false;
		int numStacks = pFBXScene->GetSrcObjectCount<FbxAnimStack>();
		for (int i = 0; i < numStacks; ++i)
		{
			FbxAnimStack* pAnimStack = pFBXScene->GetSrcObject<FbxAnimStack>(i);
			if (!pAnimStack) continue;
			names.Add(pAnimStack->GetName());
			rst = true;
		}
		return rst;
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

	bool IsWorking()
	{
		return isWorking;
	}

	FbxManager* GetFbxManager()
	{
		return pFBXSDKManager;
	}

	FbxScene* GetFbxScene()
	{
		return pFBXScene;
	}

	bool EndFBXHelper()
	{
		bool rst = true;
		mLastTime = 0l;
		mAnimTime = 0.0f;
		mDuration = 0.0f;
		boxMax.x = FLT_MIN;
		boxMax.y = FLT_MIN;
		boxMax.z = FLT_MIN;
		boxMin.x = FLT_MAX;
		boxMin.y = FLT_MAX;
		boxMin.z = FLT_MAX;
		DestroySdkObjects(pFBXSDKManager, rst);
		pFBXSDKManager = NULL;
		pFBXScene = NULL;
		isWorking = false;

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