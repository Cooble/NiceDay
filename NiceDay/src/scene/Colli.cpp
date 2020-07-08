#include "Colli.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include "Mesh.h"
namespace Colli
{
	Mesh* buildMesh(const std::string& path,float scale)
	{
		Assimp::Importer importer;

		// And have it read the given file with some example postprocessing
		// Usually - if speed is not the most important aspect for you - you'll
		// probably to request more postprocessing than we do in this example.
	//	importer.re
		auto scene = importer.ReadFile(path,
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType| aiProcess_GlobalScale);

		
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
		layout.push<float>(3);
		if (m->mNormals)
			layout.push<float>(3);
		if (m->mTextureCoords[0])
			layout.push<float>(2);
		Mesh* mesh = new Mesh;
		mesh->allocate(m->mNumVertices, layout.getStride(),
			m->mNumFaces * 3, layout);
		auto vertices = (float*)mesh->getVertices();
		for (int vIdx = 0; vIdx < m->mNumVertices; ++vIdx)
		{
			*vertices++ = m->mVertices[vIdx].x*scale;
			*vertices++ = m->mVertices[vIdx].y*scale;
			*vertices++ = m->mVertices[vIdx].z*scale;

			if (m->mNormals)
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
		}
		auto indices = (uint32_t*)mesh->getIndices();
		for (int i = 0; i < mesh->getIndicesCount() / 3; ++i)
		{
			*indices++ = m->mFaces[i].mIndices[0];
			*indices++ = m->mFaces[i].mIndices[1];
			*indices++ = m->mFaces[i].mIndices[2];
		}
		// We're done. Everything will be cleaned up by the importer destructor
		//return true;
		return mesh;
	}
}
