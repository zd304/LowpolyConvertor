#ifndef __TEST_H__
#define __TEST_H__

#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#include <string>
#include "FBXHelper.h"

struct CustomVertex
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 normal;
	DWORD color;
	D3DXVECTOR2 uv;
};

class FocusBoneWeight
{
public:
	List<int> index;
	List<double> weight;
};

class FocuseBoneSkin
{
public:
	FocuseBoneSkin();
	~FocuseBoneSkin();
public:
	typedef std::map<FBXHelper::FbxBone*, FocusBoneWeight*>::iterator IT_FBS;
	std::map<FBXHelper::FbxBone*, FocusBoneWeight*> skins;
};

class Test
{
public:
	Test();
	~Test();

	void OnInit(HWND hwnd, IDirect3DDevice9* device);

	void OnUpdate();

	void OnQuit();
public:
	IDirect3DDevice9* mDevice;
	HWND mHwnd;
	std::vector<CustomVertex> mVertices;
	ID3DXMesh* mMesh;
	DWORD fvf;
	D3DMATERIAL9 material;
	D3DXMATRIX mMatWorld;
	float rot;
	DWORD mLastTime;

	CustomVertex* pvb = NULL;
	int v_stride = 0;
	int v_count = 0;
	
	float mAnimTime;
	FocuseBoneSkin* mSkin;
};

#endif