#ifndef __INC_H__
#define __INC_H__

#include <string>
#include <map>
#include <algorithm>
#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "list.h"

#define SAFE_DELETE(x) if (x) { delete x; x = NULL; }
#define SAFE_DELETE_ARRAY(x) if (x) { delete [] x; x = NULL; }
#define SAFE_RELEASE(x) if (x) { x->Release(); x = NULL; }

std::string UTS(const std::string & str);

std::string STU(const std::string & str);

#endif