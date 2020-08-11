#pragma once
#include "graphics/API/Buffer.h"
#include <ndpch.h>
#include <core/sids.h>
#include "graphics/GContext.h"
#include "MeshData.h"

class MeshData;



struct VertexDeclaration
{
	struct VDElement
	{
		int index = -1;
		g_typ type;
		VertexType vertexType;
		constexpr bool exists() const { return index != -1; }

	};
	std::vector<VDElement> elements;

	void addElement(int index,g_typ type,VertexType vertexType)
	{
		if (elements.size() <= index)
			elements.resize(index + 1);
		elements[index] = { index,type,vertexType };
	}
};
struct VertexBufferBinding
{
	struct VBBElement
	{
		int index = -1;
		VertexBuffer* buffer;
		constexpr bool exists() const { return index != -1; }
	};
	std::vector<VBBElement> bindings;
	void setBinding(int index, VertexBuffer* b)
	{
		if (bindings.size() <= index)
			bindings.resize(index + 1);
		bindings[index] = { index,b};
		//todo fix this, this should not be here
		count = b->getSize() / b->getLayout().getStride();
	}
	size_t count = 0;
};
struct VertexData
{
	VertexDeclaration declaration;
	VertexBufferBinding binding;
};
struct IndexData
{
	IndexBuffer* indexBuffer=nullptr;
	size_t offset;
	size_t count=0;
	constexpr bool exists() const { return indexBuffer; }
};
// graphical wrapper around meshdata
class Mesh
{
public:
	VertexData vertexData;
	IndexData indexData;
	//this will not be here
	VertexArray* vao_temp;
	
	MeshData* data;

	Strid getID() const { return data->getID(); }
	const std::string& getName() const { return data->getName(); }

};
typedef Ref<Mesh> MeshPtr;
namespace MeshFactory
{
	static VertexArray* buildVAO(Mesh* mesh);
	MeshPtr buildNewMesh(MeshData* data);

	MeshPtr loadOrGet(const std::string& filePath);
	std::unordered_map<Strid, MeshPtr>& getList();


	MeshPtr& get(Strid id);
	inline void remove(Strid id) { getList().erase(getList().find(id)); }
}
