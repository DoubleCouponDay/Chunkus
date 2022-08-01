#pragma once

#include <string>
#include <vector>
#include <GL/freeglut.h>

#include "color.h"
#include "texture.h"

struct Vector2u;

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

inline Vector2i Vector2i::operator-(const Vector2u& other) const { return Vector2i{ x - other.x, y - other.y }; }
inline Vector2i Vector2i::operator+(const Vector2u& other) const { return Vector2i{ x + other.x, y + other.y }; }

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

struct SidebarButton
{
	Vector2u dimensions;
	std::string text;
	Color32 textColor;
	Color32 GroupColor;

	inline Button asButton(Vector2i pos) const
	{
		return { pos, Vector2u{ dimensions.x - 37, dimensions.y }, text, textColor };
	}
};

class Sidebar
{
	std::vector<SidebarButton> Buttons;
	Box Bounds;
	Color32 BackgroundColor;
	int margin = 5;
	int spacing = 4;

	void UpdateBounds();
public:
	Sidebar(std::vector<SidebarButton> buttons = {});
	Sidebar(Box bounds, std::vector<SidebarButton> buttons = {}, Color32 backgroundColor = Colors::Black32, int margin = 5, int spacing = 4);

	void addButton(SidebarButton button);

	inline bool isWithin(Vector2i pos) const
	{
		return pos.x >= Bounds.lower.x
			&& pos.y >= Bounds.lower.y
			&& pos.x < Bounds.upper.x
			&& pos.y < Bounds.upper.y;
	}

	inline int getButtonClicked(Vector2i pos) const
	{
		auto runningPos = Vector2i{ Bounds.lower.x, Bounds.upper.y } + Vector2i{ margin, -margin };

		auto withinButton = [](const SidebarButton& b, Vector2i p, Vector2i runningPos)
		{
			return p.x >= runningPos.x && p.y >= (runningPos.y - b.dimensions.y) && p.x < (runningPos.x + b.dimensions.x) && p.y < runningPos.y;
		};

		for (int i = 0; i < Buttons.size(); i++)
		{
			if (withinButton(Buttons[i], pos, runningPos))
			{
				return i;
			}
			runningPos.y -= Buttons[i].dimensions.y + spacing;
		}
		return -1;
	}

	void render() const;
};

void renderString(int x, int y, void* font, std::string str, Color32 color);
void renderString(Box box, void* font, std::string str, Color32 color);

void renderButton(const Button& button);

void renderArea(Box box, Color32 color);

Vector2i windowToGL(Vector2i windowCoords);

void drawVecTextureArea(const GLTexture& tex, int texWidth, int texHeight, Vector3i translate, float scale, Box box);

