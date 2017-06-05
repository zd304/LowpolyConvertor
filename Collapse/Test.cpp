#include "Test.h"
#include "Collapse.h"
#include <fstream>

FocuseBoneSkin::FocuseBoneSkin()
{

}

FocuseBoneSkin::~FocuseBoneSkin()
{
	for (IT_FBS it = skins.begin(); it != skins.end(); ++it)
	{
		FocusBoneWeight* fbw = it->second;
		if (!fbw) continue;
		delete fbw;
	}
	skins.clear();
}

Test::Test()
{
	mLastTime = 0;
	mAnimTime = 0.0f;
}

Test::~Test()
{
	mLastTime = 0;
}

void Test::OnInit(HWND hwnd, IDirect3DDevice9* device)
{
	mDevice = device;
	mHwnd = hwnd;

	mLastTime = timeGetTime();
	mAnimTime = 0.0f;

	FBXHelper::BeginFBXHelper("humanoid.fbx");
	
	FbxAMatrix mat;
	mat.SetIdentity();
	FbxVector4 axis(1.0, 1.0, 1.0);
	axis.Normalize();
	FbxQuaternion q(axis, 45);
	mat.SetQ(q);
	printf("FbxAMatrxi:\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n",
		(float)mat.Get(0, 0), (float)mat.Get(0, 1), (float)mat.Get(0, 2), (float)mat.Get(0, 3),
		(float)mat.Get(1, 0), (float)mat.Get(1, 1), (float)mat.Get(1, 2), (float)mat.Get(1, 3),
		(float)mat.Get(2, 0), (float)mat.Get(2, 1), (float)mat.Get(2, 2), (float)mat.Get(2, 3),
		(float)mat.Get(3, 0), (float)mat.Get(3, 1), (float)mat.Get(3, 2), (float)mat.Get(3, 3));
	D3DXMATRIX d3dmat;
	D3DXQUATERNION d3dQ;
	D3DXVECTOR3 d3daxis(1.0f, 1.0f, 1.0f);
	D3DXVec3Normalize(&d3daxis, &d3daxis);
	D3DXQuaternionRotationAxis(&d3dQ, &d3daxis, 0.78539816325f);
	D3DXMatrixRotationQuaternion(&d3dmat, &d3dQ);
	printf("D3DXMATRIX:\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n",
		d3dmat(0, 0), d3dmat(0, 1), d3dmat(0, 2), d3dmat(0, 3),
		d3dmat(1, 0), d3dmat(1, 1), d3dmat(1, 2), d3dmat(1, 3),
		d3dmat(2, 0), d3dmat(2, 1), d3dmat(2, 2), d3dmat(2, 3),
		d3dmat(3, 0), d3dmat(3, 1), d3dmat(3, 2), d3dmat(3, 3));

	unsigned int* pib = NULL;
	int i_stride = 0;
	int i_count = 0;
	FBXHelper::GetMesh((void**)&pvb, v_stride, v_count, (void**)&pib, i_stride, i_count);

	size_t vertexNum = v_count;//mVertices.size();
	size_t faceNum = i_count / 3;//ids.size() / 3;
	fvf = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1;
	HRESULT hr = D3DXCreateMeshFVF(faceNum, vertexNum, D3DXMESH_32BIT, fvf, device, &mMesh);
	if (FAILED(hr))
	{
		return;
	}
	CustomVertex* vertices = NULL;
	mMesh->LockVertexBuffer(0, (void**)&vertices);
	memcpy(vertices, pvb, v_stride * v_count);
	mMesh->UnlockVertexBuffer();

	unsigned int* indices = NULL;
	mMesh->LockIndexBuffer(0, (void**)&indices);
	memcpy(indices, pib, i_stride * i_count);
	mMesh->UnlockVertexBuffer();

	D3DXMATRIX matView, matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4.0f, 800.0f / 600.0f, 0.1f, 10000.0f);
	D3DXMatrixLookAtLH(&matView, &D3DXVECTOR3(0.0f, 0.0f, -400.0f),
		&D3DXVECTOR3(0.0f, 0.0f, 400.0f), &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	D3DXMatrixTranslation(&mMatWorld, 0.0f, -0.1f, 0.0f);

	device->SetTransform(D3DTS_PROJECTION, &matProj);
	device->SetTransform(D3DTS_VIEW, &matView);
	device->SetTransform(D3DTS_WORLD, &mMatWorld);
	device->SetRenderState(D3DRS_LIGHTING, TRUE);
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	D3DLIGHT9 light;
	light.Ambient = D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f);
	light.Diffuse = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	light.Direction = D3DXVECTOR3(1.0f, -1.0f, 1.0f);
	light.Type = D3DLIGHTTYPE::D3DLIGHT_DIRECTIONAL;
	device->SetLight(0, &light);
	device->LightEnable(0, TRUE);
	material.Ambient = D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f);
	material.Diffuse = D3DXCOLOR(1.0f, 0.1f, 1.0f, 1.0f);
	device->SetMaterial(&material);
	rot = 0.0f;
	
	/*CustomVertex* vs = NULL;
	mMesh->LockVertexBuffer(0, (void**)&vs);
	Collapse::BeginCollapse(vs, sizeof(CustomVertex), v_count, 0,
		pib, sizeof(unsigned int), i_count);

	Collapse::DoCollapse(v_count / 10);

	Collapse::Buffer* buffer = Collapse::GetBuffer();
	mMesh->UnlockVertexBuffer();

	mMesh->Release();
	mMesh = NULL;

	vertexNum = buffer->v_count;
	faceNum = buffer->i_count * 3;

	hr = D3DXCreateMeshFVF(faceNum, vertexNum, D3DXMESH_32BIT, fvf, device, &mMesh);
	if (FAILED(hr))
	{
		return;
	}
	void* vb = NULL;
	mMesh->LockVertexBuffer(0, (void**)&vb);
	memcpy(vb, buffer->vertices, buffer->v_count * buffer->v_stride);
	mMesh->UnlockVertexBuffer();
	void* ib = NULL;
	mMesh->LockIndexBuffer(0, (void**)&ib);
	memcpy(ib, buffer->indices, buffer->i_count * buffer->i_stride);
	mMesh->UnlockIndexBuffer();

	Collapse::EndCollapse();*/

	mSkin = new FocuseBoneSkin();
	FBXHelper::FbxModelList* modelList = FBXHelper::GetModelList();
	FBXHelper::FbxBoneMap* bonemap = FBXHelper::GetBoneMap();
	if (modelList && modelList->mSkins.Count() > 0)
	{
		FBXHelper::FbxSkinInfo* skin = modelList->mSkins[0];
		for (unsigned int i = 0; i < skin->size; ++i)
		{
			FBXHelper::FbxBoneWeight* bw = &skin->weights[i];
			for (int j = 0; j < bw->boneName.Count(); ++j)
			{
				std::string& sName = bw->boneName[j];
				FocuseBoneSkin::IT_FBS itfbs = mSkin->skins.find(sName);
				if (itfbs == mSkin->skins.end())
				{
					mSkin->skins[sName] = new FocusBoneWeight();
				}
				FocusBoneWeight* fbw = mSkin->skins[sName];
				fbw->index.Add(i);
				fbw->weight.Add(bw->weight[j]);
			}
		}
	}

	std::vector<CustomVertex> sk_vb;
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

		float thick = 2.0f;// D3DXVec3Length(&daxis) * 0.1f;

		CustomVertex cvt{ end, D3DXVECTOR3(0.0f, 1.0f, 0.0f), 0xffffffff, D3DXVECTOR2() };
		sk_vb.push_back(cvt);
		CustomVertex cv1{ start + thick * subRight, D3DXVECTOR3(0.0f, 1.0f, 0.0f), 0xffffffff, D3DXVECTOR2() };
		sk_vb.push_back(cv1);
		CustomVertex cv2{ start - thick * subRight, D3DXVECTOR3(0.0f, 1.0f, 0.0f), 0xffffffff, D3DXVECTOR2() };
		sk_vb.push_back(cv2);
		CustomVertex cv3{ start + thick * subForward, D3DXVECTOR3(0.0f, 1.0f, 0.0f), 0xffffffff, D3DXVECTOR2() };
		sk_vb.push_back(cv3);
		CustomVertex cv4{ start - thick * subForward, D3DXVECTOR3(0.0f, 1.0f, 0.0f), 0xffffffff, D3DXVECTOR2() };
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
	D3DXCreateMeshFVF(sk_ib.size() / 3, sk_vb.size(), D3DXMESH_32BIT, fvf, device, &mSkeletonMesh);
	vertices = NULL;
	mSkeletonMesh->LockVertexBuffer(0, (void**)&vertices);
	memcpy(vertices, &(sk_vb[0]), sizeof(CustomVertex) * sk_vb.size());
	mSkeletonMesh->UnlockVertexBuffer();

	indices = NULL;
	mSkeletonMesh->LockIndexBuffer(0, (void**)&indices);
	memcpy(indices, &(sk_ib[0]), sizeof(unsigned int) * sk_ib.size());
	mSkeletonMesh->UnlockVertexBuffer();
}

void Test::OnUpdate()
{
	DWORD curTime = timeGetTime();
	DWORD timeDelta = curTime - mLastTime;

	D3DXMATRIX matRot;
	D3DXMatrixRotationY(&matRot, 0.01f);
	D3DXMatrixMultiply(&mMatWorld, &mMatWorld, &matRot);
	mDevice->SetTransform(D3DTS_WORLD, &mMatWorld);

	mDevice->SetFVF(fvf);

	CustomVertex* vertices = NULL;
	FBXHelper::FbxBoneMap* bonemap = FBXHelper::GetBoneMap();

	bool drawBones = false;
	bool drawAnimatedBones = false;
	bool drawMesh = true;

	mMesh->LockVertexBuffer(0, (void**)&vertices);

	for (int i = 0; i < v_count; ++i)
	{
		vertices[i].pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	}

	FBXHelper::FbxAnimationEvaluator* animEvaluator = FBXHelper::GetAnimationEvaluator();
	if (bonemap && animEvaluator)
	{
		float dt = (float)timeDelta * 0.001f;
		mAnimTime += dt;
		if (mAnimTime > 3.0f)
			mAnimTime = 0.0f;

		List<std::string> useNames;

		for (int i = 0; i < bonemap->mBoneList.Count(); ++i)
		{
			FBXHelper::FbxBone* bone = bonemap->mBoneList[i];
			bone->offset = animEvaluator->Evaluator(bone, "shot", mAnimTime);
			//if (bone->parent)
			//{
			//	D3DXMatrixMultiply(&bone->offset, &bone->parent->offset, &bone->offset);
			//}
			FocusBoneWeight* fbw = mSkin->skins[bone->name];
			if (!fbw) continue;

			useNames.Add(bone->name);

			D3DXMATRIX mat;
			D3DXMatrixInverse(&mat, NULL, &bone->bindPose);
			D3DXMatrixMultiply(&mat, &mat, &bone->offset);

			for (int j = 0; j < fbw->index.Count(); ++j)
			{
				int idx = fbw->index[j];
				double weight = fbw->weight[j];
				CustomVertex& cv = pvb[idx];
				
				D3DXVECTOR4 vec;
				D3DXVec3Transform(&vec, &cv.pos, &mat);
				vec = vec * (float)weight;
				vertices[idx].pos.x += vec.x;
				vertices[idx].pos.y += vec.y;
				vertices[idx].pos.z += vec.z;
			}
		}
	}

	mMesh->UnlockVertexBuffer();

	if (drawMesh)
	{
		mMesh->DrawSubset(0);
	}

	if (drawBones)
	{
		if (drawAnimatedBones)
		{
			std::vector<CustomVertex> sk_vb;
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

				float thick = 2.0f;// D3DXVec3Length(&daxis) * 0.1f;

				CustomVertex cvt{ end, D3DXVECTOR3(0.0f, 1.0f, 0.0f), 0xffffffff, D3DXVECTOR2() };
				sk_vb.push_back(cvt);
				CustomVertex cv1{ start + thick * subRight, D3DXVECTOR3(0.0f, 1.0f, 0.0f), 0xffffffff, D3DXVECTOR2() };
				sk_vb.push_back(cv1);
				CustomVertex cv2{ start - thick * subRight, D3DXVECTOR3(0.0f, 1.0f, 0.0f), 0xffffffff, D3DXVECTOR2() };
				sk_vb.push_back(cv2);
				CustomVertex cv3{ start + thick * subForward, D3DXVECTOR3(0.0f, 1.0f, 0.0f), 0xffffffff, D3DXVECTOR2() };
				sk_vb.push_back(cv3);
				CustomVertex cv4{ start - thick * subForward, D3DXVECTOR3(0.0f, 1.0f, 0.0f), 0xffffffff, D3DXVECTOR2() };
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
			vertices = NULL;
			mSkeletonMesh->LockVertexBuffer(0, (void**)&vertices);
			memcpy(vertices, &(sk_vb[0]), sizeof(CustomVertex)* sk_vb.size());
			mSkeletonMesh->UnlockVertexBuffer();

			unsigned int* indices = NULL;
			mSkeletonMesh->LockIndexBuffer(0, (void**)&indices);
			memcpy(indices, &(sk_ib[0]), sizeof(unsigned int)* sk_ib.size());
			mSkeletonMesh->UnlockVertexBuffer();
		}

		mSkeletonMesh->DrawSubset(0);
	}

	mLastTime = curTime;
}

void Test::OnQuit()
{
	if (mSkin)
	{
		delete mSkin;
		mSkin = NULL;
	}
	FBXHelper::EndFBXHelper();
}