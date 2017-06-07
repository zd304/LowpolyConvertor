#ifndef __TEST_H__
#define __TEST_H__

#include "inc.h"
#include "FBXHelper.h"

class ProgressiveMeshRenderer;

class Test
{
public:
	Test();
	~Test();

	void OnInit(HWND hwnd, IDirect3DDevice9* device);

	void OnUpdate();

	void OnGUI();

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