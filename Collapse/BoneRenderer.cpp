#include "BoneRenderer.h"
#include "FBXHelper.h"

struct CustomVertex_br
{
	D3DXVECTOR3 pos;
	static DWORD fvf;
};

DWORD CustomVertex_br::fvf = D3DFVF_XYZ;

BoneRenderer::BoneRenderer(IDirect3DDevice9* device)
{
	mDevice = device;
	mMesh = NULL;
}

BoneRenderer::~BoneRenderer()
{
	SAFE_RELEASE(mMesh);
}

void BoneRenderer::BuildMesh()
{
	FBXHelper::FbxBoneMap* bonemap = FBXHelper::GetBoneMap();
	if (!bonemap)
		return;
	std::vector<CustomVertex_br> sk_vb;
	std::vector<unsigned int> sk_ib;
	int triIndex = 0;
	for (int i = 0; i < bonemap->mBoneList.Count(); ++i)
	{
		FBXHelper::FbxBone* bone = bonemap->mBoneList[i];
		if (!bone->parent)
		{
			continue;
		}
		D3DXVECTOR3 start(bone->parent->bindPose._41, bone->parent->bindPose._42, bone->parent->bindPose._43);
		D3DXVECTOR3 end(bone->bindPose._41, bone->bindPose._42, bone->bindPose._43);
		D3DXVECTOR3 subRight(bone->parent->bindPose._31, bone->parent->bindPose._32, bone->parent->bindPose._33);
		D3DXVec3Normalize(&subRight, &subRight);

		D3DXVECTOR3 daxis = end - start;
		D3DXVECTOR3 naxis;
		D3DXVec3Normalize(&naxis, &daxis);

		D3DXVECTOR3 subForward;
		D3DXVec3Cross(&subForward, &subRight, &naxis);
		D3DXVec3Normalize(&subForward, &subForward);
		D3DXVec3Cross(&subRight, &naxis, &subForward);
		D3DXVec3Normalize(&subRight, &subRight);

		float thick = 0.2f;// D3DXVec3Length(&daxis) * 0.1f;

		CustomVertex_br cvt{ end };
		sk_vb.push_back(cvt);
		CustomVertex_br cv1{ start + thick * subRight };
		sk_vb.push_back(cv1);
		CustomVertex_br cv2{ start - thick * subRight };
		sk_vb.push_back(cv2);
		CustomVertex_br cv3{ start + thick * subForward };
		sk_vb.push_back(cv3);
		CustomVertex_br cv4{ start - thick * subForward };
		sk_vb.push_back(cv4);

		sk_ib.push_back(0 + triIndex);
		sk_ib.push_back(1 + triIndex);
		sk_ib.push_back(2 + triIndex);
		sk_ib.push_back(0 + triIndex);
		sk_ib.push_back(2 + triIndex);
		sk_ib.push_back(3 + triIndex);
		sk_ib.push_back(0 + triIndex);
		sk_ib.push_back(3 + triIndex);
		sk_ib.push_back(4 + triIndex);
		sk_ib.push_back(0 + triIndex);
		sk_ib.push_back(4 + triIndex);
		sk_ib.push_back(1 + triIndex);
		sk_ib.push_back(1 + triIndex);
		sk_ib.push_back(2 + triIndex);
		sk_ib.push_back(3 + triIndex);
		sk_ib.push_back(2 + triIndex);
		sk_ib.push_back(1 + triIndex);
		sk_ib.push_back(4 + triIndex);

		triIndex += 5;
	}
	D3DXCreateMeshFVF(sk_ib.size() / 3, sk_vb.size(), D3DXMESH_32BIT, CustomVertex_br::fvf, mDevice, &mMesh);
	void* vertices = NULL;
	mMesh->LockVertexBuffer(0, &vertices);
	memcpy(vertices, &(sk_vb[0]), sizeof(CustomVertex_br) * sk_vb.size());
	mMesh->UnlockVertexBuffer();

	void* indices = NULL;
	mMesh->LockIndexBuffer(0, &indices);
	memcpy(indices, &(sk_ib[0]), sizeof(unsigned int) * sk_ib.size());
	mMesh->UnlockVertexBuffer();
}

void BoneRenderer::Render()
{
	mDevice->SetFVF(CustomVertex_br::fvf);

	FBXHelper::FbxBoneMap* bonemap = FBXHelper::GetBoneMap();
	std::vector<CustomVertex_br> sk_vb;
	std::vector<unsigned int> sk_ib;
	int triIndex = 0;
	for (int i = 0; i < bonemap->mBoneList.Count(); ++i)
	{
		FBXHelper::FbxBone* bone = bonemap->mBoneList[i];
		if (!bone->parent)
		{
			continue;
		}
		D3DXVECTOR3 start(bone->parent->offset._41, bone->parent->offset._42, bone->parent->offset._43);
		D3DXVECTOR3 end(bone->offset._41, bone->offset._42, bone->offset._43);
		D3DXVECTOR3 subRight(bone->parent->offset._31, bone->parent->offset._32, bone->parent->offset._33);
		D3DXVec3Normalize(&subRight, &subRight);

		D3DXVECTOR3 daxis = end - start;
		D3DXVECTOR3 naxis;
		D3DXVec3Normalize(&naxis, &daxis);

		D3DXVECTOR3 subForward;
		D3DXVec3Cross(&subForward, &subRight, &naxis);
		D3DXVec3Normalize(&subForward, &subForward);
		D3DXVec3Cross(&subRight, &naxis, &subForward);
		D3DXVec3Normalize(&subRight, &subRight);

		float thick = 0.2f;// D3DXVec3Length(&daxis) * 0.1f;

		CustomVertex_br cvt{ end };
		sk_vb.push_back(cvt);
		CustomVertex_br cv1{ start + thick * subRight };
		sk_vb.push_back(cv1);
		CustomVertex_br cv2{ start - thick * subRight };
		sk_vb.push_back(cv2);
		CustomVertex_br cv3{ start + thick * subForward };
		sk_vb.push_back(cv3);
		CustomVertex_br cv4{ start - thick * subForward };
		sk_vb.push_back(cv4);

		sk_ib.push_back(0 + triIndex);
		sk_ib.push_back(1 + triIndex);
		sk_ib.push_back(2 + triIndex);
		sk_ib.push_back(0 + triIndex);
		sk_ib.push_back(2 + triIndex);
		sk_ib.push_back(3 + triIndex);
		sk_ib.push_back(0 + triIndex);
		sk_ib.push_back(3 + triIndex);
		sk_ib.push_back(4 + triIndex);
		sk_ib.push_back(0 + triIndex);
		sk_ib.push_back(4 + triIndex);
		sk_ib.push_back(1 + triIndex);
		sk_ib.push_back(1 + triIndex);
		sk_ib.push_back(2 + triIndex);
		sk_ib.push_back(3 + triIndex);
		sk_ib.push_back(2 + triIndex);
		sk_ib.push_back(1 + triIndex);
		sk_ib.push_back(4 + triIndex);

		triIndex += 5;
	}
	void* vertices = NULL;
	mMesh->LockVertexBuffer(0, &vertices);
	memcpy(vertices, &(sk_vb[0]), sizeof(CustomVertex_br)* sk_vb.size());
	mMesh->UnlockVertexBuffer();

	void* indices = NULL;
	mMesh->LockIndexBuffer(0, &indices);
	memcpy(indices, &(sk_ib[0]), sizeof(unsigned int)* sk_ib.size());
	mMesh->UnlockVertexBuffer();

	mMesh->DrawSubset(0);
}