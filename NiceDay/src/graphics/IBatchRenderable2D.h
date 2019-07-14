#pragma once

class BatchRenderer2D;

//can submit renderables to renderer
class IBatchRenderable2D
{
public:
	virtual ~IBatchRenderable2D() = default;
	virtual void render(BatchRenderer2D& renderer) = 0;
	
};
