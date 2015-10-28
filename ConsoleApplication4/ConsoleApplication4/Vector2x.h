#pragma once
#ifndef MXVECTOR2
#define MXVECTOR2
#include<memory>
#include<algorithm>
#include<math.h>


#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/fast_square_root.hpp>
#include <glm/gtx/rotate_vector.hpp>

#define MX_PI        3.14159265358979323846
#define MX_2PI       6.28318530717958647692

#undef max
#undef min

namespace MX
{


	class Vector2 : public glm::vec2
	{
	public:
		Vector2(const glm::vec2& v) : glm::vec2(v){}
		using glm::vec2::vec2;

		float length() const
		{
			return glm::length((glm::vec2&)*this);
		}

		float lengthSquared() const
		{
			return x*x + y*y;
		}

		auto& normalize()
		{
			glm::fastNormalize((glm::vec2&)*this);
			return *this;
		}

		
		auto vectorByNormalize() const
		{
			auto r = *this;
			if (r.zero())
				return r;
			r.normalize();
			return r;
		}


		Vector2 vectorByNormalize2() const
		{
			if (zero())
				return *this;
			return (*this) / sqrt(x*x + y*y);
		}

		Vector2 vectorLeftPerpendicular() const
		{
			return { -y, x };
		}

		Vector2 vectorRightPerpendicular() const
		{
			return { y, -x };
		}


		Vector2 vectorByRotation(float a) const
		{
			return glm::rotate(*this, a);
		}

		auto vectorByRotation2(float a) const
		{
			auto cs = cosf(a);
			auto sn = sinf(a);
			return Vector2(x * cs - y * sn, x * sn + y * cs);
		}



		static auto CreateVectorFromAngle(float a)
		{
			return Vector2(sinf(a), -cosf(a));
		}


		static auto lerp(const Vector2& a, const Vector2& b, float p)
		{
			return Vector2(a.x + (b.x - a.x)*p, a.y + (b.y - a.y)*p);
		}

		static auto vectorFromPointToPoint(const Vector2& a, const Vector2& b)
		{
			return Vector2(b.x - a.x, b.y - a.y);
		}

		static auto mean(const Vector2& a, const Vector2& b)
		{
			return Vector2((b.x + a.x) / 2.0f, (b.y + a.y) / 2.0f);
		}

		float angle() const
		{
			return atan2f(x, -y);
		}

		float angle2() const
		{
			return glm::orientedAngle(glm::vec2({x, -y}), glm::vec2({ 0,1 }));
		}


		bool zero() const
		{
			return x == 0.0f && y == 0.0f;
		}

		auto& approach_zero(float force)
		{
			auto vector = -(vectorByNormalize() * force);

			if (x >= 0.0f != (x + vector.x) >= 0.0f)
				x = 0.0f;
			else
				x += vector.x;

			if (y >= 0.0f != (y + vector.y) >= 0.0f)
				y = 0.0f;
			else
				y += vector.y;
			return *this;
		}

		void clampX(float min, float max)
		{
			x = std::max(x, min);
			x = std::min(x, max);
		}

		void clampY(float min, float max)
		{
			y = std::max(y, min);
			y = std::min(y, max);
		}

		template <typename T>
		void clampTo(const T& t)
		{
			clampX(t.x1, t.x2);
			clampY(t.y1, t.y2);
		}



		Vector2 operator+(const Vector2& v) const
		{
			return Vector2(x + v.x, y + v.y);
		}
		
		Vector2 operator-(const Vector2& v) const
		{
			return Vector2(x - v.x, y - v.y);
		}
		Vector2 operator-() const
		{
			return -(glm::vec2&)*this;
		}

		Vector2 operator*(float s) const
		{
			return (glm::vec2&)*this * s;
		}
		Vector2 operator/(float s) const
		{
			return (glm::vec2&)*this/s;
		}

		Vector2& operator+=(const Vector2& v)
		{
			x += v.x;
			y += v.y;
			return *this;
		}

		Vector2& operator-=(const Vector2& v)
		{
			x -= v.x;
			y -= v.y;
			return *this;
		}

		Vector2& operator*=(float s)
		{
			x *= s;
			y *= s;
			return *this;
		}

		Vector2& operator/=(float s)
		{
			x /= s;
			y /= s;
			return *this;
		}

		bool operator==(const Vector2& other) const
		{
			return x == other.x && y == other.y;
		}

		bool operator!=(const Vector2& other) const
		{
			return x != other.x || y != other.y;
		}

	};


	inline float distanceBetween(const Vector2& vector1, const Vector2& vector2)
	{
		return glm::fastDistance((glm::vec2&)vector1, (glm::vec2&)vector2);
	}

	inline float distanceBetween2(const Vector2& vector1, const Vector2& vector2)
	{
		float a = vector2.x - vector1.x;
		float b = vector2.y - vector1.y;
		return sqrt(a*a + b*b);
	}
}

#endif
