#pragma once
#include "layer/Layer.h"


//used to roam not the pos space but uv space
struct FlatCam
{
	float unitsPerPixel = 4.f / 1280;
	float scrollSpeed = 0.2f;

	bool dragging = false;
	glm::vec2 pos = {};
	glm::vec2 startCursor;
	glm::vec2 startPos;

	void onEvent(nd::Event& e);

	glm::mat4 getProjMatrix();
	void imGuiPropsRender();
};


class MandelBrotLayer:public nd::Layer
{
public:
	MandelBrotLayer() = default;
	void onAttach() override;
	void onDetach() override;
	void onImGuiRender() override;
	void onRender() override;
	void onEvent(nd::Event& e) override;
	
};
