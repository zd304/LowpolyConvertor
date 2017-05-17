#ifndef __FBX_HELPER_H__
#define __FBX_HELPER_H__

#include "FBXCommon.h"

namespace FBXHelper
{
	bool BeginFBXHelper(const char* fileName);

	void GetMesh(void** ppVB, int& v_stride, int& v_count, void** ppIB, int& i_stride, int& i_count);

	bool EndFBXHelper();
}

#endif
