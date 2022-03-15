#include "texture.h"

#include <stdio.h>

Texture::Texture()
{
}

// Currently silently returns empty, should maybe throw instead
Texture::Texture(std::string fileName, bool flipY)
{
	_width = -1;
	_height = -1;
	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;


	FILE* file = fopen(fileName.c_str(), "rb");
	if (!file) { printf("Image could not be opened\n"); return; }


	if (fread(header, 1, 54, file) != 54) {
		printf("Not a correct BMP file\n");
		return;
	}

	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		return;
	}
	if (*(int*)&(header[0x1E]) != 0) { printf("Not a correct BMP file\n");		return; }
	if (*(int*)&(header[0x1C]) != 24) { printf("Not a correct BMP file\n");		return; }


	dataPos = *(int*)&(header[10]);
	imageSize = *(int*)&(header[34]);
	_width = *(int*)&(header[18]);
	_height = *(int*)&(header[22]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize == 0)    imageSize = _width * _height * 3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos == 0)      dataPos = 54; // The BMP header is done that way

	// Create a buffer
	_data = std::make_unique<unsigned char[]>(imageSize);

	// Read the actual data from the file into the buffer
	fread(_data.get(), 1, imageSize, file);

	// Everything is in memory now, the file wan be closed
	fclose(file);

	if (flipY) {
		// swap y-axis
		unsigned char* tmpBuffer = new unsigned char[_width * 3];
		int size = _width * 3;
		for (int i = 0; i < _height / 2; i++) {

			memcpy(tmpBuffer, _data.get() + _width * 3 * i, size);

			memcpy(_data.get() + _width * 3 * i, _data.get() + _width * 3 * (_height - i - 1), size);

			memcpy(_data.get() + _width * 3 * (_height - i - 1), tmpBuffer, size);
		}
		delete[] tmpBuffer;
	}
}

Texture::Texture(unsigned char* source, GLuint width, GLuint height)
	: _data(source)
	, _width(width)
	, _height(height)
{
}

Texture::Texture(Texture&& other) 
	: _data(std::move(other._data))
	, _width(other._width)
	, _height(other._height)
{
}

Texture& Texture::operator=(Texture&& other)
{
	_data = std::move(other._data);
	_width = other._width;
	_height = other._height;

	other._width = 0;
	other._height = 0;

	return *this;
}

const unsigned char* Texture::getData() const
{
	return _data.get();
}

GLuint Texture::getWidth() const
{
	return _width;
}

GLuint Texture::getHeight() const
{
	return _height;
}
