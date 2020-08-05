#include "NewMesh.h"
#include "MeshData.h"
#include "graphics/API/VertexArray.h"

namespace NewMeshFactory
{
	VertexArray* buildVAO(NewMesh* mesh)
	{
		auto vao = VertexArray::create();
		vao->addBuffer(*mesh->vertexData.binding.bindings[0].buffer);
		vao->addBuffer(*mesh->indexData.indexBuffer);
		return vao;
	}

	NewMesh* buildNewMesh(MeshData* data)
	{
		auto mesh = new NewMesh;
		
		mesh->topology = data->getTopology();
		if(data->getIndicesCount())
		{
			mesh->indexData.count = data->getIndicesCount();
			mesh->indexData.offset = 0;
			mesh->indexData.indexBuffer = IndexBuffer::create((uint32_t*)data->getIndices(), data->getIndicesCount());
		}

		int index = 0;
		VertexBufferLayout layout;
		auto vbo = VertexBuffer::create(data->getVertices(), data->getVerticesSize());
		vbo->setLayout(data->getLayout());
		for (auto& e:data->getLayout().getElements())
		{
			mesh->vertexData.declaration.addElement(index, GTypes::setCount(e.typ,e.count), VertexType::POS);
			mesh->vertexData.binding.setBinding(index++, vbo);
		}
		mesh->vao_temp = buildVAO(mesh);
		return mesh;
	}
}
