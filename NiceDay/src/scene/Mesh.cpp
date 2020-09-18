#include "Mesh.h"
#include "MeshData.h"
#include "graphics/API/VertexArray.h"
#include "scene/Colli.h"

namespace MeshLibrary
{
	VertexArray* buildVAO(Mesh* mesh)
	{
		auto vao = VertexArray::create();
		vao->addBuffer(*mesh->vertexData.binding.bindings[0].buffer);
		if(mesh->indexData.exists())
			vao->addBuffer(*mesh->indexData.indexBuffer);
		return vao;
	}

	MeshPtr buildNewMesh(MeshData* data)
	{
		auto mesh = MakeRef<Mesh>();
		mesh->data = data;
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
			mesh->vertexData.declaration.addElement(index, e.typ, VertexType::POS);
			mesh->vertexData.binding.setBinding(index++, vbo);
		}
		mesh->vao_temp = buildVAO(mesh.get());
		return mesh;
	}

	static std::unordered_map<Strid, MeshPtr> s_meshes;

	MeshPtr loadOrGet(const std::string& filePath)
	{
		auto it = s_meshes.find(SID(filePath));
		if (it != s_meshes.end())
			return it->second;
		auto t = buildNewMesh(Colli::buildMesh(filePath));
		s_meshes[SID(filePath)]=t;
		return t;
	}

	MeshPtr registerMesh(MeshData* meshData)
	{
		auto t = buildNewMesh(meshData);
		s_meshes[t->getID()] = t;
		return t;
	}

	std::unordered_map<Strid, MeshPtr>& getList()
	{
		return s_meshes;
	}
	static MeshPtr nullmesh = nullptr;
	MeshPtr& get(Strid id)
	{
		auto it = s_meshes.find(id);
		if (it == s_meshes.end())
			return nullmesh;
		return it->second;
	}
}
