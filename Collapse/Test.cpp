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
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4.0f, 800.0f / 600.0f, 0.1f, 1000.0f);
	D3DXMatrixLookAtLH(&matView, &D3DXVECTOR3(0.0f, 400.0f, -400.0f),
		&D3DXVECTOR3(0.0f, 0.0f, 400.0f), &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	D3DXMatrixTranslation(&mMatWorld, 0.0f, -0.1f, 0.0f);

	device->SetTransform(D3DTS_PROJECTION, &matProj);
	device->SetTransform(D3DTS_VIEW, &matView);
	device->SetTransform(D3DTS_WORLD, &mMatWorld);
	device->SetRenderState(D3DRS_LIGHTING, TRUE);
	//device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
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
	if (modelList && modelList->mSkins.Count() > 0)
	{
		FBXHelper::FbxBoneMap* bonemap = FBXHelper::GetBoneMap();
		FBXHelper::FbxSkinInfo* skin = modelList->mSkins[0];
		for (unsigned int i = 0; i < skin->size; ++i)
		{
			FBXHelper::FbxBoneWeight* bw = &skin->weights[i];
			for (int j = 0; j < bw->boneName.Count(); ++j)
			{
				std::string& sName = bw->boneName[j];
				FBXHelper::FbxBoneMap::IT_BM itbm = bonemap->mBones.find(sName);
				if (itbm == bonemap->mBones.end())
				{
					continue;
				}
				FocuseBoneSkin::IT_FBS itfbs = mSkin->skins.find(itbm->second);
				if (itfbs == mSkin->skins.end())
				{
					mSkin->skins[itbm->second] = new FocusBoneWeight();
				}
				FocusBoneWeight* fbw = mSkin->skins[itbm->second];
				fbw->index.Add(i);
				fbw->weight.Add(bw->weight[j]);
			}
		}
	}
}

void Test::OnUpdate()
{
	DWORD curTime = timeGetTime();
	DWORD timeDelta = curTime - mLastTime;

	//D3DXMATRIX matRot;
	//D3DXMatrixRotationY(&matRot, 0.0001f);
	//D3DXMatrixMultiply(&mMatWorld, &mMatWorld, &matRot);
	mDevice->SetTransform(D3DTS_WORLD, &mMatWorld);

	CustomVertex* vertices = NULL;
	mMesh->LockVertexBuffer(0, (void**)&vertices);

	for (int i = 0; i < v_count; ++i)
	{
		vertices[i].pos = pvb[i].pos;
	}

	FBXHelper::FbxBoneMap* bonemap = FBXHelper::GetBoneMap();
	FBXHelper::FbxAnimationEvaluator* animEvaluator = FBXHelper::GetAnimationEvaluator();
	if (bonemap && animEvaluator)
	{
		float dt = (float)timeDelta * 0.001f;
		mAnimTime += dt;
		if (mAnimTime > 3.0f)
			mAnimTime = 0.0f;
		FBXHelper::FbxBoneMap::IT_BM it;
		for (it = bonemap->mBones.begin(); it != bonemap->mBones.end(); ++it)
		{
			FBXHelper::FbxBone* bone = it->second;
			bone->offset = animEvaluator->Evaluator(bone, "shot", mAnimTime);
			//if (bone->parent)
			//{
			//	D3DXMatrixMultiply(&bone->offset, &bone->parent->offset, &bone->offset);
			//}
			FocusBoneWeight* fbw = mSkin->skins[bone];
			if (!fbw) continue;

			D3DXMATRIX mat;
			D3DXMatrixInverse(&mat, NULL, &bone->bindPose);
			D3DXMatrixMultiply(&mat, &bone->offset, &mat);

			for (int i = 0; i < fbw->index.Count(); ++i)
			{
				int idx = fbw->index[i];
				double weight = fbw->weight[i];
				CustomVertex& cv = vertices[idx];
				D3DXVECTOR4 vec;
				D3DXVec3Transform(&vec, &cv.pos, &mat);
				vec = vec * (float)weight;
				cv.pos.x += vec.x;
				cv.pos.y += vec.y;
				cv.pos.z += vec.z;
			}
		}
	}

	mMesh->UnlockVertexBuffer();

	mDevice->SetFVF(fvf);

	mMesh->DrawSubset(0);

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