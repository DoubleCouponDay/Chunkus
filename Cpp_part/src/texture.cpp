#include "texture.h"

#include <iostream>
#include <stdio.h>
#include <exception>
#include <stdexcept>
#include <cstring>

#include <imagefile/bmp.h>
#include <image.h>

#include "gl.h"


template<class ColorT>
Texture<ColorT>::Texture()
	: _data()
	, _width(0)
	, _height(0)
{
}

// Currently silently returns empty, should maybe throw instead
template<class ColorT>
Texture<ColorT>::Texture(std::string fileName, bool flipY)
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
	if (imageSize == 0)    imageSize = _width * _height * 3; // 3 Bytes for red, green and blue
	if (dataPos == 0)      dataPos = 54; // The BMP header is done that way

	// Create a buffer
	_data.resize((size_t)_width * _height); // Divide by 3 as our ColorT type should contain all 3 colors

	// Read the actual data from the file into the buffer
	fread(_data.data(), 1, imageSize, file);

	// Everything is in memory now, the file wan be closed
	fclose(file);

	if (flipY) {
		// swap y-axis
		unsigned char* tmpBuffer = new unsigned char[_width * 3];
		int size = _width * 3;
		for (int i = 0; i < _height / 2; i++) {

			memcpy(tmpBuffer, _data.data() + _width * 3 * i, size);

			memcpy(_data.data() + _width * 3 * i, _data.data() + _width * 3 * (_height - i - 1), size);

			memcpy(_data.data() + _width * 3 * (_height - i - 1), tmpBuffer, size);
		}
		delete[] tmpBuffer;
	}
}

template<class ColorT>
Texture<ColorT>::Texture(ColorT* source, GLuint width, GLuint height)
	: _data(source, source + width * height)
	, _width(width)
	, _height(height)
{
}

template<class ColorT>
Texture<ColorT>::Texture(ColorT color, GLuint width, GLuint height)
	: _data((unsigned long long)width * height)
	, _width(width)
	, _height(height)
{
	for (int i = 0; i < _width * _height; ++i)
		_data[i] = color;
}

template<class ColorT>
Texture<ColorT>::Texture(Texture<ColorT>&& other)
	: _data(std::move(other._data))
	, _width(other._width)
	, _height(other._height)
{
}

template<class ColorT>
Texture<ColorT>& Texture<ColorT>::operator=(Texture<ColorT>&& other)
{
	_data = std::move(other._data);
	_width = other._width;
	_height = other._height;

	other._width = 0;
	other._height = 0;

	return *this;
}

template<class ColorT>
const ColorT* Texture<ColorT>::getData() const
{
	return _data.data();
}

template<class ColorT>
const unsigned char* Texture<ColorT>::getBytes() const
{
	return reinterpret_cast<const unsigned char*>(_data.data());
}

template<class ColorT>
GLuint Texture<ColorT>::getWidth() const
{
	return _width;
}

template<class ColorT>
GLuint Texture<ColorT>::getHeight() const
{
	return _height;
}

template<class ColorT>
void Texture<ColorT>::clear()
{
	_data.clear();
	_width = 0;
	_height = 0;
}

template<class ColorT>
ColorT Texture<ColorT>::getPixel(int x, int y) const
{
	return _data[(long long)_width * y + x];
}

template<class ColorT>
void Texture<ColorT>::setPixel(int x, int y, ColorT color)
{
	memcpy(&_data[(long long)y * _width + x], &color, sizeof(color));
}

template<class ColorT>
void Texture<ColorT>::setArea(const Texture<ColorT>& other, int x, int y)
{
	setArea(other, x, y, other.getWidth(), other.getHeight());
}

template<class ColorT>
void Texture<ColorT>::writeToBmp(const std::string& fileName) const
{
	image c_img = create_image(_width, _height);

	for (int x = 0; x < _width; ++x)
	{
		for (int y = 0; y < _height; ++y)
		{
			c_img.pixels_array_2d[x][y].r = (unsigned char)getPixel(x, y).R;
			c_img.pixels_array_2d[x][y].g = (unsigned char)getPixel(x, y).G;
			c_img.pixels_array_2d[x][y].b = (unsigned char)getPixel(x, y).B;
		}
	}

	write_image_to_bmp(c_img, fileName.c_str());

	free_image_contents(c_img);
}


template<class ColorT>
void Texture<ColorT>::setArea(const Texture<ColorT>& other, int x, int y, int width, int height)
{
	if (x < 0 || y < 0 || x + width >= _width || y + height >= _height ||
		width > other.getWidth() || height > other.getHeight())
		throw std::invalid_argument("Invalid X/Y or Width/Height given to Texture::setArea");

	for (int x2 = 0; x2 < width; ++x2)
	{
		for (int y2 = 0; y2 < height; ++y2)
		{
			setPixel(x2 + x, y2 + y, other.getPixel(x2, y2));
		}
	}
}

template class Texture<Color32>;

template class Texture<Color8>;

unsigned int nextPow2(unsigned int v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

GLTexture::GLTexture() : _texName(0)
{
}

GLTexture::GLTexture(const Texture8& tex) : _texName(0)
{
	if (tex.getData() == nullptr || tex.getWidth() < 1 || tex.getHeight() < 1)
		return;

	checkForGlError("Creating GLTexture");
	GLuint myTex = 0;
	glGenTextures(1, &myTex);

	checkForGlError("Generated textures");
	glBindTexture(GL_TEXTURE_2D, myTex);

	checkForGlError("Bound texture");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	auto pow2width = nextPow2(tex.getWidth());
	auto pow2height = nextPow2(tex.getHeight());

	auto pow2Tex = Texture8{ Colors::Black8, pow2width, pow2height };
	pow2Tex.setArea(tex, 0, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pow2width, pow2height, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)pow2Tex.getData());

	checkForGlError("Set texture parameters");

	_texName = myTex;
}

GLTexture::GLTexture(GLTexture&& other)
	: _texName(other._texName)
{
	other._texName = 0;
}

GLTexture::~GLTexture()
{
	clear();
}

GLTexture& GLTexture::operator=(GLTexture&& other)
{
	clear();
	_texName = other._texName;

	other._texName = 0;

	return *this;
}

void GLTexture::clear()
{
	if (_texName)
	{
		glDeleteTextures(1, &_texName);
		_texName = 0;
	}
}

GLuint GLTexture::getName() const
{
	return _texName;
}

void GLTexture::bindTo(GLenum target) const
{
	glBindTexture(target, _texName);
}

WomboTexture::WomboTexture()
{
}

WomboTexture::WomboTexture(std::string fileName, bool flipY)
	: _cpuTex(fileName, flipY)
	, _glTex(_cpuTex)
{
}

WomboTexture::WomboTexture(GLTexture&& glTex, Texture8&& cpuTex)
	: _cpuTex(std::move(cpuTex))
	, _glTex(std::move(glTex))
{
}

WomboTexture::WomboTexture(Texture8&& cpuTex)
	: _cpuTex(std::move(cpuTex))
	, _glTex(_cpuTex)
{
}

WomboTexture::WomboTexture(const lunasvg::Bitmap& bitmap)
	: _cpuTex(Colors::Black8, bitmap.width(), bitmap.height())
	, _glTex()
{
	auto stride = bitmap.stride();

	auto rowData = bitmap.data();
	for (int y = 0; y < bitmap.height(); ++y)
	{
		auto data = rowData;
		for (int x = 0; x < bitmap.width(); ++x)
		{
			if (data[3] == 0)
				_cpuTex.setPixel(x, y, Colors::Black8);
			else
				_cpuTex.setPixel(x, y, Color8{ (unsigned char)data[2], (unsigned char)data[1], (unsigned char)data[0] });
			data += 4;
		}
		rowData += stride;
	}

	_glTex = GLTexture(_cpuTex);
}

WomboTexture::WomboTexture(WomboTexture&& other)
	: _cpuTex(std::move(other._cpuTex))
	, _glTex(std::move(other._glTex))
{
}

WomboTexture& WomboTexture::operator=(WomboTexture&& other)
{
	clear();

	_glTex = std::move(other._glTex);
	_cpuTex = std::move(other._cpuTex);

	return *this;
}

void WomboTexture::clear()
{
	_glTex.clear();
	_cpuTex.clear();
}

void WomboTexture::updateImage(const Texture8& src, int xoffset, int yoffset)
{
	updateImage(src, xoffset, yoffset, src.getWidth(), src.getHeight());
}

void WomboTexture::updateImage(const Texture8& src, int xoffset, int yoffset, int width, int height)
{
	if (xoffset < 0 || yoffset < 0 ||
		xoffset + width >= _cpuTex.getWidth() ||
		yoffset + height >= _cpuTex.getHeight())
		throw std::invalid_argument("Invalid X/Y or Width/Height given to WomboTexture::updateImage");

	_cpuTex.setArea(src, xoffset, yoffset, width, height);

	_glTex.bindTo(GL_TEXTURE_2D);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)src.getData());
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _cpuTex.getWidth(), _cpuTex.getHeight(), GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)_cpuTex.getData());
}

void WomboTexture::setPixel(int x, int y, Color8 color)
{
	if (x < 0 || y < 0 || x >= _cpuTex.getWidth() || y >= _cpuTex.getHeight())
		throw std::invalid_argument("Can not set out of bounds pixel");

	_cpuTex.setPixel(x, y, color);

	_glTex.bindTo(GL_TEXTURE_2D);
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &color);
}

void WomboTexture::setArea(int x, int y, int width, int height, Color8 color)
{
	// Create temporary texture that contains just the color we are setting
	auto tmpTex = Texture8(color, width, height);

	updateImage(tmpTex, x, y, width, height);
}

