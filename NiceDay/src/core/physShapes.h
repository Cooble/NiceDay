#pragma once
#include "ndpch.h"
#include <glm/glm.hpp>
#include <utility>

inline glm::vec2 toglm(float f) { return glm::vec2(f, f); }


namespace Phys
{
	
	typedef glm::vec<2, int> Vecti;
	struct Polygon;
	using namespace glm;

	inline uint64_t toInt64(float x, float y)
	{
		return *(uint64_t*)&glm::vec2(x, y);
	}
	inline uint64_t toInt64(const glm::vec2& v)
	{
		return *(uint64_t*)&v;
	}
	
	inline bool isValidFloat(float f)
	{
		return !std::isnan(f) && !std::isinf(f);
	}
	inline float angleDegrees(const vec2& v)
	{
		if (v.x == 0) // special cases
			return (v.y > 0)
			? 90
			: (v.y == 0)
			? 0
			: 270;
		else if (v.y == 0) // special cases
			return (v.x >= 0)
			? 0
			: 180;
		float ret = atanf((float)v.y / v.x) / 3.14159f * 180;
		if (v.x < 0 && v.y < 0) // quadrant Ⅲ
			ret = 180 + ret;
		else if (v.x < 0) // quadrant Ⅱ
			ret = 180 + ret; // it actually substracts
		else if (v.y < 0) // quadrant Ⅳ
			ret = 270 + (90 + ret); // it actually substracts
		return ret;
	}
	
	inline bool isValid(const vec2& v) { return isValidFloat(v.x) && isValidFloat(v.y); }
	inline vec2 vectFromAngle(float rad)
	{
		return {cos(rad), sin(rad)};
	}
	inline std::ostream& operator<<(std::ostream& os, const vec2& myObject)
	{
		if (isValid(myObject))
			os << "Vect[" << myObject.x << ", " << myObject.y << "]";
		else
			os << "Vect[invalid]";
		return os;
	}

	constexpr float toRad(float f)
	{
		return f / 180.f * 3.14159265f;
	}
	inline float angleRad(const vec2& v)
	{
		return toRad(angleDegrees(v));
	}

	constexpr float toDeg(float f)
	{
		return f / 3.14159265f * 180.f;
	}
	inline vec2 invalidVec2()
	{
		return { std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN() };
	}

	template <typename T=float>
	struct Rect
	{
		T x0, y0, x1, y1;

		inline bool containsPoint(T x, T y) const
		{
			return (x >= x0) && (x <= x1) && (y >= y0) && (y <= y1);
		}

		inline bool containsPoint(const vec2& v) const
		{
			return (v.x >= x0) && (v.x <= x1) && (v.y >= y0) && (v.y <= y1);
		}

		inline T width() const { return x1 - x0; }
		inline T height() const { return y1 - y0; }

		inline void add(int x, int y)
		{
			x0 += x;
			x1 += x;
			y0 += y;
			y1 += y;
		}

		inline void add(const vec2& v)
		{
			add(v.x, v.y);
		}

		inline void add(float x, float y)
		{
			x0 += x;
			x1 += x;
			y0 += y;
			y1 += y;
		}

		inline void minus(int x, int y)
		{
			x0 -= x;
			x1 -= x;
			y0 -= y;
			y1 -= y;
		}

		inline void minus(const vec2& v)
		{
			minus(v.x, v.y);
		}

		inline void minus(float x, float y)
		{
			x0 -= x;
			x1 -= x;
			y0 -= y;
			y1 -= y;
		}

		inline static Rect<T> createFromDimensions(T x, T y, T width, T height)
		{
			return {x, y, x + width, y + height};
		}

		inline void setWidth(T width) { x1 = x0 + width; }
		inline void setHeight(T height) { y1 = y0 + height; }

	private:
		inline bool intersectsInner(const Rect<T>& rect) const
		{
			return
				rect.containsPoint(x0, y0) ||
				rect.containsPoint(x1, y0) ||
				rect.containsPoint(x0, y1) ||
				rect.containsPoint(x1, y1);
		}

	public:
		inline bool intersects(const Rect<T>& rect) const
		{
			return intersectsInner(rect) || rect.intersectsInner(*this);
		}
	};

	typedef Rect<int> Rectanglei;
	typedef Rect<float> Rectangle;

	struct Polygon
	{
		std::vector<vec2> list;
		int count;
		bool isRectangle = false;
	private:
		Rectangle bounds;
	public:

		Polygon(std::vector<vec2> v = {}): list(std::move(v))
		{
			count = list.size();
			recalculateBounds();
		}

		Polygon(const Polygon& p)
		{
			list.resize(p.list.size());
			memcpy(list.data(), p.list.data(), sizeof(vec2) * p.size());
			count = p.size();
			isRectangle = p.isRectangle;
			bounds = p.bounds;
		}

		inline void recalculateBounds()
		{
			bounds.x0 = std::numeric_limits<float>::max();
			bounds.y0 = std::numeric_limits<float>::max();

			bounds.x1 = std::numeric_limits<float>::lowest();
			bounds.y1 = std::numeric_limits<float>::lowest();

			for (const auto& v : list)
			{
				bounds.x0 = std::min(bounds.x0, v.x);
				bounds.x1 = std::max(bounds.x1, v.x);
				bounds.y0 = std::min(bounds.y0, v.y);
				bounds.y1 = std::max(bounds.y1, v.y);
			}
		}

		inline vec2& operator[](size_t index)
		{
			ASSERT(index < count&&index >= 0, "Invalid index");
			return list[index];
		}

		inline const vec2& operator[](size_t index) const
		{
			ASSERT(index < count&&index >= 0, "Invalid index");
			return list[index];
		}

		inline Polygon& plus(const vec2& v)
		{
			for (auto& vec : list)
				vec += v;
			recalculateBounds();
			return *this;
		}

		inline Polygon& minus(const vec2& v)
		{
			for (auto& vec : list)
				vec -= v;
			recalculateBounds();
			return *this;
		}

		inline int size() const { return count; }

		inline const Rectangle& getBounds() const { return bounds; }

		inline Polygon copy() const
		{
			return Polygon(*this);
		}
	};

	inline Polygon toPolygon(const Rectangle& r)
	{
		auto out = Polygon({
			{r.x0, r.y0},
			{r.x1, r.y0},
			{r.x1, r.y1},
			{r.x0, r.y1}
		});
		out.isRectangle = true;
		return out;
	}

	// lines collisions ================================================================

	// returns the point of intersection between line a and b or if (no point or every point) -> invalid
	inline vec2 intersectLines(const vec2& pa0, const vec2& pa1, const vec2& pb0, const vec2& pb1)
	{
		auto deltaA = pa1 - pa0;
		auto deltaB = pb1 - pb0;

		float t = (deltaB.x * (pb0.y - pa0.y) + deltaB.y * (pa0.x - pb0.x)) / (deltaB.x * deltaA.y - deltaA.x * deltaB.y
		);
		return pa0 + deltaA * t;
	}

	inline bool isBetween(float val, float bound0, float bound1)
	{
		return ((bound0 - val) * (val - bound1)) >= 0; //without branch prediction it should be faster...
	}

	// returns the point of intersection between abscisses a and b or if (no point or every point) -> invalid
	inline vec2 intersectAbscisses(const vec2& pa0, const vec2& pa1, const vec2& pb0, const vec2& pb1)
	{
		vec2 v = intersectLines(pa0, pa1, pb0, pb1);
		if (!isValid(v))
			return v;

		if (
			isBetween(v.x, pa0.x, pa1.x) &&
			isBetween(v.x, pb0.x, pb1.x) &&
			isBetween(v.y, pa0.y, pa1.y) &&
			isBetween(v.y, pb0.y, pb1.y))
			return v;
		return invalidVec2();
	}

	// checks if abscisses a and b intersect at one point
	inline bool isIntersectAbscisses(const  vec2& pa0, const  vec2& pa1, const  vec2& pb0, const  vec2& pb1)
	{
		auto v = intersectLines(pa0, pa1, pb0, pb1);
		if (!isValid(v))
			return false;

		return
			isBetween(v.x, pa0.x, pa1.x) &&
			isBetween(v.x, pb0.x, pb1.x) &&
			isBetween(v.y, pa0.y, pa1.y) &&
			isBetween(v.y, pb0.y, pb1.y);
	}

	//polygons =========================================================================

	inline bool isIntersects(const Polygon& a, const Polygon& b)
	{
		if (!a.getBounds().intersects(b.getBounds())) //first check if it is even worth trying
			return false;

	//	if (a.size() == 3 || b.size() == 3)
	//		__debugbreak();


		//if (a.isRectangle&&b.isRectangle)
		//	return true;
		for (int i = 0; i < a.size() - 1; ++i)
			//it is that big because not using modulo for p1,p2 (bigger code - bigger performance)
		{
			auto& p0 = a[i];
			auto& p1 = a[i + 1];

			for (int j = 0; j < b.size() - 1; ++j)
			{
				auto& p2 = b[j];
				auto& p3 = b[j + 1];
				if (isIntersectAbscisses(p0, p1, p2, p3))
					return true;
			}
			auto& p2 = b[b.size() - 1];
			auto& p3 = b[0];
			if (isIntersectAbscisses(p0, p1, p2, p3))
				return true;
		}

		auto& p0 = a[a.size() - 1];
		auto& p1 = a[0];

		for (int j = 0; j < b.size() - 1; ++j)
		{
			auto& p2 = b[j];
			auto& p3 = b[j + 1];
			if (isIntersectAbscisses(p0, p1, p2, p3))
				return true;
		}
		auto& p2 = b[b.size() - 1];
		auto& p3 = b[0];
		return isIntersectAbscisses(p0, p1, p2, p3);
	}
	inline bool isIntersects(const Polygon& aa,const vec2& aPos,const Polygon& b, const  vec2& bPos)
	{
		Polygon a = aa.copy().plus(aPos - bPos);
		return isIntersects(a, b);
	}

	inline bool contains(const Polygon& a, const  vec2& point)
	{
		if (!a.getBounds().containsPoint(point))
			return false;
		if (a.isRectangle)
			return true;
		vec2 secPoint = {0,123451526789.f};
		int intersections = 0;
		for (int i = 0; i < a.size(); ++i)
		{
			auto& p0 = a[i];
			auto& p1 = a[(i + 1) % a.size()];


			if (isIntersectAbscisses(p0, p1, point, secPoint))
				++intersections;
		}
		return intersections & 1;
	}

	inline  vec2 intersects(const Polygon& a, const Polygon& b)
	{
		if (a.getBounds().intersects(b.getBounds())) //first check if it is even worth trying
		{
			for (int i = 0; i < a.size() - 1; ++i)
				//it is that big because not using modulo for p1,p2 (bigger code - bigger performance)
			{
				auto& p0 = a[i];
				auto& p1 = a[i + 1];

				for (int j = 0; j < b.size() - 1; ++j)
				{
					auto& p2 = b[j];
					auto& p3 = b[j + 1];
					auto out = intersectAbscisses(p0, p1, p2, p3);
					if (isValid(out))
						return out;
				}
				auto& p2 = b[b.size() - 1];
				auto& p3 = b[0];
				auto out = intersectAbscisses(p0, p1, p2, p3);
				if (isValid(out))
					return out;
			}

			auto& p0 = a[a.size() - 1];
			auto& p1 = a[0];

			for (int j = 0; j < b.size() - 1; ++j)
			{
				auto& p2 = b[j];
				auto& p3 = b[j + 1];
				auto out = intersectAbscisses(p0, p1, p2, p3);
				if (isValid(out))
					return out;
			}
			auto& p2 = b[b.size() - 1];
			auto& p3 = b[0];
			auto out = intersectAbscisses(p0, p1, p2, p3);
			if (isValid(out))
				return out;
		}
		return invalidVec2();
	}
}
