#pragma once

#include <glm/glm.hpp>
#include <utility>

inline glm::vec2 toglm(float f) { return glm::vec2(f, f); }

namespace Phys
{
	struct Polygon;

	inline static bool isValidFloat(float f)
	{
		return !std::isnan(f) && !std::isinf(f);
	}

	struct Vect
	{
		float x, y;

		Vect()
		{
		}

		Vect(float x, float y) : x(x), y(y)
		{
		}

		Vect(int x, int y) : x(x), y(y)
		{
		}

		Vect(int x, float y) : x(x), y(y)
		{
		}

		Vect(float x, int y) : x(x), y(y)
		{
		}

		Vect(float m) : x(m), y(m)
		{
		}

		Vect(const glm::vec2& m) : x(m.x), y(m.y)
		{
		}

		inline glm::vec2& asGLM() { return *((glm::vec2*)this); }

		inline float angleDegrees() const
		{
			if (x == 0) // special cases
				return (y > 0)
					       ? 90
					       : (y == 0)
					       ? 0
					       : 270;
			else if (y == 0) // special cases
				return (x >= 0)
					       ? 0
					       : 180;
			float ret = atanf((float)y / x) / 3.14159f * 180;
			if (x < 0 && y < 0) // quadrant Ⅲ
				ret = 180 + ret;
			else if (x < 0) // quadrant Ⅱ
				ret = 180 + ret; // it actually substracts
			else if (y < 0) // quadrant Ⅳ
				ret = 270 + (90 + ret); // it actually substracts
			return ret;
		}

		inline glm::vec2& operator()()
		{
			return asGLM();
		}

		inline const glm::vec2& operator()() const
		{
			return *(const glm::vec2*)this;
		}

		inline Vect operator+(const Vect& v) const
		{
			return {x + v.x, y + v.y};
		}

		inline Vect operator-(const Vect& v) const
		{
			return {x - v.x, y - v.y};
		}

		inline Vect operator-() const
		{
			return {-x, -y};
		}

		inline Vect operator*(const Vect& v) const
		{
			return {x * v.x, y * v.y};
		}

		inline Vect operator/(const Vect& v) const
		{
			return {x / v.x, y / v.y};
		}

		inline Vect operator*(float v) const
		{
			return {x * v, y * v};
		}

		inline Vect operator/(float v) const
		{
			return {x / v, y / v};
		}


		inline void operator*=(float v)
		{
			x *= v;
			y *= v;
		}

		inline void operator/=(float v)
		{
			x /= v;
			y /= v;
		}

		inline void operator+=(const Vect& v)
		{
			plus(v);
		}

		inline void plus(const Vect& v)
		{
			x += v.x;
			y += v.y;
		}

		inline void operator-=(const Vect& v)
		{
			minus(v);
		}

		inline void minus(const Vect& v)
		{
			x -= v.x;
			y -= v.y;
		}

		inline void operator*=(const Vect& v)
		{
			multiply(v);
		}

		inline void multiply(const Vect& v)
		{
			x *= v.x;
			y *= v.y;
		}

		inline void operator/=(const Vect& v)
		{
			divide(v);
		}

		inline void divide(const Vect& v)
		{
			x /= v.x;
			y /= v.y;
		}


		inline bool operator==(const Vect& other) const
		{
			return x == other.x && y == other.y;
		}

		inline bool operator!=(const Vect& other) const
		{
			return !operator==(other);
		}


		inline float lengthSquared() const
		{
			return x * x + y * y;
		}

		inline float length() const
		{
			return sqrt(lengthSquared());
		}

		inline float lengthCab() const
		{
			return abs(x) + abs(y);
		}

		inline float distanceSquared(const Vect& v) const
		{
			return (v - *this).lengthSquared();
		}

		inline float distance(const Vect& v) const
		{
			return (v - *this).length();
		}

		inline float distanceCab(const Vect& v) const
		{
			return (v - *this).lengthCab();
		}

		inline bool isValid() const { return isValidFloat(x) && isValidFloat(y); }
		inline Vect copy() const { return Vect(x, y); }

		inline static Vect invalid()
		{
			return Vect(std::numeric_limits<float>::quiet_NaN());
		}

		inline Vect& normalize()
		{
			float l = length();
			x /= l;
			y /= l;
			return *this;
		}
	};

	inline Vect vectFromAngle(float rad)
	{
		return { cos(rad),sin(rad) };
	}

	inline Vect clamp(const Vect& a, const Vect& min, const Vect& max)
	{
		Vect out;
		out.x = std::clamp(a.x, min.x, max.x);
		out.y = std::clamp(a.y, min.y, max.y);
		return out;
	}

	inline Vect operator+(float a, const Phys::Vect& b)
	{
		//outside the class
		return b + a;
	}

	inline Vect operator*(float a, const Phys::Vect& b)
	{
		//outside the class
		return b * a;
	}

	inline Vect& asVect(glm::vec2& v) { return *(Vect*)&v; }
	inline const Vect& asVect(const glm::vec2& v) { return *(Vect*)&v; }


	inline std::ostream& operator<<(std::ostream& os, const Vect& myObject)
	{
		if (myObject.isValid())
			os << "Vect[" << myObject.x << ", " << myObject.y << "]";
		else
			os << "Vect[invalid]";
		return os;
	}

	constexpr float toRad(float f)
	{
		return f / 180.f * 3.14159265f;
	}

	constexpr float toDeg(float f)
	{
		return f / 3.14159265f * 180.f;
	}

	struct Rectangle
	{
		float x0, y0, x1, y1;

		inline bool containsPoint(float x, float y) const
		{
			return (x >= x0) && (x <= x1) && (y >= y0) && (y <= y1);
		}

		inline float width() const { return x1 - x0; }
		inline float height() const { return y1 - y0; }

	private:
		inline bool intersectsInner(const Rectangle& rect) const
		{
			return
				rect.containsPoint(x0, y0) ||
				rect.containsPoint(x1, y0) ||
				rect.containsPoint(x0, y1) ||
				rect.containsPoint(x1, y1);
		}

	public:
		inline bool intersects(const Rectangle& rect) const
		{
			return intersectsInner(rect) || rect.intersectsInner(*this);
		}
	};

	struct Polygon
	{
		std::vector<Vect> vect;
		Vect* vects;
		int count;
	private:
		Rectangle bounds;
	public:

		Polygon(std::vector<Vect> v = {}): vect(std::move(v))
		{
			vects = vect.data();
			count = vect.size();
			recalculateBounds();
		}

		inline void recalculateBounds()
		{
			bounds.x0 = std::numeric_limits<float>::max();
			bounds.y0 = std::numeric_limits<float>::max();

			bounds.x1 = std::numeric_limits<float>::min();
			bounds.y1 = std::numeric_limits<float>::min();

			for (const auto& v : vect)
			{
				bounds.x0 = std::min(bounds.x0, v.x);
				bounds.x1 = std::max(bounds.x1, v.x);
				bounds.y0 = std::min(bounds.y0, v.y);
				bounds.y1 = std::max(bounds.y1, v.y);
			}
		}

		inline Vect& operator[](size_t index)
		{
			ASSERT(index < count&&index >= 0, "Invalid index");
			return vects[index];
		}

		inline const Vect& operator[](size_t index) const
		{
			ASSERT(index < count&&index >= 0, "Invalid index");
			return vects[index];
		}

		inline Polygon& plus(const Vect& v)
		{
			for (auto& vec : vect)
				vec.plus(v);
			recalculateBounds();
			return *this;
		}

		inline Polygon& minus(const Vect& v)
		{
			for (auto& vec : vect)
				vec.minus(v);
			recalculateBounds();
			return *this;
		}

		inline int size() const { return count; }

		inline const Rectangle& getBounds() const { return bounds; }

		inline Polygon copy() const
		{
			return Polygon(vect);
		}
	};

	inline Polygon toPolygon(const Rectangle& r)
	{
		return Polygon({
			{r.x0, r.y0},
			{r.x1, r.y0},
			{r.x1, r.y1},
			{r.x0, r.y1}
		});
	}

	// lines collisions ================================================================

	// returns the point of intersection between line a and b or if (no point or every point) -> invalid
	inline Vect intersectLines(const Vect& pa0, const Vect& pa1, const Vect& pb0, const Vect& pb1)
	{
		Vect deltaA = pa1 - pa0;
		Vect deltaB = pb1 - pb0;

		float t = (deltaB.x * (pb0.y - pa0.y) + deltaB.y * (pa0.x - pb0.x)) / (deltaB.x * deltaA.y - deltaA.x * deltaB.y
		);
		return pa0 + deltaA * t;
	}

	inline bool isBetween(float val, float bound0, float bound1)
	{
		return ((bound0 - val) * (val - bound1)) >= 0; //without branch prediction it should be faster...
	}

	// returns the point of intersection between abscisses a and b or if (no point or every point) -> invalid
	inline Vect intersectAbscisses(const Vect& pa0, const Vect& pa1, const Vect& pb0, const Vect& pb1)
	{
		Vect v = intersectLines(pa0, pa1, pb0, pb1);
		if (!v.isValid())
			return v;

		if (
			isBetween(v.x, pa0.x, pa1.x) &&
			isBetween(v.x, pb0.x, pb1.x) &&
			isBetween(v.y, pa0.y, pa1.y) &&
			isBetween(v.y, pb0.y, pb1.y))
			return v;
		return Vect::invalid();
	}

	// checks if abscisses a and b intersect at one point
	inline bool isIntersectAbscisses(const Vect& pa0, const Vect& pa1, const Vect& pb0, const Vect& pb1)
	{
		//todo add check if two abscisses touch at infinite points
		Vect v = intersectLines(pa0, pa1, pb0, pb1);
		if (!v.isValid())
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
		if (a.getBounds().intersects(b.getBounds())) //first check if it is even worth trying
		{
			for (int i = 0; i < a.size() - 1; ++i)
				//it is that big because not using modulo for p1,p2 (bigger code - bigger performance)
			{
				auto& p0 = a[i];
				auto& p1 = a[i + 1];

				for (int j = 0; j < b.size() - 1; ++j)
				{
					auto& p2 = a[j];
					auto& p3 = a[j + 1];
					if (isIntersectAbscisses(p0, p1, p2, p3))
						return true;
				}
				auto& p2 = a[b.size() - 1];
				auto& p3 = a[0];
				if (isIntersectAbscisses(p0, p1, p2, p3))
					return true;
			}

			auto& p0 = a[a.size() - 1];
			auto& p1 = a[0];

			for (int j = 0; j < b.size() - 1; ++j)
			{
				auto& p2 = a[j];
				auto& p3 = a[j + 1];
				if (isIntersectAbscisses(p0, p1, p2, p3))
					return true;
			}
			auto& p2 = a[b.size() - 1];
			auto& p3 = a[0];
			if (isIntersectAbscisses(p0, p1, p2, p3))
				return true;
		}
		return false;
	}

	inline bool contains(const Polygon& a, const Vect& point)
	{
		Vect secPoint = {std::numeric_limits<float>::max() / 2};
		int intersections = 0;
		for (int i = 0; i < a.size(); ++i)
			//it is that big because not using modulo for p1,p2 (bigger code - bigger performance)
		{
			auto& p0 = a[i];
			auto& p1 = a[(i + 1) % a.size()];


			if (isIntersectAbscisses(p0, p1, point, secPoint))
				++intersections;
		}
		return intersections & 1;
	}

	inline Vect intersects(const Polygon& a, const Polygon& b)
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
					auto& p2 = a[j];
					auto& p3 = a[j + 1];
					auto out = intersectAbscisses(p0, p1, p2, p3);
					if (out.isValid())
						return out;
				}
				auto& p2 = a[b.size() - 1];
				auto& p3 = a[0];
				auto out = intersectAbscisses(p0, p1, p2, p3);
				if (out.isValid())
					return out;
			}

			auto& p0 = a[a.size() - 1];
			auto& p1 = a[0];

			for (int j = 0; j < b.size() - 1; ++j)
			{
				auto& p2 = a[j];
				auto& p3 = a[j + 1];
				auto out = intersectAbscisses(p0, p1, p2, p3);
				if (out.isValid())
					return out;
			}
			auto& p2 = a[b.size() - 1];
			auto& p3 = a[0];
			auto out = intersectAbscisses(p0, p1, p2, p3);
			if (out.isValid())
				return out;
		}
		return Vect::invalid();
	}

	inline bool collideCircles(float x1, float y1, float rad1, float x2, float y2, float rad2)
	{
		return false;
	}
}
