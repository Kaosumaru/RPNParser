#pragma once
#ifndef MXVector2x
#define MXVector2x
#include<memory>
#include<algorithm>
#include<math.h>
#define MX_PI        3.14159265358979323846
#define MX_2PI       6.28318530717958647692

#undef max
#undef min

namespace MX
{


	class Vector2x
	{
	public:
		float x = 0.0f, y = 0.0f;
		Vector2x() {}
		explicit Vector2x(float d) : x(d), y(d) {}
		Vector2x(float x, float y) : x(x), y(y) {}

		Vector2x operator+(const Vector2x& v) const
		{
			return Vector2x(x + v.x, y + v.y);
		}
		Vector2x operator-(const Vector2x& v) const
		{
			return Vector2x(x - v.x, y - v.y);
		}
		Vector2x operator-() const
		{
			return Vector2x(-x, -y);
		}
		Vector2x operator*(float s) const
		{
			return Vector2x(x*s, y*s);
		}
		Vector2x operator/(float s) const
		{
			return Vector2x(x / s, y / s);
		}

		Vector2x& operator+=(const Vector2x& v)
		{
			x += v.x;
			y += v.y;
			return *this;
		}

		Vector2x& operator-=(const Vector2x& v)
		{
			x -= v.x;
			y -= v.y;
			return *this;
		}

		Vector2x& operator*=(float s)
		{
			x *= s;
			y *= s;
			return *this;
		}

		Vector2x& operator/=(float s)
		{
			x /= s;
			y /= s;
			return *this;
		}

		bool operator==(const Vector2x& other) const
		{
			return x == other.x && y == other.y;
		}

		bool operator!=(const Vector2x& other) const
		{
			return x != other.x || y != other.y;
		}

		float length() const
		{
			return sqrt(x*x + y*y);
		}

		float lengthSquared() const
		{
			return x*x + y*y;
		}

		Vector2x& normalize()
		{
			if (!zero())
				(*this) /= length();
			return *this;
		}

		Vector2x vectorByNormalize() const
		{
			if (zero())
				return *this;
			return (*this) / length();
		}

		Vector2x vectorByRotation(float a) const
		{
			auto cs = cosf(a);
			auto sn = sinf(a);
			return Vector2x(x * cs - y * sn, x * sn + y * cs);

			//return CreateVectorFromAngle(angle() + a) * length();
		}

		Vector2x vectorLeftPerpendicular() const
		{
			return Vector2x(-y, x);
		}

		Vector2x vectorRightPerpendicular() const
		{
			return Vector2x(y, -x);
		}

		static Vector2x CreateVectorFromAngle(float a)
		{
			return Vector2x(sinf(a), -cosf(a));
		}

		static Vector2x lerp(const Vector2x& a, const Vector2x& b, float p)
		{
			return Vector2x(a.x + (b.x - a.x)*p, a.y + (b.y - a.y)*p);
		}

		static Vector2x vectorFromPointToPoint(const Vector2x& a, const Vector2x& b)
		{
			return Vector2x(b.x - a.x, b.y - a.y);
		}

		static Vector2x mean(const Vector2x& a, const Vector2x& b)
		{
			return Vector2x((b.x + a.x) / 2.0f, (b.y + a.y) / 2.0f);
		}

		float angle() const
		{
			return atan2f(x, -y);
		}

		bool zero() const
		{
			return x == 0.0f && y == 0.0f;
		}


		Vector2x& approach_zero(float force)
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
	};

	inline float VectorDotProduct(const Vector2x& a, const Vector2x& b)
	{
		return a.x*b.x + a.y*b.y;
	}

	inline float VectorCrossProduct(const Vector2x& v1, const Vector2x& v2)
	{
		return (v1.x*v2.y) - (v1.y*v2.x);
	}


	template <typename T>
	T angle_difference(T firstAngle, T secondAngle)
	{
		T difference = secondAngle - firstAngle;

		while (difference < -MX_PI)
			difference = difference + (T)(2.*MX_PI);

		while (difference > MX_PI)
			difference = difference - (T)(2.*MX_PI);

		return difference;
	}

	inline float angle_difference(const Vector2x& a, const Vector2x& b)
	{
		return angle_difference(a.angle(), b.angle());
	}

	//rotates vector by value no larger than maxAngle to match targetVector, and return angle of new vector
	inline float rotateVectorToTarget(const Vector2x& targetVector, Vector2x& vector, float maxAngle)
	{
		float targetAngle = targetVector.angle();
		float angle = vector.angle();
		float angleDiff = angle_difference(targetAngle, angle);
		if (fabs(angleDiff) <= maxAngle)
		{
			vector = targetVector;
			return targetAngle;
		}

		float target_angle = angleDiff > 0.f ? angle - maxAngle : angle + maxAngle;
		vector = Vector2x::CreateVectorFromAngle(target_angle) * targetVector.length();
		return target_angle;
	}

	inline float rotateVectorToTarget(float targetAngle, Vector2x& vector, float maxAngle)
	{
		float angle = vector.angle();
		float angleDiff = angle_difference(targetAngle, angle);
		if (fabs(angleDiff) <= maxAngle)
		{
			vector = Vector2x::CreateVectorFromAngle(targetAngle);
			return targetAngle;
		}

		float target_angle = angleDiff > 0.f ? angle - maxAngle : angle + maxAngle;
		vector = Vector2x::CreateVectorFromAngle(target_angle);
		return target_angle;
	}

	inline bool distanceBetweenPointsLowerThan(const Vector2x& vector1, const Vector2x& Vector2x, float distance)
	{
		float a = Vector2x.x - vector1.x;
		float b = Vector2x.y - vector1.y;
		float c = distance;
		return a*a + b*b <= c*c;

	}

	inline float distanceBetween(const Vector2x& vector1, const Vector2x& Vector2x)
	{
		float a = Vector2x.x - vector1.x;
		float b = Vector2x.y - vector1.y;
		return sqrt(a*a + b*b);
	}

	inline float distanceBetweenSquared(const Vector2x& vector1, const Vector2x& Vector2x)
	{
		float a = Vector2x.x - vector1.x;
		float b = Vector2x.y - vector1.y;
		return a*a + b*b;
	}


}

#endif
