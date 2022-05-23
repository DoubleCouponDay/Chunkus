#pragma once

#include <entrypoint.h>

#include <unordered_map>
#include <string>
#include <filesystem>
#include <memory>

#include <lunasvg.h>

class gui_image_data
{
	gui_imagesp _imagesStart;

	gui_imagesp _current;

	int _currentIndex = 0;

	std::filesystem::path _fileDirectory;
	std::unordered_map<NSVGimage*, std::string> _svgFiles;

	std::string pathForStep(int stepNo);

public:
	gui_image_data(gui_imagesp images);
	~gui_image_data();

	std::unique_ptr<lunasvg::Document> getCurrent();

	void prev();
	void next();

	bool isPrev();
	bool isNext();
};