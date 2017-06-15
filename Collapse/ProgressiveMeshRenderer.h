#ifndef __PROGRESSIVE_MESH_RENDERER_H__
#define __PROGRESSIVE_MESH_RENDERER_H__

#include "inc.h"

struct PMeshVertex_Tmp
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 normal;
	unsigned int color;
	D3DXVECTOR2 uv;
	void* skin = NULL;
};

struct PMeshModel
{
	PMeshVertex_Tmp* pVB = NULL;
	int nVertexCount = 0;
	unsigned int* pIB = NULL;
	int nIndexCount = 0;
};

struct PMeshWeight
{
	List<std::string> boneName;
	List<double> weight;
};

struct BindVertexBuffer
{
	void* vb;
	int vcount;
	void* ib;
	int icount;
};

struct FocuseBoneSkin_t;

class ProgressiveMeshRenderer
{
public:
	ProgressiveMeshRenderer(IDirect3DDevice9* device);
	~ProgressiveMeshRenderer();

	void Collapse(int* facenums, int meshCount, bool seperation = false);
	void Render();
	void SaveFile(const char* filename);
private:
	void Clear();
	void ModifyMesh(void* pNode);
public:
	// ԭʼģ������;
	List<PMeshModel*> mModels;
	// ��Ⱦ�õ��豸�������;
	List<ID3DXMesh*> mMeshes;
	// BindPose��ģ�Ͷ�������;
	List<BindVertexBuffer> mBindVertexBuffer;
	IDirect3DDevice9* mDevice;
	bool mIsSkinnedMesh;
	// ��ע�ڹ�������Ƥ��Ϣ������CPU��Ƥ��;
	List<FocuseBoneSkin_t*> mFBSkin;
private:
	int mCurMeshIndex;
};

#endif