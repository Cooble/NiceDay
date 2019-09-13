#pragma once
#include "ndpch.h"//todo add glm to ndpch

#include "API/Buffer.h"
#include "API/VertexArray.h"
#include "API/Shader.h"
#include "API/Texture.h"

class Sprite2D
{
private:
	static VertexBuffer* s_vbo;
	static VertexArray* s_vao;
	static Shader* s_program;

public:
	
	static void init();
	static inline Shader& getProgramStatic() { return *s_program; }
	static inline VertexArray& getVAOStatic() { return *s_vao; }

private:
	Texture* m_texture;
	glm::vec2 m_scale;
	glm::vec2 m_position;
	glm::mat4 m_model_matrix;
	glm::vec4 m_cutout;
	glm::mat4 m_uv_matrix;

	bool m_stale_model_matrix;
	bool m_stale_uv_matrix;
private:
	void computeModelMatrix();
	void computeUVMatrix();
public:
	Sprite2D(Texture*);
	Sprite2D(const char* texture_path);
	~Sprite2D();

	inline Shader& getProgram() { return *s_program; }
	inline VertexArray& getVAO() { return *s_vao; }
	inline void setModelMatrix(const glm::mat4& m)
	{
		m_model_matrix = m;
		m_stale_model_matrix = false;
	}
	inline void setUVMatrix(const glm::mat4& m)
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
	inline const Texture& getTexture() const { return *m_texture; }
	const glm::mat4& getModelMatrix();
	const glm::mat4& getUVMatrix();
};

