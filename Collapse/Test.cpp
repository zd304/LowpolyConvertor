#include "Test.h"
#include <fstream>
#include "ProgressiveMeshRenderer.h"

Test::Test()
{
	mLastTime = 0;
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

	FBXHelper::BeginFBXHelper("humanoid.fbx");

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

	mMeshRenderer = new ProgressiveMeshRenderer(mDevice);
	int ffff[] = {1000};
	mMeshRenderer->Collapse(ffff, 1);
}

void Test::OnUpdate()
{
	DWORD curTime = timeGetTime();
	DWORD timeDelta = curTime - mLastTime;
	float dt = (float)timeDelta * 0.001f;

	D3DXMATRIX matRot;
	D3DXMatrixRotationY(&matRot, dt);
	D3DXMatrixMultiply(&mMatWorld, &mMatWorld, &matRot);
	mDevice->SetTransform(D3DTS_WORLD, &mMatWorld);

	mMeshRenderer->Render();
	mLastTime = curTime;
}

void Test::OnQuit()
{
	if (mMeshRenderer)
		delete mMeshRenderer;
	FBXHelper::EndFBXHelper();
}