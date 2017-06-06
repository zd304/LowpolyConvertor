#ifndef __TEST_H__
#define __TEST_H__

#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#include <string>
#include "FBXHelper.h"

class ProgressiveMeshRenderer;

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
	D3DMATERIAL9 material;
	D3DXMATRIX mMatWorld;
	float rot;
	DWORD mLastTime;

	ProgressiveMeshRenderer* mMeshRenderer;
};

#endif