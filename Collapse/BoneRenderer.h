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

	void SetBoneThick(float thick);

	float GetBoneThick();

	void SetShowAnimated(bool animated);

	bool GetShowAnimated();
private:
	ID3DXMesh* mMesh;
	IDirect3DDevice9* mDevice;
	float mThick;
	bool mShowAnimated;
};

#endif // !__BONE_RENDERER_H__
