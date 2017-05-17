#ifndef __FBX_COMMON_H__
#define __FBX_COMMON_H__

#include <fbxsdk.h>

namespace FBXHelper
{
	void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene);
	void DestroySdkObjects(FbxManager* pManager, bool pExitStatus);
	void CreateAndFillIOSettings(FbxManager* pManager);

	bool SaveScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename, int pFileFormat = -1, bool pEmbedMedia = false);
	bool LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename);
}

#endif