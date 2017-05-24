#include "Test.h"
#include "Collapse.h"
#include "FBXHelper.h"
#include <fstream>

Test::Test()
{

}

Test::~Test()
{

}

void Test::OnInit(HWND hwnd, IDirect3DDevice9* device)
{
	mDevice = device;
	mHwnd = hwnd;

	/*std::ifstream file;
	file.open("bunny_vb.txt", std::ios::binary);
	const int LINE_LENGTH = 512;
	char line[LINE_LENGTH];
	memset(line, 0, LINE_LENGTH);
	while (file.getline(line, LINE_LENGTH))
	{
		std::string sLine = line;
		memset(line, 0, LINE_LENGTH);

		float v[3];
		int index = 0;
		bool ns = false;

		std::string element;
		for (size_t i = 0; i < sLine.size(); ++i)
		{
			char c = sLine[i];
			if (c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != ',')
			{
				element += c;
				ns = true;
			}
			else
			{
				if (!ns) continue;
				ns = false;
				float e = (float)atof(element.c_str());
				element.clear();
				v[index++] = e;
				if (index >= 3)
					break;
			}
		}
		D3DXVECTOR3 vec(v);
		CustomVertex vertex;
		vertex.pos = v;
		vertex.color = 0xff00ff00;
		mVertices.push_back(vertex);
	}
	file.close();

	std::vector<unsigned int> ids;
	file.open("bunny_ib.txt", std::ios::binary);
	memset(line, 0, LINE_LENGTH);
	while (file.getline(line, LINE_LENGTH))
	{
		std::string sLine = line;
		memset(line, 0, LINE_LENGTH);

		int v[3];
		int index = 0;
		bool ns = false;

		std::string element;
		for (size_t i = 0; i < sLine.size(); ++i)
		{
			char c = sLine[i];
			if (c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != ',')
			{
				element += c;
				ns = true;
			}
			else
			{
				if (!ns) continue;
				ns = false;
				int e = atoi(element.c_str());
				element.clear();
				v[index++] = e;
				if (index >= 3)
					break;
			}
		}
		ids.push_back((unsigned int)v[0]);
		ids.push_back((unsigned int)v[1]);
		ids.push_back((unsigned int)v[2]);
	}
	file.close();*/

	FBXHelper::BeginFBXHelper("humanoid.fbx");

	CustomVertex* pvb = NULL;
	int v_stride = 0;
	int v_count = 0;
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

	D3DXComputeNormals(mMesh, 0);

	D3DXMATRIX matView, matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4.0f, 800.0f / 600.0f, 0.1f, 1000.0f);
	D3DXMatrixLookAtLH(&matView, &D3DXVECTOR3(0.0f, 200.0f, -200.0f),
		&D3DXVECTOR3(0.0f, 0.0f, 200.0f), &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
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
}

void Test::OnUpdate()
{
	D3DXMATRIX matRot;
	D3DXMatrixRotationY(&matRot, 0.0001f);
	D3DXMatrixMultiply(&mMatWorld, &mMatWorld, &matRot);
	mDevice->SetTransform(D3DTS_WORLD, &mMatWorld);

	mDevice->SetFVF(fvf);

	mMesh->DrawSubset(0);
}

void Test::OnQuit()
{
	FBXHelper::EndFBXHelper();
}