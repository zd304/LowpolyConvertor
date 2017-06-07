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
	int mWidth;
	int mHeight;
	D3DMATERIAL9 material;
	D3DXMATRIX mMatWorld;
	float rot;
	DWORD mLastTime;

	ProgressiveMeshRenderer* mMeshRenderer;

	int mImGuiID;
	List<int> mDisireVtxNums;
	List<int> mMaxDisireVtxNums;
};

#endif