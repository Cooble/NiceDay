#pragma once
#include "graphics/API/Buffer.h"
#include <ndpch.h>
#include "graphics/GContext.h"

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
class NewMesh
{
public:
	VertexData vertexData;
	IndexData indexData;

	Topology topology;
	
	//this will not be here
	VertexArray* vao_temp;
	
};
namespace NewMeshFactory
{
	static VertexArray* buildVAO(NewMesh* mesh);
	NewMesh* buildNewMesh(MeshData* data);
}
