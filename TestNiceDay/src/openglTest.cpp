#include "ndpch.h"
#include "NDTests.h"
#include <random>

#include "core/Window.h"
#include "graphics/API/FrameBuffer.h"
#include "graphics/API/Buffer.h"
#include "graphics/API/Shader.h"
#include "graphics/API/VertexArray.h"
#include "graphics/API/Texture.h"


int openglTest()
{
	using namespace nd;

	auto window = new Window(1280, 720, "Niceday Test");
	GContext::init(Renderer::getAPI());

	auto fbo = FrameBuffer::create(FrameBufferInfo(1280,720,TextureFormat::RGB).multiSample(4));
	float f[]
	{	//this is counterclockwise
		//   x,y   u,v
		-1, 1, 0, 1,
		-1, -1, 0, 0,
		1, 1, 1, 1,
		1, -1, 1, 0,
	};
	auto vbo = VertexBuffer::create(f, sizeof(f));
	VertexBufferLayout layout{
		g_typ::VEC2, //pos
		g_typ::VEC2, //uv
	};
	vbo->setLayout(layout);
	auto vao = VertexArray::create();
	vao->addBuffer(*vbo);

	auto texture = Texture::create(TextureInfo().size(128));
	std::string vertex = R"(
		#version 330 core
		layout(location = 0) in vec2 a_pos;
		layout(location = 1) in vec2 a_uv;
		out vec2 uv;
		void main()
		{
			uv = a_uv;
			gl_Position = vec4(a_pos, 0.0, 1.0);
		}
	)";
	std::string fragment = R"(
		#version 330 core
		in vec2 uv;
		out vec4 color;
		uniform sampler2D u_texture;
		void main()
		{
			color = texture(u_texture, uv);
		}
	)";

	{
		auto shader = Shader::create(Shader::ShaderProgramSources(vertex, fragment));


		shader->bind();
		vao->bind();
		texture->bind(0);
		fbo->bind();
		Gcon.cmdDrawArrays(Topology::TRIANGLE_STRIP, 4);


		// cannot draw into window fbo, since App must be initialized for that to work (FBO.bind() calls App::get().getWindowSize())
		//auto windowFbo = window->getFBO();
		//windowFbo->bind();

		//fbo->getAttachment()->bind(0);
		//Gcon.cmdDrawArrays(Topology::TRIANGLE_STRIP, 4);


		// wait for some reason
		std::this_thread::sleep_for(std::chrono::seconds(1));

		delete vao;
		delete vbo;
		delete fbo;
	}

	delete window;


	NDT_ASSERT(true);

	return 0;
}
