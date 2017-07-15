#include "Collapse.h"
#include "list.h"
#include <assert.h>
#include "SkinCollapse.h"

namespace Collapse
{
	class CollapseVertex;

	class CollapseTriangle
	{
	public:
		CollapseTriangle(CollapseVertex* v0, CollapseVertex* v1, CollapseVertex* v2);
		~CollapseTriangle();

		void ComputeNormal();
		void ReplaceVertex(CollapseVertex* vold, CollapseVertex* vnew);
		int HasVertex(CollapseVertex* v);
	public:
		// ��������������;
		CollapseVertex* vertices[3];
		// �淨��;
		float normal[3];
	};

	class CollapseVertex
	{
	public:
		CollapseVertex(void* _vertex, size_t stride, size_t _offset);
		~CollapseVertex();

		void RemoveIfNonNeighbor(CollapseVertex* n);
		float* GetPosition();
	public:
		// ��������;
		char* vertex;
		// ���������е�λ����������;
		size_t offset;
		// ԭID;
		unsigned int id;
		// ���ڶ���;
		List<CollapseVertex*> neighbor;
		// ������;
		List<CollapseTriangle*> face;
		// ����ߵ�̮��ֵ;
		float cost;
		// ��ѡ̮���Ķ���;
		CollapseVertex* collapse;
	};

	List<CollapseVertex*> vertices;
	List<CollapseTriangle*> triangles;
	size_t g_v_stride = 0;
	size_t g_i_stride = 0;
	std::map<CollapseVertex*, unsigned int> vimap;

	void cross(float* v1, float* v2, float* rst)
	{
		rst[0] = v1[1] * v2[2] - v1[2] * v2[1];
		rst[1] = v1[2] * v2[0] - v1[0] * v2[2];
		rst[2] = v1[0] * v2[1] - v1[1] * v2[0];
	}

	float dot(float* v1, float* v2)
	{
		return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
	}

	float length(float* v)
	{
		return sqrtf(dot(v, v));
	}

	void normalize(float* v)
	{
		float len = length(v);
		if (len < FLT_EPSILON) len = 1;
		len = 1 / len;

		v[0] *= len;
		v[1] *= len;
		v[2] *= len;
	}

	//////////////////////////CollapseVertex//////////////////////////////////

	CollapseVertex::CollapseVertex(void* _vertex, size_t _stride, size_t _offset)
	{
		vertex = (char*)malloc(_stride);
		memcpy(vertex, _vertex, _stride);
		offset = _offset;
		cost = 0.0f;
		collapse = NULL;

		vertices.Add(this);
	}

	CollapseVertex::~CollapseVertex()
	{
		free(vertex);
		cost = 0.0f;
		collapse = NULL;

		//assert(face.num == 0);
		while (neighbor.num)
		{
			neighbor[0]->neighbor.Remove(this);
			neighbor.Remove(neighbor[0]);
		}

		neighbor.Clear();
		face.Clear();

		vertices.Remove(this);
	}

	void CollapseVertex::RemoveIfNonNeighbor(CollapseVertex* n)
	{
		if (!neighbor.Contains(n)) return;
		for (int i = 0; i < face.num; i++)
		{
			if (face[i]->HasVertex(n) >= 0) return;
		}
		neighbor.Remove(n);
	}

	float* CollapseVertex::GetPosition()
	{
		char* p = vertex;
		p += offset;
		return (float*)p;
	}
	
	//////////////////////////////////////////////////////////////////////////

	/////////////////////////CollapseTriangle/////////////////////////////////

	CollapseTriangle::CollapseTriangle(CollapseVertex* v0, CollapseVertex* v1, CollapseVertex* v2)
	{
		assert(v0 != v1 && v1 != v2 && v2 != v0);
		vertices[0] = v0;
		vertices[1] = v1;
		vertices[2] = v2;
		ComputeNormal();
		triangles.Add(this);
		for (int i = 0; i < 3; i++)
		{
			vertices[i]->face.Add(this);
			for (int j = 0; j < 3; j++)
			{
				if (i != j)
				{
					vertices[i]->neighbor.AddUnique(vertices[j]);
				}
			}
		}
	}

	CollapseTriangle::~CollapseTriangle()
	{
		int i;
		triangles.Remove(this);
		for (i = 0; i < 3; i++)
		{
			if (vertices[i])
			{
				vertices[i]->face.Remove(this);
			}
		}
		for (i = 0; i < 3; i++)
		{
			int i2 = (i + 1) % 3;
			if (!vertices[i] || !vertices[i2]) continue;
			vertices[i]->RemoveIfNonNeighbor(vertices[i2]);
			vertices[i2]->RemoveIfNonNeighbor(vertices[i]);
		}
	}

	void CollapseTriangle::ComputeNormal()
	{
		float* pos0 = vertices[0]->GetPosition();
		float* pos1 = vertices[1]->GetPosition();
		float* pos2 = vertices[2]->GetPosition();

		float edge0[3] = {
			pos1[0] - pos0[0],
			pos1[1] - pos0[1],
			pos1[2] - pos0[2],
		};
		float edge1[3] = {
			pos2[0] - pos1[0],
			pos2[1] - pos1[1],
			pos2[2] - pos1[2],
		};
		float* n = new float[3];
		cross(edge0, edge1, n);
		normalize(n);
		normal[0] = n[0];
		normal[1] = n[1];
		normal[2] = n[2];
	}

	void CollapseTriangle::ReplaceVertex(CollapseVertex* vold, CollapseVertex* vnew)
	{
		assert(vold && vnew);
		assert(vold == vertices[0] || vold == vertices[1] || vold == vertices[2]);
		assert(vnew != vertices[0] && vnew != vertices[1] && vnew != vertices[2]);
		if (vold == vertices[0])
		{
			vertices[0] = vnew;
		}
		else if (vold == vertices[1])
		{
			vertices[1] = vnew;
		}
		else
		{
			assert(vold == vertices[2]);
			vertices[2] = vnew;
		}
		int i;
		vold->face.Remove(this);
		assert(!vnew->face.Contains(this));
		vnew->face.Add(this);
		for (i = 0; i < 3; i++)
		{
			vold->RemoveIfNonNeighbor(vertices[i]);
			vertices[i]->RemoveIfNonNeighbor(vold);
		}
		for (i = 0; i < 3; i++)
		{
			assert(vertices[i]->face.Contains(this) == 1);
			for (int j = 0; j < 3; j++)
			{
				if (i != j)
				{
					vertices[i]->neighbor.AddUnique(vertices[j]);
				}
			}
		}
		ComputeNormal();
	}

	int CollapseTriangle::HasVertex(CollapseVertex* v)
	{
		for (size_t i = 0; i < 3; ++i)
		{
			CollapseVertex* v1 = vertices[i];
			if (v == v1)
				return i;
		}
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////

	Buffer::Buffer()
	{
		v_count = Collapse::vertices.num;
		v_stride = g_v_stride;
		size_t vsize = v_stride * v_count;
		vertices = (char*)malloc(vsize);
		char* p = vertices;
		for (int i = 0; i < Collapse::vertices.num; ++i)
		{
			CollapseVertex* v = Collapse::vertices[i];
			memcpy(p, v->vertex, v_stride);
			p += v_stride;
		}
		
		i_count = Collapse::triangles.num;
		i_count *= 3;
		i_stride = g_i_stride;
		size_t isize = i_stride * i_count;
		indices = (char*)malloc(isize);
		p = indices;
		std::list<CollapseTriangle*>::iterator it;
		for (int i = 0; i < triangles.num; ++i)
		{
			CollapseTriangle* tri = triangles[i];
			unsigned int index0 = vimap[tri->vertices[0]];
			unsigned int index1 = vimap[tri->vertices[1]];
			unsigned int index2 = vimap[tri->vertices[2]];
			memcpy(p, &index0, i_stride);
			p += i_stride;
			memcpy(p, &index1, i_stride);
			p += i_stride;
			memcpy(p, &index2, i_stride);
			p += i_stride;
		}
	}

	Buffer::~Buffer()
	{
		free(vertices);
		free(indices);
	}

	Buffer* buffer = NULL;

	//////////////////////////////////////////////////////////////////////////

	void ComputeEdgeCostAtVertex(CollapseVertex* v);

	void BeginCollapse(void* _vertices, size_t v_stride, size_t v_count, size_t posOffset, void* indices, size_t i_stride, size_t i_count)
	{
		if (vertices.num != 0)
		{
			return;
		}
		g_v_stride = v_stride;
		g_i_stride = i_stride;
		vimap.clear();
		char* p = (char*)_vertices;
		for (size_t i = 0; i < v_count; ++i)
		{
			CollapseVertex* pv = new CollapseVertex(p, v_stride, posOffset);
			pv->id = i;
			p += v_stride;
		}
		p = (char*)indices;
		for (size_t i = 0; i + 2 < i_count; i += 3)
		{
			CollapseVertex* vs[3];
			for (size_t j = 0; j < 3; ++j)
			{
				unsigned int index = *(unsigned int*)p;
				p += i_stride;

				vs[j] = vertices[index];
			}
			
			CollapseTriangle* triangle = new CollapseTriangle(vs[0], vs[1], vs[2]);
		}

		for (int i = 0; i < vertices.num; i++)
		{
			ComputeEdgeCostAtVertex(vertices[i]);
		}
	}

	float ComputeEdgeCollapseCost(CollapseVertex* u, CollapseVertex* v)
	{
		// ���Ҫ��u��v̮���ߣ�ģ�ͽ����ж��ٸı�;
		float* vpos = v->GetPosition();
		float* upos = u->GetPosition();
		float evec[3] = {
			vpos[0] - upos[0],
			vpos[1] - upos[1],
			vpos[2] - upos[2]
		};
		float edgelength = length(evec);
		float curvature = 0;
		// �ҳ�������uvΪ�ߵ�������;
		List<CollapseTriangle*> sides;
		for (int i = 0; i < u->face.num; ++i)
		{
			CollapseTriangle* tri = u->face[i];
			if (tri->HasVertex(v) >= 0)
			{
				sides.Add(tri);
			}
		}
		// u�������ٽ����sides�����������泯���������һ������Ϊ����ϵ��;
		for (int i = 0; i < u->face.num; i++)
		{
			CollapseTriangle* tri = u->face[i];
			float mincurv = 1;
			for (int j = 0; j < sides.num; j++)
			{
				// ���������;
				float dotprod = dot(tri->normal, sides[j]->normal);
				mincurv = min(mincurv, (1 - dotprod) / 2.0f);
			}
			curvature = max(curvature, mincurv);
		}

		float skincost = SkinCollapse::ComputeSkinCost(v->vertex, u->vertex);
		return edgelength * curvature * skincost;
	}

	void ComputeEdgeCostAtVertex(CollapseVertex* v)
	{
		if (v->neighbor.num == 0)
		{
			v->collapse = NULL;
			v->cost = -0.01f;
			return;
		}
		v->cost = FLT_MAX;
		v->collapse = NULL;
		// ���������ٽӱ߻�ȡ��С̮��ֵ;
		for (int i = 0; i < v->neighbor.num; ++i)
		{
			CollapseVertex* neighbor = v->neighbor[i];
			float c;
			c = ComputeEdgeCollapseCost(v, neighbor);
			if (c < v->cost)
			{
				v->collapse = neighbor;
				v->cost = c;
			}
		}
		if (!v->collapse)
		{
			int a = 0;
		}
	}

	void CollapseEdge(CollapseVertex* u, CollapseVertex *v)
	{
		// ��u��v��̮����uv;
		if (!v)
		{
			// ���u��̮���������㣬ֱ��ɾ��u;
			delete u;
			return;
		}
		int i;
		List<CollapseVertex*> tmp;
		// ��u������neighbor����ֵ��tmp;
		for (i = 0; i < u->neighbor.num; i++)
		{
			tmp.Add(u->neighbor[i]);
		}
		// ɾ������uv����;
		for (i = u->face.num - 1; i >= 0; i--)
		{
			if (u->face[i]->HasVertex(v) >= 0)
			{
				delete u->face[i];
			}
		}
		// ����ʣ�µ��棬��v����u;
		for (i = u->face.num - 1; i >= 0; i--)
		{
			u->face[i]->ReplaceVertex(u, v);
		}
		delete u;
		// ���¼�������neighbor��̮��ֵ;
		for (i = 0; i < tmp.num; i++)
		{
			ComputeEdgeCostAtVertex(tmp[i]);
		}
	}

	CollapseVertex* MinimumCostEdge()
	{
		if (vertices.num <= 0)
			return NULL;
		CollapseVertex* mincost = vertices[0];
		for (int i = 0; i < vertices.num; ++i)
		{
			CollapseVertex* v = vertices[i];
			if (v->cost < mincost->cost)
			{
				mincost = v;
			}
		}
		return mincost;
	}

	void DoCollapse(int desired)
	{
		vimap.clear();
		if (buffer)
		{
			delete buffer;
			buffer = NULL;
		}
		while (vertices.num > desired)
		{
			CollapseVertex* min = MinimumCostEdge();
			CollapseEdge(min, min->collapse);
		}
		for (int i = 0; i < vertices.num; ++i)
		{
			vimap[vertices[i]] = i;
		}
		buffer = new Buffer();
	}

	Buffer* GetBuffer()
	{
		return buffer;
	}

	void EndCollapse()
	{
		g_v_stride = 0;
		g_i_stride = 0;
		vimap.clear();
		for (int i = 0; i < triangles.num; ++i)
		{
			CollapseTriangle* triangle = triangles[i];
			if (triangle)
			{
				delete triangle;
			}
		}
		triangles.Clear();

		for (int i = 0; i < vertices.num; ++i)
		{
			CollapseVertex* v = vertices[i];
			if (v)
			{
				delete v;
				vertices[i] = NULL;
			}
		}
		vertices.Clear();
	}
};