#pragma once
#include "Renderer.h"
class IRenderable {
public:
	virtual void render(Renderer &r) = 0;
};
