#pragma once
#include "ndpch.h"//todo add glm to ndpch

#include "buffer/VertexBuffer.h"
#include "buffer/VertexArray.h"
#include "Program.h"
#include "Texture.h"

class Sprite2D
{
private:
	static VertexBuffer* s_vbo;
	static VertexArray* s_vao;
	static Program* s_program;

public:
	
	static void init();

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

	inline Program& getProgram() { return *s_program; }
	inline VertexArray& getVAO() { return *s_vao; }
	void setPosition(glm::vec2);
	void setScale(glm::vec2);

	//rectangle x1 y1 x2 y2
	void setCutout(glm::vec4);
	//reset cutout to whole texture
	void setCutout();
	
	void setDimensions(int pixelX, int pixelY);
	inline const Texture& getTexture() const { return *m_texture; }
	const glm::mat4& getModelMatrix();
	const glm::mat4& getUVMatrix();
};

