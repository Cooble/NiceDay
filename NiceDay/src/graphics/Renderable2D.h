#pragma once
class Texture;
struct UVQuad
{
	glm::vec2 uv[4];
	UVQuad(const glm::vec2& v0, const glm::vec2& v1, const glm::vec2& v2, const glm::vec2& v3 )
	{
		uv[0] = v0;
		uv[1] = v1;
		uv[2] = v2;
		uv[3] = v3;
	}
	UVQuad(const glm::vec2& pos, const glm::vec2& size)
	{
		uv[0] = pos;
		uv[1] = pos + glm::vec2(size.x,0);
		uv[2] = pos + size;
		uv[3] = pos + glm::vec2(0,size.y);
	}
	UVQuad(){}
	inline void setPosition(const glm::vec2& pos)
	{
		auto dx = uv[2].x - uv[0].x;
		auto dy = uv[2].y - uv[0].y;

		uv[0] = pos;
		uv[1] = pos+glm::vec2(dx,0);
		uv[2] = pos + glm::vec2(dx, dy);
		uv[3] = pos+glm::vec2(0, dy);

	}

};
class Renderable2D
{
public:
	virtual ~Renderable2D()=default;
	virtual const Texture* getTexture() const = 0;
	virtual const glm::vec3& getPosition() const = 0;
	virtual const glm::vec2& getSize() const = 0;
	virtual const UVQuad& getUV() const = 0;
	
};
