#pragma once
#include "ndpch.h"

class VertexBufferLayout;
class MeshData;

typedef int ColliFlags;
enum ColliFlags_
{
	ColliFlags_None = 0,
	ColliFlags_NoNormals = 1 << 0,
	ColliFlags_BuildTangent = 1 << 1,
	
};

namespace Colli
{

	MeshData* buildMesh(const std::string& path,float scale=1, ColliFlags flags=0,VertexBufferLayout* targetLayout=nullptr);
	
};
