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
	inline void setSize(const glm::vec2& size)
	{
		uv[1] = uv[0] + glm::vec2(size.x, 0);
		uv[2] = uv[0] + size;
		uv[3] = uv[0] + glm::vec2(0, size.y);
	}
	inline static UVQuad elementary()
	{
		return UVQuad({ 0,0 }, { 1,1 });
	}
	inline static UVQuad build(const glm::vec2& pos, const glm::vec2& size, bool horizontalFlip = false, bool verticalFlip = false,bool rotate90=false)
	{
		glm::vec2 pos0 = pos;
		glm::vec2 pos1 = pos+size;
		if(horizontalFlip)
		{
			float x = pos0.x;
			pos0.x = pos1.x;
			pos1.x = x;
		}
		if (verticalFlip)
		{
			float y = pos0.y;
			pos0.y = pos1.y;
			pos1.y = y;
		}

		UVQuad out;
		
		out.uv[0] = pos0;
		out.uv[1] = glm::vec2(pos1.x,pos0.y);
		out.uv[2] =pos1;
		out.uv[3] = glm::vec2(pos0.x,pos1.y);

		if(rotate90)
		{
			auto last = out.uv[0];
			out.uv[0] = out.uv[1];
			out.uv[1] = out.uv[2];
			out.uv[2] = out.uv[3];
			out.uv[3] = last;
		}

		
		return out;
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
