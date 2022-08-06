#pragma once

#include <string>
#include <vector>
#include <GL/freeglut.h>

#include "color.h"
#include "texture.h"
#include "3d.h"

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

struct visual_algorithm_data;

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

	int getButtonClicked(Vector2i pos) const;

	void render() const;
	void updateFromVisuals(const visual_algorithm_data& data);
};

void renderString(int x, int y, void* font, std::string str, Color32 color);
void renderString(Box box, void* font, std::string str, Color32 color);

void renderButton(const Button& button);

void renderArea(Box box, Color32 color);

void renderAlgorithm(const visual_algorithm_data& data, float scale, Box box, int selectedGroup);

Vector2i windowToGL(Vector2i windowCoords);

void drawVecTextureArea(const GLTexture& tex, int texWidth, int texHeight, Vector3i translate, float scale, Box box);

