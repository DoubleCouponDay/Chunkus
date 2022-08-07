#pragma once

#include <string>

template<class T>
T vec_max(T a, T b)
{
    return a > b ? a : b;
}

struct Vector2u;

struct Vector2f
{
	Vector2f() = default;
	Vector2f(float x, float y) : x(x), y(y) {}
	Vector2f(const Vector2f& other) = default;

	inline Vector2f operator+(const Vector2f& other) const { return Vector2f(x + other.x, y + other.y); }
	inline Vector2f operator-(const Vector2f& other) const { return Vector2f(x - other.x, y - other.y); }
	inline Vector2f operator*(const Vector2f& other) const { return Vector2f(x * other.x, y * other.y); }
	inline Vector2f operator/(const Vector2f& other) const { return Vector2f(x / other.x, y / other.y); }
	inline Vector2f operator*(float scalar) const { return Vector2f(x * scalar, y * scalar); }
	inline Vector2f operator/(float scalar) const { return Vector2f(x / scalar, y / scalar); }

	float x;
	float y;
};

struct Vector2i
{
	Vector2i() = default;
	Vector2i(int x, int y) : x(x), y(y) {}
	int x;
	int y;

	inline Vector2i operator-() const { return { -x, -y }; }
	inline Vector2i operator-(const Vector2i& other) const { return Vector2i{ x - other.x, y - other.y }; }
	inline Vector2i operator+(const Vector2i& other) const { return Vector2i{ x + other.x, y + other.y }; }
	Vector2i operator-(const Vector2u& other) const;
	Vector2i operator+(const Vector2u& other) const;
};

struct Vector2u
{
	unsigned int x;
	unsigned int y;
};

inline Vector2i Vector2i::operator-(const Vector2u& other) const { return Vector2i{ x - (int)other.x, y - (int)other.y }; }
inline Vector2i Vector2i::operator+(const Vector2u& other) const { return Vector2i{ x + (int)other.x, y + (int)other.y }; }

struct Vector3i
{
	int x, y, z;

	inline std::string toString() const
	{
		char buf[80] = { 0 };

		auto len = snprintf(buf, sizeof(buf), "(%.2u, %.2u, %.2u)", x, y, z);
		return std::string(buf, buf + len);
	}
};

struct Box
{
	Box() {}
	Box(Vector2i low, Vector2i up) : lower(low), upper(up) {}
	Box(Vector2i lower, Vector2u bounds) : lower(lower), upper(lower.x + bounds.x, lower.y + bounds.y) {}
	Vector2i lower;
	Vector2i upper;

	inline Vector2i dimensions() const
	{
		return upper - lower;
	}

	inline int width() const { return dimensions().x; }
	inline int height() const { return dimensions().y; }
};