#pragma once

#include <entrypoint.h>

#include <unordered_map>
#include <string>
#include <filesystem>
#include <memory>

#include <lunasvg.h>

#include "render.h"
#include "3d.h"
#include "c_wrappers.h"

enum class ActiveTexture
{
	INPUT = 0,
	INTERMEDIATE,
	VECTORIZED,
};

struct GUIData
{
	Vector2i windowSize = { 800, 600 };
	Vector3i lowLeft = { 0, 0, 0 }, lowRight = { 0, 0, 0, }, upRight = { 0, 0, 0 }, upLeft = { 0, 0, 0 };
	WomboTexture inputTexture;
	WomboTexture intermediateTexture;
	WomboTexture vectorizedTexture;
	Box textureArea;
	Color32 texColor = Colors::White32;

	Button quitButton;
	Button leftButton;
	Button rightButton;
	Button switchInputButton;
	Button switchInterButton;
	Button switchVectorButton;
	Button writeToBmpButton;

	algorithm_data algorithmData;
	visual_algorithm_data visuals;
	int selectedGroup = -1;
	int scrollage = 0;
	ActiveTexture activeTexture = ActiveTexture::INPUT;
	std::string statusString = "";

	std::vector<Button*> buttons;
	Sidebar sidebar;

	inline WomboTexture& getActiveTexture()
	{
		switch (activeTexture)
		{
		default:
		case ActiveTexture::INPUT: return inputTexture;
		case ActiveTexture::INTERMEDIATE: return intermediateTexture;
		case ActiveTexture::VECTORIZED: return vectorizedTexture;
		}
	}

	inline const char* getCurrentText() const
	{
		switch (activeTexture)
		{
		default:
		case ActiveTexture::INPUT: return "Input Image";
		case ActiveTexture::INTERMEDIATE: return "Intermediate";
		case ActiveTexture::VECTORIZED: return "Vectorized Output";
		}
	}
};
