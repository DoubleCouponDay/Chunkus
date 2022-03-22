#pragma once

#include <entrypoint.h>

#include "color.h"
#include "texture.h"
#include "render.h"

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
	Button button1;
	Button button2;
	Button button3;
	Button button4;
	Button leftButton;
	Button rightButton;
	vectorizer_data data;
	int scrollage = 0;
	ActiveTexture activeTexture = ActiveTexture::INPUT;
	std::string statusString = "Yo what's up";

	std::vector<Button*> buttons;

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

extern GUIData myData;