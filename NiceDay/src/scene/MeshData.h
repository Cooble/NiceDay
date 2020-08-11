#pragma once
#include "ndpch.h"
#include "graphics/API/Buffer.h"

class MeshData
{
private:
	char* m_vertices=nullptr;
	char* m_indices=nullptr;

	size_t m_vertex_size;
	size_t m_vertex_count;
	size_t m_index_count;
	
	size_t m_vertex_count_max=0;
	size_t m_index_count_max=0;

	Strid m_id=0;
	std::string m_name;
	VertexBufferLayout m_layout;
	
public:
	struct AABB
	{
		glm::vec3 min{0.f};
		glm::vec3 max{0.f};

		glm::vec3 getDim() { return max - min; }
		glm::vec3 getCenter() { return (max + min) / 2.f; }
	};
private:
	AABB m_aabb;
public:
	Topology m_topology=Topology::TRIANGLES;
	MeshData(size_t maxVertexCount, size_t vertexSize, size_t maxIndexCount, const VertexBufferLayout& layout);
	MeshData();
	~MeshData();

	void setID(const char* id) { m_id = SID(id); m_name = id; }
	void setAABB(AABB box) { m_aabb = box; }
	const auto& getAABB()const { return m_aabb; }
	// clears all data
	void allocate(size_t maxVertexCount, size_t vertexSize, size_t maxIndexCount, const VertexBufferLayout& layout);
	
	template <typename T = char>
	T* vertex(size_t index)
	{
		ASSERT(index < m_vertex_count_max, "Mesh too small");
		return (T*)&m_vertices[index*sizeof(T)];
	}

	uint32_t* index(size_t index)
	{
		ASSERT(index < m_index_count_max, "Mesh too small");
		return (uint32_t*)&m_indices[index];
	}

	char* getIndices() { return m_indices; }
	char* getVertices() { return m_vertices; }
	
	bool isAllocated() const { return m_vertices; }
	
	size_t getIndicesCount() const { return m_index_count; }
	size_t getVerticesCount() const { return m_vertex_count; }

	size_t getIndicesCountMax() const { return m_index_count_max; }
	size_t getVerticesCountMax() const { return m_vertex_count_max; }

	size_t getIndicesSize() const { return m_index_count*sizeof(uint32_t); }
	size_t getVerticesSize() const { return m_vertex_count * m_vertex_size; }
	size_t getOneVertexSize() const { return m_vertex_size; }
	const VertexBufferLayout& getLayout() const { return m_layout; }
	void setCounts(size_t vertexCount,size_t indexCount)
	{
		ASSERT(vertexCount <= m_vertex_count_max && indexCount <= m_index_count_max, "Mesh is too small");
		m_vertex_count = vertexCount;
		m_index_count = indexCount;
	}
	Topology getTopology() const { return m_topology; }
	Strid getID() { return m_id; }
	const std::string& getName() const { return m_name; }
};
namespace MeshDataFactory
{
	// builds mesh from obj file at path, (will contain vertices and indices)
	// usePosNormUv whether will the layout be forced to pos,norm,uv or make it smaller if obj is not full
	MeshData* buildFromObj(const std::string& path, bool usePosNormUv=true);

	MeshData* buildWirePlane(int x, int z);
	MeshData* buildCube(float scale = 1);

	void writeBinaryFile(const std::string& filePath, MeshData& mesh);
	MeshData* readBinaryFile(const std::string& filePath);
};
