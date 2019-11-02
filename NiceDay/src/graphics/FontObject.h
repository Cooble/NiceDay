#pragma once
#include "API/Shader.h"
#include "API/VertexArray.h"
#include "API/Texture.h"
#include "font/FontParser.h"
#include "font/TextBuilder.h"

typedef int FontResourceID;
struct FontResource
{
	FontResourceID id;
	Shader* shader;
	Texture* texture;
	Font* font;
};
class FontObject
{
private:
	TextMesh m_textMesh;
	VertexArray* m_vao;
	VertexBuffer* m_vbo;
public:

	FontObject(int size);
	inline auto& getTextMesh() { return m_textMesh; }

	void updateMesh();
	void bind();
	void unbind();
};
