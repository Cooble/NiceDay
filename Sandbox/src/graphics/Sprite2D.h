#pragma once
#include "ndpch.h"
#include "graphics/API/Shader.h"

class Sprite2D
{
private:
	static nd::VertexBuffer* s_vbo;
	static nd::VertexArray* s_vao;
	static nd::ShaderPtr s_program;

public:

	static void init();
	static nd::Shader& getProgramStatic() { return *s_program; }
	static nd::VertexArray& getVAOStatic() { return *s_vao; }

private:
	nd::Texture* m_texture;
	glm::vec2 m_scale;
	glm::vec2 m_position;
	glm::mat4 m_model_matrix;
	glm::vec4 m_cutout;
	glm::mat4 m_uv_matrix;

	bool m_stale_model_matrix;
	bool m_stale_uv_matrix;
	bool m_is_light_applied = true;
	float m_alpha = 1;
private:
	void computeModelMatrix();
	void computeUVMatrix();
public:
	Sprite2D(nd::Texture*);
	Sprite2D(const char* texture_path);
	~Sprite2D();

	void setLightApplied(bool light) { m_is_light_applied = light; }
	bool isLightApplied() { return m_is_light_applied; }
	nd::Shader& getProgram() { return *s_program; }
	nd::VertexArray& getVAO() { return *s_vao; }


	void setModelMatrix(const glm::mat4& m)
	{
		m_model_matrix = m;
		m_stale_model_matrix = false;
	}

	void setUVMatrix(const glm::mat4& m)
	{
		m_uv_matrix = m;
		m_stale_uv_matrix = false;
	}

	void setPosition(const glm::vec2&);
	void setScale(const glm::vec2&);

	//rectangle x1 y1 x2 y2
	void setCutout(const glm::vec4&);
	//reset cutout to whole texture
	void setCutout();

	void setDimensions(int pixelX, int pixelY);
	const nd::Texture& getTexture() const { return *m_texture; }
	const glm::mat4& getModelMatrix();
	const glm::mat4& getUVMatrix();
	float getAlpha() const { return m_alpha; }
	void setAlpha(float f) { m_alpha = f; }
};
