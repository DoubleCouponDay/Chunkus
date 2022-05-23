#include "gui_data.h"

#include <imagefile/svg.h>

#include <lunasvg.h>

std::string gui_image_data::pathForStep(int stepNo)
{
	auto path = _fileDirectory / ("step: " + std::to_string(stepNo));
	return path.string();
}

gui_image_data::gui_image_data(gui_imagesp imagesp)
	: _imagesStart(imagesp)
	, _current(imagesp)
	, _currentIndex(0)
	, _fileDirectory(std::filesystem::current_path() / "tmp")
	, _svgFiles()
{
	int index = 0;
	std::string path_str = pathForStep(index);
	gui_imagesp cur = _imagesStart;
	while (cur->next != nullptr)
	{
		write_svg_file(cur->current, path_str.c_str());

		_svgFiles[cur->current] = path_str;

		path_str = pathForStep(index);

		index++;
	}
}

gui_image_data::~gui_image_data()
{
	if (_imagesStart)
	{
		free_gui_images(_imagesStart);
	}
}

std::unique_ptr<lunasvg::Document> gui_image_data::getCurrent()
{
	if (!_current || !_current->current)
		return nullptr;

	auto it = _svgFiles.find(_current->current);
	if (it == _svgFiles.end())
	{
		auto pathStr = pathForStep(_currentIndex);
		write_svg_file(_current->current, pathStr.c_str());

		if (!std::filesystem::exists(pathStr))
			return nullptr;

		_svgFiles[_current->current] = pathStr;

		it = _svgFiles.find(_current->current);
	}

	auto target = it->second;

	if (!std::filesystem::exists(target))
		return nullptr;


	auto doc = lunasvg::Document::loadFromFile(target);

	return doc;
}

void gui_image_data::prev()
{
	if (!_current)
		return;

	if (_current->previous)
	{
		_current = _current->previous;
		_currentIndex--;
	}
}

void gui_image_data::next()
{
	if (!_current)
		return;

	if (_current->next)
	{
		_current = _current->next;
		_currentIndex++;
	}
}

bool gui_image_data::isPrev()
{
	return _current && _current->previous;
}

bool gui_image_data::isNext()
{
	return _current && _current->next;
}
