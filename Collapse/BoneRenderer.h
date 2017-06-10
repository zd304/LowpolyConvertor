#ifndef __BONE_RENDERER_H__
#define __BONE_RENDERER_H__

#include "inc.h"

class BoneRenderer
{
public:
	BoneRenderer(IDirect3DDevice9* device);
	~BoneRenderer();

	void BuildMesh();

	void Render();
public:
	ID3DXMesh* mMesh;
	IDirect3DDevice9* mDevice;
};

#endif // !__BONE_RENDERER_H__
