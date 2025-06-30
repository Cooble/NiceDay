#pragma once
#include "layer/Layer.h"

namespace nd {

class NewScene;

class EditorLayer : public Layer
{
private:
	NewScene* m_scene;
	glm::vec2 m_depth_sampling_position;
public:
	void onAttach() override;
	void onDetach() override;
	void onUpdate() override;
	void onRender() override;
	void onImGuiRender() override;
	void onEvent(Event& e) override;

	float getCurrentDepth();
	void onWindowResize(int width, int height) override;

	auto& scene() { return *m_scene; }


	void addExampleObjects();
	void initDefaultScene();

	// normalized world direction
	glm::vec3 screenToWorld(const glm::vec2& pixelScreenPos);
	//void initDefaultCamera();

	// this will return the last depth value, not the current one
	float getDepthAtScreen(const glm::vec2& pixelScreenPos);
};
}
