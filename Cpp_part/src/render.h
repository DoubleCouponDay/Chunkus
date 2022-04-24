#pragma once

#include <string>
#include <GL/freeglut.h>

#include "color.h"
#include "texture.h"


struct Vector2i
{
	Vector2i() = default;
	Vector2i(int x, int y) : x(x), y(y) {}
	int x;
	int y;

	inline Vector2i operator-() const { return { -x, -y }; }
	inline Vector2i operator-(const Vector2i& other) const { return { x - other.x, y - other.y }; }
	inline Vector2i operator+(const Vector2i& other) const { return { x + other.x, y + other.y }; }
};

struct Vector2u
{
	unsigned int x;
	unsigned int y;
};

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

struct Button
{
	Vector2i position;
	Vector2u dimensions;
	std::string text;
	Color32 color;

	inline bool isWithin(Vector2i pos) const
	{
		return pos.x >= position.x
			&& pos.y >= position.y
			&& pos.x < position.x + dimensions.x
			&& pos.y < position.y + dimensions.y;
	}
};

void renderString(int x, int y, void* font, std::string str, Color32 color);
void renderString(Box box, void* font, std::string str, Color32 color);

void renderButton(const Button& button);

Vector2i windowToGL(Vector2i windowCoords);

void drawVecTextureArea(const GLTexture& tex, int texWidth, int texHeight, Vector3i translate, float scale, Box box);

