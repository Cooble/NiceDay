#define ND_TEST
#include "App.h"
#include "imgui.h"
#include "graphics/font/FontParser.h"
#include "graphics/font/TextBuilder.h"
#include "graphics/API/Texture.h"
#include "graphics/API/Shader.h"
#include "graphics/API/VertexArray.h"
#include "graphics/GContext.h"

class TestLayer:public Layer
{
private:
	Texture* fontTexture;
	TextMesh textMesh= TextMesh(200);
	Shader* fontShader;
	Font font;
	VertexArray* fontVAO;
	VertexBuffer* fontVBO;
public:
	TestLayer() = default;
	
	inline void onAttach() override
	{
		
		ND_INFO("TestLayer attached");

		FontParser::parse(font,"arial_distance_field.fnt");

		TextBuilder::buildMesh("Wwi\nihesKarelbgngb\nllo this is \njust very interest",
			font, textMesh, 1000);
		
		fontTexture = Texture::create(TextureInfo(font.texturePath));
		fontShader = new Shader("res/shaders/Font.shader");
		glm::mat4 trans = glm::scale(glm::mat4(1.f), glm::vec3(4*1.f/App::get().getWindow()->getWidth(), 4*1.f/App::get().getWindow()->getHeight(), 1.f));
		fontShader->bind();
		fontShader->setUniformMat4("u_transform", trans);

		fontVBO = VertexBuffer::create(textMesh.getSrc(), textMesh.getByteSize());
		VertexBufferLayout layout;
		layout.push<float>(2);
		layout.push<float>(2);
		fontVBO->setLayout(layout);
		fontVAO = VertexArray::create();
		fontVAO->addBuffer(*fontVBO);
		
		
		ND_INFO("font loaded");
	}
	void onRender() override
	{
		Gcon.enableBlend();
		Gcon.setBlendEquation(BlendEquation::FUNC_ADD);
		Gcon.setBlendFunc(Blend::SRC_ALPHA, Blend::ONE_MINUS_SRC_ALPHA);
		fontShader->bind();
		fontVAO->bind();
		fontTexture->bind(0);
		Gcon.cmdDrawArrays(Topology::TRIANGLES, textMesh.getVertexCount());
	}
	void onUpdate() override
	{
	}
	void onImGuiRender() override
	{
		
		if(ImGui::Begin("Karel"))
		{
			ImGui::Text("Sup dawg");
		}
		ImGui::End();
	}
};
class TestApp:public App
{
public:
	TestApp()
	{
		m_LayerStack.PushLayer(new TestLayer());
	}
	
};
#ifdef ND_TEST
int main()
{
	TestApp t;

	t.start();
	ND_INFO("EXxit test");
	ND_WAIT_FOR_INPUT;
	return 0;
	
}
#endif