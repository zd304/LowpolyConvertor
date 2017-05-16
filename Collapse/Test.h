#ifndef __TEST_H__
#define __TEST_H__

#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#include <string>

struct CustomVertex
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 normal;
	DWORD color;
};

class Test
{
public:
	Test();
	~Test();

	void OnInit(HWND hwnd, IDirect3DDevice9* device);

	void OnUpdate();
public:
	IDirect3DDevice9* mDevice;
	HWND mHwnd;
	std::vector<CustomVertex> mVertices;
	ID3DXMesh* mMesh;
	IDirect3DVertexBuffer9* mVB;
	DWORD fvf;
	D3DMATERIAL9 material;
	D3DXMATRIX mMatWorld;
	float rot;
};

#endif