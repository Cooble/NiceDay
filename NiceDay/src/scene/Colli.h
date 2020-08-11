#pragma once
#include "ndpch.h"

class MeshData;
namespace Colli
{

	MeshData* buildMesh(const std::string& path,float scale=1,bool buildTangent=false);
	
};
