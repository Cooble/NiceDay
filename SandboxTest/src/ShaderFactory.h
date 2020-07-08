#pragma once

class VertexBufferLayout;
class Material;
class Shader;
namespace ShaderFactory
{
	Shader* getShader(Material* material, const VertexBufferLayout& layout);
};
