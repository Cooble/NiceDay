#include "Colli.h"
#include <files/FUtil.h>     
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include "MeshData.h"
namespace Colli
{
	static MeshData* loadWithAssimp(const std::string& path, float scale, ColliFlags flags, VertexBufferLayout* targetLayout)
	{
		FUTIL_ASSERT_EXIST(path);

		Assimp::Importer importer;

		// And have it read the given file with some example postprocessing
		// Usually - if speed is not the most important aspect for you - you'll
		// probably to request more postprocessing than we do in this example.
	//	importer.re
		auto scene = importer.ReadFile(ND_RESLOC(path),
			aiProcess_Triangulate |
			aiProcess_GenBoundingBoxes |
			aiProcess_JoinIdenticalVertices |
			(flags & ColliFlags_BuildTangent ? aiProcess_CalcTangentSpace : 0) |
			aiProcess_SortByPType | aiProcess_GlobalScale);


		/*aiSetImportPropertyFloat(aiprops, AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, GlobalScale);
		flags |= aiProcess_GlobalScale;
		 aiReleaseImport( scene);
		scene_ = aiImportFileExWithProperties(GetNativePath(inFile).CString(), flags, nullptr, aiprops);
		*/
		// If the import failed, report it
		if (!scene)
		{
			ND_ERROR(importer.GetErrorString());
			return nullptr;
		}


		// Now we can access the file's contents.
		if (scene->mNumMeshes == 0)
			return nullptr;
		auto m = scene->mMeshes[0];

		VertexBufferLayout layout;
		layout.pushElement({ g_typ::VEC3 });
		if (m->mNormals && !(flags&ColliFlags_NoNormals))
			layout.pushElement({ g_typ::VEC3 });
		if (m->mTextureCoords[0])
			layout.pushElement({ g_typ::VEC2 });
		if (m->mTangents && flags & ColliFlags_BuildTangent)
			layout.pushElement({ g_typ::VEC3 });
		MeshData* mesh = new MeshData;
		mesh->allocate(m->mNumVertices, layout.getStride(),
			m->mNumFaces * 3, layout);
		auto vertices = (float*)mesh->getVertices();
		for (int vIdx = 0; vIdx < m->mNumVertices; ++vIdx)
		{
			*vertices++ = m->mVertices[vIdx].x * scale;
			*vertices++ = m->mVertices[vIdx].y * scale;
			*vertices++ = m->mVertices[vIdx].z * scale;

			if (m->mNormals&&!(flags&ColliFlags_NoNormals))
			{
				*vertices++ = m->mNormals[vIdx].x;
				*vertices++ = m->mNormals[vIdx].y;
				*vertices++ = m->mNormals[vIdx].z;
			}
			if (m->mTextureCoords[0])
			{
				*vertices++ = m->mTextureCoords[0][vIdx].x;
				*vertices++ = m->mTextureCoords[0][vIdx].y;
			}
			if (m->mTangents && flags & ColliFlags_BuildTangent)
			{
				*vertices++ = m->mTangents[vIdx].x;
				*vertices++ = m->mTangents[vIdx].y;
				*vertices++ = m->mTangents[vIdx].z;
			}
		}
		auto indices = (uint32_t*)mesh->getIndices();
		for (int i = 0; i < mesh->getIndicesCount() / 3; ++i)
		{
			*indices++ = m->mFaces[i].mIndices[0];
			*indices++ = m->mFaces[i].mIndices[1];
			*indices++ = m->mFaces[i].mIndices[2];
		}
		// We're done. Everything will be cleaned up by the importer destructor
		mesh->m_topology = Topology::TRIANGLES;
		mesh->setID(path.c_str());
		mesh->setAABB({ glm::vec3(m->mAABB.mMin.x,m->mAABB.mMin.y,m->mAABB.mMin.z),glm::vec3(m->mAABB.mMax.x,m->mAABB.mMax.y,m->mAABB.mMax.z) });
		return mesh;
	}
	MeshData* buildMesh(const std::string& path,float scale, ColliFlags flags, VertexBufferLayout* targetLayout)
	{
		
		if(SUtil::endsWith(path,".bin"))
			return MeshDataFactory::readBinaryFile(path);
		return loadWithAssimp(path, scale, flags, targetLayout);
		
	}

}
