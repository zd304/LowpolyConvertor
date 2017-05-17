#include "FBXHelper.h"
#include <d3dx9.h>

namespace FBXHelper
{
	FbxManager* pFBXSDKManager = NULL;
	FbxScene* pFBXScene = NULL;

	struct FbxMeshVertex_Tmp
	{
		D3DXVECTOR3 pos;
		D3DXVECTOR3 normal;
		unsigned int color;
		D3DXVECTOR2 uv;
	};
	FbxMeshVertex_Tmp* pVB = NULL;
	int nVertexCount = 0;
	unsigned int* pIB = NULL;
	int nIndexCount = 0;

	void ProcessNode(FbxNode* pNode);

	bool BeginFBXHelper(const char* fileName)
	{
		InitializeSdkObjects(pFBXSDKManager, pFBXScene);

		bool rst = LoadScene(pFBXSDKManager, pFBXScene, fileName);

		if (rst == false)
		{
			FBXSDK_printf("\n\nAn error occurred while loading the scene...");
			return false;
		}

		ProcessNode(pFBXScene->GetRootNode());
	}

	void ReadVertex(FbxMesh* pMesh, int ctrlPointIndex)
	{
		if (ctrlPointIndex < 0 || ctrlPointIndex > nVertexCount)
			return;
		FbxVector4* pCtrlPoint = pMesh->GetControlPoints();

		FbxVector4 pt = pCtrlPoint[ctrlPointIndex];

		FbxMeshVertex_Tmp& vtx = pVB[ctrlPointIndex];
		vtx.pos.x = (float)pt.mData[0];
		vtx.pos.y = (float)pt.mData[1];
		vtx.pos.z = (float)pt.mData[2];
	}

	void ReadColor(FbxMesh* pMesh, int ctrlPointIndex, int vertexCounter)
	{
		if (ctrlPointIndex < 0 || ctrlPointIndex > nVertexCount)
			return;
		if (pMesh->GetElementVertexColorCount() < 1)
		{
			return;
		}

		FbxMeshVertex_Tmp& vtx = pVB[ctrlPointIndex];
		FbxGeometryElementVertexColor* pVertexColor = pMesh->GetElementVertexColor(0);
		double w = 0, x = 0, y = 0, z = 0;
		switch (pVertexColor->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
		{
			switch (pVertexColor->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				x = pVertexColor->GetDirectArray().GetAt(ctrlPointIndex).mRed;
				y = pVertexColor->GetDirectArray().GetAt(ctrlPointIndex).mGreen;
				z = pVertexColor->GetDirectArray().GetAt(ctrlPointIndex).mBlue;
				w = pVertexColor->GetDirectArray().GetAt(ctrlPointIndex).mAlpha;
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int id = pVertexColor->GetIndexArray().GetAt(ctrlPointIndex);
				x = pVertexColor->GetDirectArray().GetAt(id).mRed;
				y = pVertexColor->GetDirectArray().GetAt(id).mGreen;
				z = pVertexColor->GetDirectArray().GetAt(id).mBlue;
				w = pVertexColor->GetDirectArray().GetAt(id).mAlpha;
			}
			break;

			default:
				break;
			}
		}
		break;

		case FbxGeometryElement::eByPolygonVertex:
		{
			switch (pVertexColor->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				x = pVertexColor->GetDirectArray().GetAt(vertexCounter).mRed;
				y = pVertexColor->GetDirectArray().GetAt(vertexCounter).mGreen;
				z = pVertexColor->GetDirectArray().GetAt(vertexCounter).mBlue;
				w = pVertexColor->GetDirectArray().GetAt(vertexCounter).mAlpha;
			}
			break;
			case FbxGeometryElement::eIndexToDirect:
			{
				int id = pVertexColor->GetIndexArray().GetAt(vertexCounter);
				x = pVertexColor->GetDirectArray().GetAt(id).mRed;
				y = pVertexColor->GetDirectArray().GetAt(id).mGreen;
				z = pVertexColor->GetDirectArray().GetAt(id).mBlue;
				w = pVertexColor->GetDirectArray().GetAt(id).mAlpha;
			}
			break;
			default:
				break;
			}
		}
		break;
		}

		unsigned int a = (unsigned int)(w * 255.0);
		unsigned int r = (unsigned int)(x * 255.0);
		unsigned int g = (unsigned int)(y * 255.0);
		unsigned int b = (unsigned int)(z * 255.0);
		vtx.color = b | (g << 8) | (r << 16) | (a << 24);
	}

	void ReadUV(FbxMesh* pMesh, int ctrlPointIndex, int textureUVIndex, int uvLayer)
	{
		if (ctrlPointIndex < 0 || ctrlPointIndex > nVertexCount)
			return;
		if (uvLayer >= 2 || pMesh->GetElementUVCount() <= uvLayer)
		{
			return;
		}

		FbxGeometryElementUV* pVertexUV = pMesh->GetElementUV(uvLayer);
		FbxMeshVertex_Tmp& vtx = pVB[ctrlPointIndex];

		switch (pVertexUV->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
		{
			switch (pVertexUV->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				FbxVector2 pt = pVertexUV->GetDirectArray().GetAt(ctrlPointIndex);
				vtx.uv.x = (float)pt.mData[0];
				vtx.uv.y = (float)pt.mData[1];
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int id = pVertexUV->GetIndexArray().GetAt(ctrlPointIndex);
				FbxVector2 pt = pVertexUV->GetDirectArray().GetAt(id);
				vtx.uv.x = (float)pt.mData[0];
				vtx.uv.y = (float)pt.mData[1];
			}
			break;

			default:
				break;
			}
		}
		break;

		case FbxGeometryElement::eByPolygonVertex:
		{
			switch (pVertexUV->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			case FbxGeometryElement::eIndexToDirect:
			{
				FbxVector2 pt = pVertexUV->GetDirectArray().GetAt(textureUVIndex);
				vtx.uv.x = (float)pt.mData[0];
				vtx.uv.y = (float)pt.mData[1];
			}
			break;

			default:
				break;
			}
		}
		break;
		}
	}
	
	void ReadNormal(FbxMesh* pMesh, int ctrlPointIndex, int vertexCounter)
	{
		if (ctrlPointIndex < 0 || ctrlPointIndex > nVertexCount)
			return;
		if (pMesh->GetElementNormalCount() < 1)
		{
			return;
		}

		FbxMeshVertex_Tmp& vtx = pVB[ctrlPointIndex];
		FbxGeometryElementNormal* leNormal = pMesh->GetElementNormal(0);
		switch (leNormal->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
		{
			switch (leNormal->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				FbxVector4 v = leNormal->GetDirectArray().GetAt(ctrlPointIndex);
				vtx.normal.x = (float)v.mData[0];
				vtx.normal.y = (float)v.mData[1];
				vtx.normal.z = (float)v.mData[2];
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int id = leNormal->GetIndexArray().GetAt(ctrlPointIndex);
				FbxVector4 v = leNormal->GetDirectArray().GetAt(id);
				vtx.normal.x = (float)v.mData[0];
				vtx.normal.y = (float)v.mData[1];
				vtx.normal.z = (float)v.mData[2];
			}
			break;

			default:
				break;
			}
		}
		break;

		case FbxGeometryElement::eByPolygonVertex:
		{
			switch (leNormal->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				FbxVector4 v = leNormal->GetDirectArray().GetAt(vertexCounter);
				vtx.normal.x = (float)v.mData[0];
				vtx.normal.y = (float)v.mData[1];
				vtx.normal.z = (float)v.mData[2];
			}
			break;

			case FbxGeometryElement::eIndexToDirect:
			{
				int id = leNormal->GetIndexArray().GetAt(vertexCounter);
				FbxVector4 v = leNormal->GetDirectArray().GetAt(id);
				vtx.normal.x = (float)v.mData[0];
				vtx.normal.y = (float)v.mData[1];
				vtx.normal.z = (float)v.mData[2];
			}
			break;

			default:
				break;
			}
		}
		break;
		}
	}

	void ProcessMesh(FbxNode* pNode)
	{
		FbxMesh* pMesh = pNode->GetMesh();
		if (pMesh == NULL)
		{
			return;
		}
		if (pVB != NULL)
		{
			return;
		}

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

		nVertexCount = pMesh->GetControlPointsCount();
		if (!allByControlPoint)
		{
			nVertexCount = triangleCount * 3;
		}
		nIndexCount = triangleCount * 3;

		pVB = new FbxMeshVertex_Tmp[nVertexCount];
		pIB = new unsigned int[nIndexCount];

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
			for (int i = 0; i < nVertexCount; ++i)
			{
				currentVertex = controlPoints[i];
				FbxMeshVertex_Tmp* vertex = &(pVB[i]);
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
					pIB[polygonIndex * 3 + i] = (unsigned int)controlPointIndex;
				}
				else
				{
					pIB[polygonIndex * 3 + i] = static_cast<unsigned int>(vertexIndex);

					FbxMeshVertex_Tmp* vertex = &(pVB[vertexIndex]);
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
	}

	void ProcessNode(FbxNode* pNode)
	{
		FbxNodeAttribute* attributeType = pNode->GetNodeAttribute();
		if (attributeType)
		{
			switch (attributeType->GetAttributeType())
			{
			case FbxNodeAttribute::eMesh:
				ProcessMesh(pNode);
				break;
			}
		}
		for (int i = 0; i < pNode->GetChildCount(); ++i)
		{
			ProcessNode(pNode->GetChild(i));
		}
	}

	void GetMesh(void** ppVB, int& v_stride, int& v_count, void** ppIB, int& i_stride, int& i_count)
	{
		(*ppVB) = pVB;
		v_stride = sizeof(FbxMeshVertex_Tmp);
		v_count = nVertexCount;
		(*ppIB) = pIB;
		i_stride = sizeof(unsigned int);
		i_count = nIndexCount;
	}

	bool EndFBXHelper()
	{
		bool rst = true;
		DestroySdkObjects(pFBXSDKManager, rst);
		pFBXSDKManager = NULL;
		pFBXScene = NULL;

		if (pVB)
		{
			delete[] pVB;
			pVB = NULL;
		}
		if (pIB)
		{
			delete[] pIB;
			pIB = NULL;
		}
		nVertexCount = 0;
		nIndexCount = 0;

		return true;
	}
}