#include "EditorLayer.h"
#include "graphics/API/Texture.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "MeshData.h"
#include "Colli.h"
#include "graphics/API/VertexArray.h"
#include "graphics/GContext.h"
#include "NewScene.h"
#include "components.h"
#include "scene/Mesh.h"
#include "platform/OpenGL/GLShader.h"
#include "graphics/API/FrameBuffer.h"
#include "core/App.h"
#include "Camm.h"
#include "platform/OpenGL/GLRenderer.h"
#include "components_imgui_access.h"
#include "Atelier.h"
#include "files/FUtil.h"
#include "script/NativeScript.h"
#include "graphics/TextureAtlas.h"
#include "graphics/Effect.h"
#include "core/ImGuiLayer.h"

static Entity sphere0;
static Entity sphere1;
static Entity sphere2;


ShaderPtr oneColorShader;
struct Env
{
	glm::mat4 view;
	glm::mat4 proj;

	glm::vec3 sunPos;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 camera_pos;

	float constant;
	float linear;
	float quadratic;
} env;

namespace Spacer3D
{
	glm::vec3 getPointOnPlane(const glm::vec3& planeNormal, const glm::vec3& planePos, const glm::vec3& rayDir, const glm::vec3& rayPos)
	{
		float dot = glm::dot(planeNormal, rayDir);
		if (dot == 0.0)
			return glm::vec3(0, 0, 0);//todo return some kind of invalid value

		float t = -glm::dot(planeNormal, rayPos - planePos) / dot;
		return t * rayDir + rayPos;
	}
	glm::vec3 screenToRay(const glm::vec2& src, const glm::mat4 proj, const glm::mat4& view, const glm::vec2& screenResolution)
	{
		glm::vec2 s = src;
		s /= screenResolution;
		s = s * 2.f - 1.f;

		auto p = glm::inverse(proj) * glm::vec4(s, 0, 1);
		p.w = 0;//we care only about direction
		p.z = -1;//looking towards
		return  glm::normalize(glm::vec3(glm::inverse(view) * p));
	}

	/*
	 * Solves equation: A*a+B*b+C*c = D
	 * Returns vec3(a,b,c)
	 */
	glm::vec3 matrixSolver(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C, const glm::vec3& D)
	{
		glm::vec4 X(A.x, B.x, C.x, D.x);
		glm::vec4 Y(A.y, B.y, C.y, D.y);
		glm::vec4 Z(A.z, B.z, C.z, D.z);

		Z -= Z.x / Y.x * Y;
		Y -= Y.x / X.x * X;
		Z -= Z.y / Y.y * Y;
		X -= X.z / Y.z * Y;
		Y -= Y.z / Z.z * Z;
		X -= X.y / Y.y * Y;

		glm::vec3 ret = { X.w / X.x, Y.w / Y.y, Z.w / Z.z };
		return ret;
	}

	glm::vec3 getClosestPointOnLine(
		const glm::vec3& lineDir, const glm::vec3& linePos,
		const glm::vec3& secondDir, const glm::vec3& secondPos)
	{
		auto abc = matrixSolver(secondDir, glm::normalize(glm::cross(glm::normalize(secondDir), glm::normalize(lineDir))), -lineDir, linePos - secondPos);
		/*{
			auto& tra = sphere0.get<TransformComponent>();
			tra.pos = secondPos + secondDir * abc.x;
			tra.recomputeMatrix();
		}
		{
			auto& tra = sphere1.get<TransformComponent>();
			tra.pos = linePos - lineDir * abc.z;
			tra.recomputeMatrix();
		}*/
		/*{
			auto& tra = line1.get<TransformComponent>();
			ND_INFO("cros {}", glm::cross(secondDir, lineDir));
			auto t = glm::normalize(glm::cross(secondDir, lineDir));
			auto r = sqrt(t.x * t.x + t.y * t.y + t.z * t.z);
			tra.rot.p = glm::atan(t.y / t.x);
			tra.rot.t = glm::acos(t.z / r);
			tra.pos = secondPos + secondDir * abc.z;
			tra.recomputeMatrix();
		}
		{
			auto& tra = line2.get<TransformComponent>();
			auto t = lineDir;
			auto r = sqrt(t.x * t.x + t.y * t.y + t.z * t.z);
			tra.rot.t = glm::atan(t.y / t.x);
			tra.rot.p = glm::acos(t.z / r);
			tra.pos = linePos + lineDir * abc.z;
			tra.recomputeMatrix();
		}*/
		return abc.z * lineDir + linePos;
	}
}





float thic = 2;
int ringSegments = 30;
static MeshData* buildRing(int segments)
{
	constexpr  float thickness = 0.05f;
	VertexBufferLayout l{
		g_typ::VEC3,
		g_typ::VEC3
	};
	auto mesh = new MeshData(segments * 6 * 3+segments*2, l.getStride(), 0, l);
	glm::vec3* buff = (glm::vec3*)mesh->getVertices();
	float stepAngle = 2 * glm::pi<float>() / segments;
	float currentAngle = 0;

	// X AXIS
	glm::vec3 color = { 1.f,0.f,0.f };
	for (int seg = 0; seg < segments; ++seg)
	{
		glm::vec3 p0 = { -thickness,glm::sin(currentAngle),glm::cos(currentAngle) };
		glm::vec3 p1 = { thickness,p0.y,p0.z };
		currentAngle += stepAngle;
		glm::vec3 p2 = { -thickness,glm::sin(currentAngle),glm::cos(currentAngle) };
		glm::vec3 p3 = { thickness,p2.y,p2.z };

		*buff++ = p0;	*buff++ = color;
		*buff++ = p1;	*buff++ = color;
		*buff++ = p3;	*buff++ = color;
		
		*buff++ = p0;	*buff++ = color;
		*buff++ = p3;	*buff++ = color;
		*buff++ = p2;	*buff++ = color;
	}
	// Y AXIS
	color = { 0.f,1.f,0.f };
	for (int seg = 0; seg < segments; ++seg)
	{
		glm::vec3 p0 = { glm::cos(currentAngle) ,-thickness,glm::sin(currentAngle) };
		glm::vec3 p1 = { p0.x, thickness,p0.z };
		currentAngle += stepAngle;
		glm::vec3 p2 = { glm::cos(currentAngle) ,-thickness,glm::sin(currentAngle) };
		glm::vec3 p3 = { p2.x, thickness,p2.z };

		*buff++ = p0;	*buff++ = color;
		*buff++ = p1;	*buff++ = color;
		*buff++ = p3;	*buff++ = color;

		*buff++ = p0;	*buff++ = color;
		*buff++ = p3;	*buff++ = color;
		*buff++ = p2;	*buff++ = color;
	}
	// Z AXIS
	color = { 0.f,0.f,1.f };
	for (int seg = 0; seg < segments; ++seg)
	{
		glm::vec3 p0 = { glm::sin(currentAngle) ,glm::cos(currentAngle) ,-thickness};
		glm::vec3 p1 = { p0.x, p0.y, thickness };
		currentAngle += stepAngle;
		glm::vec3 p2 = { glm::sin(currentAngle) ,glm::cos(currentAngle) ,-thickness };
		glm::vec3 p3 = { p2.x, p2.y, thickness };
		

		*buff++ = p0;	*buff++ = color;
		*buff++ = p1;	*buff++ = color;
		*buff++ = p3;	*buff++ = color;

		*buff++ = p0;	*buff++ = color;
		*buff++ = p3;	*buff++ = color;
		*buff++ = p2;	*buff++ = color;
	}

	// Outer ring
	/*constexpr float thick = 1.1f;
	color = { 0.5f,0.5f,0.5f };
	for (int seg = 0; seg < segments; ++seg)
	{
		glm::vec3 p0 = { glm::sin(currentAngle) ,glm::cos(currentAngle) ,0 };
		glm::vec3 p1 = { p0.x* thick, p0.y* thick, 0 };
		currentAngle += stepAngle;
		glm::vec3 p2 = { glm::sin(currentAngle) ,glm::cos(currentAngle) ,0 };
		glm::vec3 p3 = { p2.x * thick, p2.y * thick, 0 };

		*buff++ = p0;	*buff++ = color;
		*buff++ = p1;	*buff++ = color;
		*buff++ = p3;	*buff++ = color;

		*buff++ = p0;	*buff++ = color;
		*buff++ = p3;	*buff++ = color;
		*buff++ = p2;	*buff++ = color;
	}*/
	color = { 0.5f,0.5f,0.5f };
	for (int seg = 0; seg < segments; ++seg)
	{
		glm::vec3 p0 = { glm::sin(currentAngle) ,glm::cos(currentAngle) ,0 };
		currentAngle += stepAngle;
		glm::vec3 p1 = { glm::sin(currentAngle) ,glm::cos(currentAngle) ,0 };


		*buff++ = p0;	*buff++ = color;
		*buff++ = p1;	*buff++ = color;
	}

	return mesh;


}
struct EditorHUD
{
	TransOp mode = TRANSOP_MOVE;
	bool enable = true;
	Entity selectedEntity;
	FrameBuffer* fbo;
	ShaderPtr shader;
	glm::mat4 world_trans;//trans applied to cross
	MeshPtr cubeMesh;
	float rescale = 1;//distance to camera
	glm::vec4 color;
	bool dragging = false;
	glm::vec3 oldPos;//position before dragging
	glm::vec3 oldScale;//scale before dragging
	glm::vec3 scaler = glm::vec3(1);//amount of relative current scale
	glm::vec3 oldPosOffset;//delta between proper line pos and real object position
	glm::vec3 oldPlanePoint;//point on directional_plane (dir is normal to that plane)
	glm::vec3 oldAngles;//rotation before dragging
	VertexBuffer* lineVBO;
	glm::vec3* lineBuff;
	VertexArray* lineVAO;
	float quantizationPos;
	float quantizationScale;
	float quantizationRotate;
	MeshPtr ring;
	enum DIR
	{
		X, Y, Z, NONE
	} dir = NONE;
	static glm::vec3 getDir(DIR d) {
		switch (d) {
		case X: return { 1,0,0 };
		case Y: return { 0,1,0 };
		case Z: return { 0,0,1 };
		default: return { 0,0,0 };
		}
	}
	void init()
	{
		shader = Shader::create("res/shaders/Arrow.shader");

		fbo = FrameBuffer::create(FrameBufferInfo().defaultTarget(1920, 1080, TextureFormat::RGBA).special(FBAttachment::DEPTH_STENCIL));
		cubeMesh = MeshLibrary::buildNewMesh(MeshDataFactory::buildCube(0.001f * thic));
		constexpr int size = 12 * sizeof(glm::vec3);
		lineBuff = (glm::vec3*)malloc(size);
		glm::vec3 red(1, 0, 0);
		glm::vec3 green(0, 1, 0);
		glm::vec3 blue(0, 0, 1);
		int index = 0;
		lineBuff[index++] = glm::vec3(0, 0, 0);
		lineBuff[index++] = red;
		lineBuff[index++] = glm::vec3(1, 0, 0);
		lineBuff[index++] = red;

		lineBuff[index++] = glm::vec3(0, 0, 0);
		lineBuff[index++] = green;
		lineBuff[index++] = glm::vec3(0, 1, 0);
		lineBuff[index++] = green;

		lineBuff[index++] = glm::vec3(0, 0, 0);
		lineBuff[index++] = blue;
		lineBuff[index++] = glm::vec3(0, 0, 1);
		lineBuff[index++] = blue;




		lineVBO = VertexBuffer::create(lineBuff, size);
		VertexBufferLayout layout{
			g_typ::VEC3,
			g_typ::VEC3,
		};
		lineVBO->setLayout(layout);
		lineVAO = VertexArray::create();
		lineVAO->addBuffer(*lineVBO);


		auto ringsData = buildRing(ringSegments);

		VertexBufferLayout lay{
			g_typ::VEC3,//pos
			g_typ::VEC3,//color
		};
		ring = MeshLibrary::buildNewMesh(ringsData);
		//MeshData nData = MeshData(ringsData->getVerticesCount() * 2, ringsData->getOneVertexSize() * 2, 0);
		//ring = MeshLibrary::loadOrGet("res/models/circle.fbx");


	}
	void onScreenResize(int width, int height)
	{
		fbo->resize(width, height);
	}
	void renderRing(const glm::mat4& trans)
	{
		auto sh = std::static_pointer_cast<GLShader>(shader);
		sh->setUniform4f("color", 1, 1, 1, 1);
		sh->setUniformMat4("world", trans * glm::translate(glm::mat4(1.f), glm::vec3(0, 0, 0)) * glm::scale(glm::mat4(1.f), glm::vec3(0.1f)));
		ring->vao_temp->bind();
		Gcon.cmdDrawArrays(ring->data->getTopology(), 6*ringSegments*3);

		GLCall(glLineWidth(3));
		auto pos = env.view  *glm::vec4(selectedEntity.get<TransformComponent>().pos,1);
		//pos.z = +0.5f;
		sh->setUniformMat4("world", env.proj * glm::scale(glm::translate(glm::mat4(1.f),glm::vec3(pos)),glm::vec3(rescale*0.1f)));
		Gcon.cmdDrawArrays(Topology::LINES, ringSegments*2, 6 * ringSegments * 3);
	}
	void renderCross(const glm::mat4& trans)
	{
		auto sh = std::static_pointer_cast<GLShader>(shader);
		sh->setUniform4f("color", 1, 1, 1, 1);
		sh->setUniformMat4("world", trans * glm::translate(glm::mat4(1.f), glm::vec3(0, 0, 0)) * glm::scale(glm::mat4(1.f), glm::vec3(0.1f)));
		lineVAO->bind();
		GLCall(glLineWidth(5));
		Gcon.cmdDrawArrays(Topology::LINES, 6);

		/*sh->setUniformMat4("world", trans *glm::translate(glm::mat4(1.f),glm::vec3(0.1* thic/2,0,0))* glm::scale(glm::mat4(1.f), glm::vec3(100/ thic, 1, 1)));
		Gcon.cmdDrawArrays(cubeMesh->data->getTopology(), cubeMesh->vertexData.binding.count);
		sh->setUniform4f("color", 0, 1, 0, 1);
		sh->setUniformMat4("world", trans * glm::translate(glm::mat4(1.f), glm::vec3(0.0,0.1* thic/2, 0)) * glm::scale(glm::mat4(1.f), glm::vec3(1, 100 / thic, 1)));
		Gcon.cmdDrawArrays(cubeMesh->data->getTopology(), cubeMesh->vertexData.binding.count);
		sh->setUniform4f("color", 0, 0, 1, 1);
		sh->setUniformMat4("world", trans * glm::translate(glm::mat4(1.f), glm::vec3(0, 0, 0.1* thic/2)) * glm::scale(glm::mat4(1.f), glm::vec3(1, 1, 100 / thic)));
		Gcon.cmdDrawArrays(cubeMesh->data->getTopology(), cubeMesh->vertexData.binding.count);*/
	}

	void render()
	{
		if (!enable || !selectedEntity)
			return;
		fbo->bind();
		fbo->clear(BuffBit::COLOR | BuffBit::DEPTH, { 0,0,0,0 });
		cubeMesh->vao_temp->bind();
		shader->bind();
		auto sh = std::static_pointer_cast<GLShader>(shader);
		if (mode == TRANSOP_MOVE || mode == TRANSOP_SCALE)
			renderCross(env.proj * env.view * world_trans * glm::scale(glm::mat4(1.f), glm::vec3(rescale)));
		else if (mode == TRANSOP_ROTATE)
		{
			renderRing(env.proj * env.view * world_trans* glm::scale(glm::mat4(1.f), glm::vec3(rescale)));
		}
		auto t = glm::mat4(glm::mat3(env.view));
		t[3][3] = 1;
		renderCross(glm::translate(glm::mat4(1.f), glm::vec3(0.9, 0.9, -0.1)) * t);


		glm::ivec2 pos = APin().getMouseLocation();
		auto tex = fbo->getAttachment();
		pos = glm::clamp(pos, glm::ivec2(0, 0), glm::ivec2(tex->width(), tex->height()));
		GLCall(glReadPixels(pos.x, tex->height() - pos.y, 1, 1, GL_RGBA, GL_FLOAT, &color));

		ND_IMGUI_VIEW("HUD", fbo->getAttachment());
		Gcon.enableDepthTest(false);
		Gcon.enableBlend();
		Gcon.setBlendEquation(BlendEquation::FUNC_ADD);
		Gcon.setBlendFunc(Blend::SRC_ALPHA, Blend::ONE_MINUS_SRC_ALPHA);
		Effect::render(fbo->getAttachment(), Renderer::getDefaultFBO());
		Renderer::getDefaultFBO()->bind();
	}


	glm::vec3 quantize(glm::vec3 t, glm::vec3 offset, DIR dir, float quantization)
	{
		if (quantization) {
			t = (t - offset) / quantization;
			switch (dir) {
			case X:
				t.x = glm::round(t.x);
				break;
			case Y:
				t.y = glm::round(t.y);
				break;
			case Z:
				t.z = glm::round(t.z);
				break;
			}
			return t * quantization + offset;
		}
		return t;
	}
	void onUpdate()
	{
		if (!enable || !selectedEntity)
			return;

		auto rot = selectedEntity.get<TransformComponent>().rot;
		world_trans = glm::translate(glm::mat4(1.f), selectedEntity.get<TransformComponent>().pos) * glm::eulerAngleYXZ(rot.x, rot.y, rot.z) * glm::scale(glm::mat4(1.f), scaler);
		rescale = glm::distance(selectedEntity.get<TransformComponent>().pos, env.camera_pos);

		//ND_BUG("loc {} {}", APin().getMouseLocation().x, APin().getMouseLocation().y);
		if (!dragging) {
			if (APin().isMouseFreshlyPressed(MouseCode::LEFT))
			{
				if (color == glm::vec4(0, 0, 0, 0))
					return;
				dir = NONE;
				if (color == glm::vec4(0, 0, 1, 1))
					dir = Z;
				else if (color == glm::vec4(0, 1, 0, 1))
					dir = Y;
				else if (color == glm::vec4(1, 0, 0, 1))
					dir = X;
				if (dir != NONE)
				{
					dragging = true;
					auto& trans = selectedEntity.get<TransformComponent>();
					oldPos = trans.pos;
					oldScale = trans.scale;
					oldAngles = trans.rot;
					auto s = Spacer3D::screenToRay(glm::vec2(APin().getMouseLocation().x, APwin()->getDimensions().y - APin().getMouseLocation().y), env.proj, env.view, APwin()->getDimensions());
					oldPosOffset = selectedEntity.get<TransformComponent>().pos - Spacer3D::getClosestPointOnLine(glm::vec3(glm::eulerAngleYXZ(oldAngles.x, oldAngles.y, oldAngles.z) * glm::vec4(getDir(dir), 0.f)), oldPos, s, env.camera_pos);
					oldPlanePoint = Spacer3D::getPointOnPlane(glm::vec3(glm::eulerAngleYXZ(oldAngles.x, oldAngles.y, oldAngles.z) * glm::vec4(getDir(dir), 0.f)), oldPos, s, env.camera_pos) /*+ oldPosOffset*/;

				}
			}
		}
		else
		{
			if (!APin().isMousePressed(MouseCode::LEFT))
			{
				dragging = false;
				scaler = glm::vec3(1.f);
			}
			else
			{
				switch (mode) {
				case TRANSOP_MOVE:
				{
					auto s = Spacer3D::screenToRay(glm::vec2(APin().getMouseLocation().x, APwin()->getDimensions().y - APin().getMouseLocation().y), env.proj, env.view, APwin()->getDimensions());
					auto& trans = selectedEntity.get<TransformComponent>();

					trans.pos = Spacer3D::getClosestPointOnLine(glm::vec3(glm::eulerAngleYXZ(oldAngles.x, oldAngles.y, oldAngles.z) * glm::vec4(getDir(dir), 0.f)), oldPos, s, env.camera_pos) + oldPosOffset;
					trans.pos = quantize(trans.pos, oldPos, dir, quantizationPos);
					trans.recomputeMatrix();
				}
				break;
				case TRANSOP_SCALE:
				{
					auto s = Spacer3D::screenToRay(glm::vec2(APin().getMouseLocation().x, APwin()->getDimensions().y - APin().getMouseLocation().y), env.proj, env.view, APwin()->getDimensions());
					auto& trans = selectedEntity.get<TransformComponent>();

					auto scale = Spacer3D::getClosestPointOnLine(getDir(dir), oldPos, s, env.camera_pos) + oldPosOffset;
					scale -= oldPos;
					float len = 0;
					switch (dir) {
					case X: len = scale.x; break;
					case Y: len = scale.y; break;
					case Z: len = scale.z; break;
					}
					len /= rescale / 10;

					if (quantizationScale)
						len = glm::round(len / quantizationScale) * quantizationScale;
					if (APin().isKeyPressed(KeyCode::LEFT_CONTROL))
						scaler = glm::vec3(len + 1.f);
					else
						scaler = glm::vec3(1) + getDir(dir) * len;
					trans.scale = oldScale * scaler;
					trans.recomputeMatrix();

					break;
				}
				case TRANSOP_ROTATE:
				{
					auto s = Spacer3D::screenToRay(glm::vec2(APin().getMouseLocation().x, APwin()->getDimensions().y - APin().getMouseLocation().y), env.proj, env.view, APwin()->getDimensions());
					auto& trans = selectedEntity.get<TransformComponent>();

					auto planePoint = Spacer3D::getPointOnPlane(glm::vec3(glm::eulerAngleYXZ(oldAngles.x, oldAngles.y, oldAngles.z)*glm::vec4(getDir(dir),0.f)), trans.pos, s, env.camera_pos) /*+ oldPosOffset*/;
					//sphere1.get<TransformComponent>().pos = planePoint;
					//sphere1.get<TransformComponent>().recomputeMatrix();
					float angle = glm::acos(glm::dot(glm::normalize(planePoint - trans.pos), glm::normalize(oldPlanePoint - trans.pos)))/*/glm::length(planePoint - trans.pos)/glm::length(oldPlanePoint - trans.pos)*/;
					if (!(glm::abs(glm::length2(glm::normalize(glm::cross(planePoint - trans.pos, oldPlanePoint - trans.pos)) - glm::normalize(glm::vec3(glm::eulerAngleYXZ(oldAngles.x, oldAngles.y, oldAngles.z) * glm::vec4(getDir(dir), 0.f))))) > 0.1f))
						angle *= -1;
					if (quantizationRotate)
						angle = glm::round(angle / quantizationRotate) * quantizationRotate;
					switch (dir) {
					case X:  trans.rot.y = oldAngles.y + angle; 
						break;
					case Y:
						trans.rot.x = oldAngles.x + angle;
						break;
					case Z:trans.rot.z = oldAngles.z + angle;
						break;
					}
					trans.recomputeMatrix();

				}
				break;
				default:;
				}

			}
		}
	}
};
static EditorHUD hud;

static glm::vec3 worldToScreen(const glm::vec3& src, const glm::mat4 proj, const glm::mat4& view, const glm::vec2& screenResolution)
{
	glm::vec4 s = view * glm::vec4(src.x, src.y, src.z, 1);
	auto depth = s.z;
	s = proj * s;
	s /= s.w;
	s.x = (s.x + 1.f) / 2.f;
	s.y = (s.y + 1.f) / 2.f;
	auto v = glm::vec2(s) * screenResolution;
	return glm::vec3(v, depth);
}



UniformLayout envLayout;
static MaterialPtr enviroment;

static void onCamComponentDestroyed(entt::registry& reg, entt::entity ent)
{

}

struct WireMoveScript :NativeScript
{
	void onUpdate()
	{
		auto& transform = getComponent<TransformComponent>();
		auto& camTrans = scene->currentCamera().get<TransformComponent>();

		bool rec = false;
		if (abs((transform.pos.x + 20 * 4) - camTrans.pos.x) > 4) {
			transform.pos.x = camTrans.pos.x - 20 * 4;
			rec = true;
		}
		if (abs((transform.pos.z + 20 * 4) - camTrans.pos.z) > 4) {
			transform.pos.z = camTrans.pos.z - 20 * 4;
			rec = true;
		}
		if (rec)
			transform.recomputeMatrix();

	}
};
static float centerDepth;
static float* depth_buff;
static Entity sphere;

void EditorLayer::onAttach()
{
	hud.init();
	components_imgui_access::windows.quantizationPos = &hud.quantizationPos;
	components_imgui_access::windows.quantizationScale = &hud.quantizationScale;
	components_imgui_access::windows.quantizationRot = &hud.quantizationRotate;

	auto screenRes = glm::vec2(100, 100);
	auto worldPos = glm::vec3(2, 5, 10);
	auto proj = glm::perspective(glm::quarter_pi<float>(), screenRes.x / screenRes.y, 1.f, 100.f);
	auto view = glm::translate(glm::mat4(1.f), glm::vec3(1, 1, 1));
	auto world = glm::mat4(1.f);
	//auto screenTrans = worldToScreen(worldPos, proj, view, world, screenRes);
	//auto back = screenToWorld(screenTrans,proj,view,world,screenRes);


	depth_buff = &centerDepth;
	//depth_buff = (float*)malloc(1920 * 1080 * sizeof(float));
	components_imgui_access::windows.init();
	auto t = Atelier::get();//just init atelier
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF(ND_RESLOC("res/fonts/ignore/OpenSans-Regular.ttf").c_str(), 20);

	//adding JP (merging with current font) -> nihongode ok kedo, mada nai to omou
	/*ImFontConfig config;
	config.MergeMode = true;
	io.Fonts->AddFontFromFileTTF(ND_RESLOC("res/fonts/NotoSansCJKjp-Medium.otf").c_str(), 20, &config, io.Fonts->GetGlyphRangesJapanese());
	io.Fonts->Build();*/

	oneColorShader = ShaderLib::loadOrGetShader("res/shaders/OneColor.shader");

	m_scene = new NewScene;
	m_scene->reg().on_destroy<CameraComponent>().connect<&onCamComponentDestroyed>();
	components_imgui_access::windows.scene = m_scene;


	envLayout.name = "GLO";
	envLayout.prefixName = "glo";
	envLayout.emplaceElement(g_typ::MAT4, 1, "glo.view");
	envLayout.emplaceElement(g_typ::MAT4, 1, "glo.proj");

	envLayout.emplaceElement(g_typ::VEC3, 1, "glo.sunPos");
	envLayout.emplaceElement(g_typ::VEC3, 1, "glo.ambient");
	envLayout.emplaceElement(g_typ::VEC3, 1, "glo.diffuse");
	envLayout.emplaceElement(g_typ::VEC3, 1, "glo.specular");
	envLayout.emplaceElement(g_typ::VEC3, 1, "glo.camera_pos");

	envLayout.emplaceElement(g_typ::FLOAT, 1, "glo.constant");
	envLayout.emplaceElement(g_typ::FLOAT, 1, "glo.linear");
	envLayout.emplaceElement(g_typ::FLOAT, 1, "glo.quadratic");
	enviroment = Material::create({ nullptr, "GLO", "Enviroment", &envLayout });


	auto modelMat = Material::create({
		std::shared_ptr<Shader>(ShaderLib::loadOrGetShader("res/shaders/Model.shader")), "MAT",
		"modelMaterial"
		});
	modelMat->setValue("color", glm::vec4(1.0, 1.0, 0, 1));
	modelMat->setValue("shines", 64.f);


	auto crate1Mesh = MeshLibrary::loadOrGet("res/models/cube.fbx");


	auto simpleMat = MaterialLibrary::create({
	std::shared_ptr<Shader>(ShaderLib::loadOrGetShader("res/shaders/Model.shader")), "MAT",
	"simpleColorMat"
		});
	simpleMat->setValue("color", glm::vec4(1.0, 1.0, 0.5, 1));
	simpleMat->setValue("shines", 64.f);

	if (!FUtil::exists("res/models/dragon.bin"))
	{
		ND_INFO("Building dragon binary mesh");
		MeshDataFactory::writeBinaryFile(
			"res/models/dragon.bin", *Colli::buildMesh("res/models/dragon.obj"));
	}
	//adding cubemap
	{
		//adding cube_map
		auto mesh = MeshLibrary::registerMesh(MeshDataFactory::buildCube(40.f));

		auto flags = MaterialFlags::DEFAULT_FLAGS;
		flags = (~(MaterialFlags::FLAG_CULL_FACE | MaterialFlags::FLAG_DEPTH_MASK) & flags) | MaterialFlags::FLAG_CHOP_VIEW_MAT_POS;

		ND_INFO("Loading cubemaps");
		auto mat = MaterialLibrary::create({ ShaderLib::loadOrGetShader("res/shaders/CubeMap.shader"),"MAT","SkyMaterial2" ,nullptr,flags });
		mat->setValue("cubemap", std::shared_ptr<Texture>(Texture::create(TextureInfo(TextureType::_CUBE_MAP, "res/images/skymap2/*.png"))));
		/*auto mat1 = MaterialLibrary::create({ ShaderLib::loadOrGetShader("res/shaders/CubeMap.shader"),"MAT","SkyMaterial3" ,nullptr,flags });
		mat1->setValue("cubemap", std::shared_ptr<Texture>(Texture::create(TextureInfo(TextureType::_CUBE_MAP, "res/images/skymap3/*.png"))));
		auto mat2 = MaterialLibrary::create({ ShaderLib::loadOrGetShader("res/shaders/CubeMap.shader"),"MAT","SkyMaterial4" ,nullptr,flags });
		mat2->setValue("cubemap", std::shared_ptr<Texture>(Texture::create(TextureInfo(TextureType::_CUBE_MAP, "res/images/skymap4/*.png"))));*/

		auto ent = m_scene->createEntity("SkyBox");
		ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f), glm::vec3(1.f), glm::vec3(0.f));
		ent.emplaceOrReplace<ModelComponent>(mesh->getID(), mat->getID());
	}

	//adding crate
	{
		//auto diffuse = Texture::create(TextureInfo("res/models/crate.png"));
		//auto specular = Texture::create(TextureInfo("res/models/crate_specular.png"));

		//auto mesh = NewMeshFactory::buildNewMesh(Colli::buildMesh(ND_RESLOC("res/models/cube.fbx")));

		/*auto mat = modelMat->copy("crateMaterial");
		mat->setValue("diffuse", std::shared_ptr<Texture>(diffuse));
		mat->setValue("specular", std::shared_ptr<Texture>(specular));
		mat->setValue("color", glm::vec4(1.0, 1.0, 0, 0));
		MaterialLibrary::save(mat, "res/crateM.mat");*/
		auto mat = MaterialLibrary::loadOrGet("res/models/crateM.mat");

		auto ent = m_scene->createEntity("Crate");
		ent.emplaceOrReplace<TransformComponent>(glm::vec3(1.f), glm::vec3(100.f), glm::vec3(0.f));
		ent.emplaceOrReplace<ModelComponent>(crate1Mesh->getID(), mat->getID());
	}

	//adding sphere
	{
		auto diffuse = Texture::create(TextureInfo("res/models/crate.png"));

		auto mesh = MeshLibrary::loadOrGet("res/models/sphere.fbx");

		auto mat = MaterialLibrary::copy(modelMat, "SphereMat");
		mat->setValue("color", glm::vec4(1.0, 1.0, 0, 0));
		mat->setValue("diffuse", std::shared_ptr<Texture>(diffuse));

		auto ent = m_scene->createEntity("Sphere");
		//ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f, 5.f, 0.f), glm::vec3(1.f), glm::vec3(0.f));
		ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f), glm::vec3(0.f));
		ent.emplaceOrReplace<ModelComponent>(mesh->getID(), mat->getID());
		{

			auto mat = MaterialLibrary::copy(simpleMat, "SphereMat0");
			mat->setValue("color", glm::vec4(1.0, 0.0, 0, 1));
			mat->setValue("diffuse", std::shared_ptr<Texture>(diffuse));

			sphere0 = m_scene->createEntity("Sphere0");
			sphere0.emplaceOrReplace<TransformComponent>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.2f), glm::vec3(0.f));
			sphere0.emplaceOrReplace<ModelComponent>(mesh->getID(), mat->getID());
			{
				auto mat = MaterialLibrary::copy(simpleMat, "SphereMat1");
				mat->setValue("color", glm::vec4(0.0, 1.0, 0, 1));
				mat->setValue("diffuse", std::shared_ptr<Texture>(diffuse));

				sphere1 = m_scene->createEntity("Sphere1");
				sphere1.emplaceOrReplace<TransformComponent>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.2f), glm::vec3(0.f));
				sphere1.emplaceOrReplace<ModelComponent>(mesh->getID(), mat->getID());
			}
			{
				auto mat = MaterialLibrary::copy(simpleMat, "SphereMat2");
				mat->setValue("color", glm::vec4(0.0, 0.0, 1, 1));
				mat->setValue("diffuse", std::shared_ptr<Texture>(diffuse));

				sphere2 = m_scene->createEntity("Sphere2");
				sphere2.emplaceOrReplace<TransformComponent>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.2f), glm::vec3(0.f));
				sphere2.emplaceOrReplace<ModelComponent>(mesh->getID(), mat->getID());
			}
		}
		sphere = ent;
	}
	//adding dragoon
	/*{
	auto dragonMesh = NewMeshFactory::buildNewMesh(MeshFactory::readBinaryFile(ND_RESLOC("res/models/dragon.bin")));
		//auto mesh = NewMeshFactory::buildNewMesh(data);
		auto mesh = dragonMesh;
		mesh->topology = Topology::TRIANGLES;
		auto mat = MaterialLibrary::copy(modelMat,"dragoonMat");
		//mat->setValue("color", glm::vec4(0.f, 1.f, 0.f, 1.f));
		MaterialLibrary::save(mat, "res/drag.mat");

		auto ent = m_scene->createEntity("Dragoon");
		ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f), glm::vec3(1.f), glm::vec3(0.f));
		ent.emplaceOrReplace<ModelComponent>(mesh, mat);
	}*/

	//adding light
	{
		auto ent = m_scene->createEntity("Light");
		ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f), glm::vec3(1, 10.f, 1), glm::vec3(0.f));
		ent.emplaceOrReplace<LightComponent>();
	}
	//adding camera
	{
		auto ent = m_scene->createEntity("Cam");
		ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f), glm::vec3(1.f), glm::vec3(0.f));
		ent.emplaceOrReplace<CameraComponent>(glm::mat4(1.f), glm::quarter_pi<float>(), 1.f, 100.f);
		m_scene->currentCamera() = ent;
		ent.emplaceOrReplace<NativeScriptComponent>();
		auto& script = ent.get<NativeScriptComponent>();
		script.bind<EditCameraController>();
	}
	//adding wire
	{
		auto ent = m_scene->createEntity("Wire");
		ent.emplaceOrReplace<TransformComponent>(glm::vec3(0.f), glm::vec3(4.f), glm::vec3(0.f));

		auto mesh = MeshLibrary::registerMesh(MeshDataFactory::buildWirePlane(40, 40));
		auto mat = MaterialLibrary::create({ ShaderLib::loadOrGetShader("res/shaders/Model.shader"),"MAT","WireMat" ,nullptr,MaterialFlags::DEFAULT_FLAGS });
		mat->setValue("shines", 0.f);
		mat->setValue("color", glm::vec4(0.5f, 0.5f, 0.5f, 1));


		ent.emplaceOrReplace<ModelComponent>(mesh->getID(), mat->getID());
		ent.emplaceOrReplace<NativeScriptComponent>();
		auto& script = ent.get<NativeScriptComponent>();
		script.bind<WireMoveScript>();
	}
}

void EditorLayer::onDetach()
{
}

void EditorLayer::onUpdate()
{
	hud.onUpdate();
	auto view = m_scene->reg().view<NativeScriptComponent>();
	for (auto entity : view)
	{
		auto& script = view.get(entity);
		if (!script.ptr) {
			script.construct(m_scene->wrap(entity), m_scene);
			script.onCreate();
		}
		script.onUpdate();
	}
}
static glm::vec2 minMaxCam;
void EditorLayer::onRender()
{
	GLCall(glLineWidth(1));
	Atelier::get().makePendingPhotos();
	Renderer::getDefaultFBO()->bind();
	Gcon.enableCullFace(true);
	Gcon.enableDepthTest(true);
	Entity camEntity = m_scene->currentCamera();
	//Renderer::getDefaultFBO()->clear(BuffBit::COLOR, { 0,1,0,1 });
	auto& camCom = camEntity.get<CameraComponent>();
	minMaxCam = { camCom.Near,camCom.Far };
	auto& transCom = camEntity.get<TransformComponent>();
	env.camera_pos = transCom.pos;

	env.view = camCom.viewMatrix;
	env.proj = glm::perspective(camCom.fov,
		(float)APwin()->getWidth() / (float)App::get()
		.getWindow()->getHeight(),
		camCom.Near, camCom.Far);

	auto lights = m_scene->view<LightComponent>();
	for (auto light : lights)
	{
		auto& l = lights.get(light);
		env.ambient = l.ambient;
		env.diffuse = l.diffuse;
		env.specular = l.specular;
		env.constant = l.constant;
		env.linear = l.linear;
		env.quadratic = l.quadratic;
		env.sunPos = m_scene->wrap(light).get<TransformComponent>().pos;
	}
	auto models = m_scene->group<TransformComponent, ModelComponent>();
	int index = 0;
	entt::entity selectedE = entt::null;
	for (auto model : models)
	{
		if (!m_scene->reg().get<TagComponent>(model).enabled)
			continue;
		//auto& [trans, mod] = models.get<TransformComponent,ModelComponent>(model);
		auto& trans = models.get<TransformComponent>(model);
		trans.recomputeMatrix();
		auto& mod = models.get<ModelComponent>(model);
		auto& material = mod.Material();
		auto& mesh = mod.Mesh();
		//bind mat vars
		material->bind();

		//bind enviroment vars
		enviroment->setRaw(env);
		enviroment->bind(0, material->getShader());
		mesh->vao_temp->bind();

		//bind other vars
		auto s = std::static_pointer_cast<GLShader>(material->getShader());
		//if(mod.material->getShader()->getLayout().getLayoutByName("world"))
		s->setUniformMat4("world", trans.trans);

		auto flags = material->getFlags();
		if (flags != MaterialFlags::DEFAULT_FLAGS) {
			Gcon.depthMask(flags & MaterialFlags::FLAG_DEPTH_MASK);
			Gcon.enableCullFace(flags & MaterialFlags::FLAG_CULL_FACE);
			Gcon.enableDepthTest(flags & MaterialFlags::FLAG_DEPTH_TEST);
			if (flags & MaterialFlags::FLAG_CHOP_VIEW_MAT_POS)
				s->setUniformMat4("glo.view", glm::mat4(glm::mat3(env.view)) * glm::scale(glm::mat4(1.f), trans.scale));
		}
		bool selected = hud.selectedEntity == m_scene->wrap(model);

		if (selected)
		{
			selectedE = model;
			Gcon.enableStencilTest(true);
			Gcon.stencilOp(StencilOp::KEEP, StencilOp::REPLACE, StencilOp::REPLACE);
			Gcon.stencilFunc(StencilFunc::ALWAYS, 1);

		}
		if (mesh->indexData.exists())
			Gcon.cmdDrawElements(mesh->data->getTopology(), mesh->indexData.count);
		else
			Gcon.cmdDrawArrays(mesh->data->getTopology(), mesh->vertexData.binding.count);
		if (selected)
			Gcon.enableStencilTest(false);

		//reset if neccessary
		if (flags != MaterialFlags::DEFAULT_FLAGS) {
			Gcon.depthMask(true);
			Gcon.enableCullFace(true);
			Gcon.enableDepthTest(true);
		}
	}
	auto size = Renderer::getDefaultFBO()->getSize();
	GLCall(glReadPixels(size.x / 2, size.y / 2, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, depth_buff));
	m_scene->getLookingDepth() = getCurrentDepth();

	if (selectedE != entt::null)
	{
		auto& trans = models.get<TransformComponent>(selectedE);
		auto& mod = models.get<ModelComponent>(selectedE);
		auto& mesh = mod.Mesh();
		Gcon.enableDepthTest(false);
		Gcon.enableStencilTest(true);
		Gcon.stencilOp(StencilOp::KEEP, StencilOp::KEEP, StencilOp::KEEP);
		Gcon.stencilFunc(StencilFunc::EQUAL, 0);

		oneColorShader->bind();
		mesh->vao_temp->bind();
		auto sha = dynamic_cast<GLShader*>(oneColorShader.get());
		sha->setUniform4f("color", 0, 1, 0, 1);
		sha->setUniformMat4("world", env.proj * env.view * glm::scale(trans.trans, glm::vec3(1.05f)));
		if (mesh->indexData.exists())
			Gcon.cmdDrawElements(mesh->data->getTopology(), mesh->indexData.count);
		else
			Gcon.cmdDrawArrays(mesh->data->getTopology(), mesh->vertexData.binding.count);

		Gcon.enableStencilTest(false);
		Gcon.enableDepthTest(true);

	}

	hud.render();


}
// get world distance based on depth pixel value d
static float transformDepth(float d, float min, float max) {
	return (max * min / (max - min)) / (-d + max / (max - min));
}

void EditorLayer::onImGuiRender()
{
	components_imgui_access::windows.activeCamera = m_scene->currentCamera();
	components_imgui_access::windows.drawWindows();
	hud.mode = components_imgui_access::windows.transformOperation;
	hud.selectedEntity = components_imgui_access::windows.selectedEntity;

	m_scene->currentCamera() = components_imgui_access::windows.activeCamera;
	/*auto s = screenToRay(glm::vec2(APin().getMouseLocation().x, APwin()->getDimensions().y -APin().getMouseLocation().y), env.proj, env.view, APwin()->getDimensions());
	//ImGui::Text("x=%f y=%f z=%f", s.x, s.y, s.z);
	ImGui::ColorEdit4("color", (float*)&hud.color);

	//sphere.get<TransformComponent>().pos = s * 10.f + env.camera_pos;
	static glm::vec3 defPos = sphere.get<TransformComponent>().pos;
	sphere.get<TransformComponent>().pos = getClosestPointOnLine(glm::vec3(1,0 ,0), glm::vec3(0.0f), s,env.camera_pos);
	sphere.get<TransformComponent>().recomputeMatrix();*/


}

void EditorLayer::onEvent(Event& e)
{
	auto view = m_scene->reg().view<NativeScriptComponent>();
	for (auto entity : view)
	{
		auto& script = view.get(entity);
		if (!script.ptr)
			script.construct(m_scene->wrap(entity), m_scene);
		script.onEvent(e);
	}
}

float EditorLayer::getCurrentDepth() {
	//float f = *(depth_buff + APwin()->getWidth() * APwin()->getHeight() / 2);
	float f = *depth_buff;
	return transformDepth(f, minMaxCam.x, minMaxCam.y);
}

void EditorLayer::onWindowResize(int width, int height)
{

	hud.onScreenResize(width, height);
}

