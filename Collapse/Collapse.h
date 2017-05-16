#ifndef __COLLAPSE_H__
#define __COLLAPSE_H__

#include <Windows.h>
#include <list>
#include <vector>
#include <map>

namespace Collapse
{
	class Buffer
	{
	public:
		Buffer();
		~Buffer();
	public:
		char* vertices;
		size_t v_stride;
		size_t v_count;
		char* indices;
		size_t i_stride;
		size_t i_count;
	};

	void BeginCollapse(void* vertices, size_t v_stride, size_t v_count, size_t posOffset, void* indices, size_t i_stride, size_t i_count);

	void DoCollapse(int desired);

	Buffer* GetBuffer();

	void EndCollapse();
};

#endif
