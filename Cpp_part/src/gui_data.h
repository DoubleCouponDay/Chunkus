#pragma once

#include <entrypoint.h>

#include <unordered_map>
#include <string>
#include <filesystem>
#include <memory>

#include <lunasvg.h>

#include "render.h"
#include "3d.h"

enum class ActiveTexture
{
	INPUT = 0,
	INTERMEDIATE,
	VECTORIZED,
};

struct GUIData
{
	// For Vectorizer
	vectorize_options options;
	std::string file_path; // Store a string so that the const char* pointer in options is valid.

	// For Rendering
	Vector2i windowSize = { 800, 600 };

	// 3 Textures, one for ActiveTexture::INPUT, one for ActiveTexture::INTERMEDIATE, one for ActiveTexture::VECTORIZED
	WomboTexture inputTexture;
	WomboTexture intermediateTexture;
	WomboTexture vectorizedTexture;
	ActiveTexture activeTexture = ActiveTexture::INPUT;

	Box textureArea;
	Color32 texColor = Colors::White32;

	Button quitButton;
	Button leftButton;
	Button rightButton;
	Button switchInputButton;
	Button switchInterButton;
	Button switchVectorButton;
	Button writeToBmpButton;

	TextField inputField;
	bool inputFieldSelected;


	int scrollage = 0;
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
