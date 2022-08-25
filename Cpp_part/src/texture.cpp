#include "texture.h"

#include <iostream>
#include <stdio.h>
#include <exception>
#include <stdexcept>
#include <cstring>

#include <imagefile/bmp.h>
#include <image.h>
#include <imagefile/converter.h>

#include "gl.h"

class wrapped_c_img
{
	image c_img;
public:
	wrapped_c_img() = default;
	wrapped_c_img(image&& img) : c_img(img) {}
	wrapped_c_img(wrapped_c_img&& other) : c_img(other.c_img) { other.c_img.pixels_array_2d = nullptr; }
	~wrapped_c_img()
	{
		reset();
	}

	inline void reset()
	{
		if (c_img.pixels_array_2d)
		{
			free_image_contents(c_img);
			c_img.pixels_array_2d = nullptr;
		}
	}

	inline operator const image&() const { return c_img; }
	inline const image& get() const { return c_img; }
	inline operator bool() const { return c_img.pixels_array_2d != nullptr; }
};

RawPixelData loadPixelsFromC(const std::string& filename)
{
	RawPixelData data;
	data.data.resize(0);
	data.width = 0;
	data.height = 0;

	wrapped_c_img c_img(convert_file_to_image(filename.c_str()));
	if (!c_img)
	{
		throw std::runtime_error("Failed to load image. " + filename);
	}

	data.width = c_img.get().width;
	data.height = c_img.get().height;
	data.data.resize(data.width * data.height * 3);

	for (int x = 0; x < data.width; ++x)
	{
		for (int y = 0; y < data.height; ++y)
		{
			pixel p = c_img.get().pixels_array_2d[x][y];
			data.data[(y * data.width + x) * 3 + 0] = p.r;
			data.data[(y * data.width + x) * 3 + 1] = p.g;
			data.data[(y * data.width + x) * 3 + 2] = p.b;
		}
	}

	return data;
}

template<class ColorT>
Texture<ColorT>::Texture()
	: _data()
	, _width(0)
	, _height(0)
{
}

// Currently silently returns empty, should maybe throw instead
template<class ColorT>
Texture<ColorT>::Texture(std::string fileName)
{
	bool flipY = true;
	auto rawdata = loadPixelsFromC(fileName);

	_width = rawdata.width;
	_height = rawdata.height;

	_data.resize(_width * _height);
	
	for (int x = 0; x < _width; ++x)
	{
		for (int y = 0; y < _height; ++y)
		{
			int flippedY = flipY ? _height - y - 1 : y;
			int flippedIndex = flippedY * _width + x;
			int dataIndex = y * _width + x;
			int Rindex = dataIndex * 3;
			int Gindex = dataIndex * 3 + 1;
			int Bindex = dataIndex * 3 + 2;
			unsigned char R = rawdata.data[Rindex];
			unsigned char G = rawdata.data[Gindex];
			unsigned char B = rawdata.data[Bindex];
			_data[flippedIndex] = ColorT::fromRGB(R, G, B);
		}
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
	if (x < 0 || y < 0 || x + width > _width || y + height > _height ||
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

GLTexture::GLTexture() 
	: _texName(0)
	, _xScale(1)
	, _yScale(1)
	, _alphaTag(false)
{
}

GLTexture::GLTexture(const Texture8& tex, bool alpha) 
	: _texName(0)
	, _xScale(0)
	, _yScale(0)
	, _alphaTag(alpha)
{
	if (tex.getData() == nullptr || tex.getWidth() < 1 || tex.getHeight() < 1)
		return;

	checkForGlError("Creating GLTexture");
	GLuint myTex = 0;
	glGenTextures(1, &myTex);

	checkForGlError("Generated textures");
	glBindTexture(GL_TEXTURE_2D, myTex);

	checkForGlError("Bound texture");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	auto pow2width = nextPow2(tex.getWidth());
	auto pow2height = nextPow2(tex.getHeight());

	auto pow2Tex = Texture8{ Colors::Black8, pow2width, pow2height };
	pow2Tex.setArea(tex, 0, 0);

	_xScale = (float)tex.getWidth() / pow2width;
	_yScale = (float)tex.getHeight() / pow2height;
	
	if (_alphaTag)
	{
		std::vector<unsigned char> data(pow2width * pow2height * 4);
		for (int x = 0; x < pow2width; ++x)
		{
			for (int y = 0; y < pow2height; ++y)
			{
				auto color = pow2Tex.getPixel(x, y);
				data[(long long)y * pow2width * 4 + x * 4 + 0] = color.R;
				data[(long long)y * pow2width * 4 + x * 4 + 1] = color.G;
				data[(long long)y * pow2width * 4 + x * 4 + 2] = color.B;
				data[(long long)y * pow2width * 4 + x * 4 + 3] = (color.R > 0 && color.G > 0 && color.B > 0) ? 255 : 0;
			}
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pow2width, pow2height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)data.data());
	}
	else
	{	
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pow2width, pow2height, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)pow2Tex.getData());
	}

	checkForGlError("Set texture parameters");

	_texName = myTex;
}

GLTexture::GLTexture(GLTexture&& other)
	: _texName(other._texName)
	, _xScale(other._xScale)
	, _yScale(other._yScale)
	, _alphaTag(other._alphaTag)
{
	other._texName = 0;
	other._xScale = 1.f;
	other._yScale = 1.f;
	other._alphaTag = false;
}

GLTexture::~GLTexture()
{
	clear();
}

GLTexture& GLTexture::operator=(GLTexture&& other)
{
	clear();
	_texName = other._texName;
	_xScale = other._xScale;
	_yScale = other._yScale;

	other._texName = 0;
	other._xScale = 1.f;
	other._yScale = 1.f;

	return *this;
}

void GLTexture::clear()
{
	if (_texName)
	{
		glDeleteTextures(1, &_texName);
		_texName = 0;
	}
	_xScale = 1.f;
	_yScale = 1.f;
}

GLuint GLTexture::getName() const
{
	return _texName;
}

float GLTexture::getXScale() const
{
	return _xScale;
}

float GLTexture::getYScale() const
{
	return _yScale;
}

void GLTexture::bindTo(GLenum target) const
{
	glBindTexture(target, _texName);
}

WomboTexture::WomboTexture()
{
}

WomboTexture::WomboTexture(std::string fileName)
	: _cpuTex(fileName)
	, _glTex(_cpuTex)
{
}

WomboTexture::WomboTexture(GLTexture&& glTex, Texture8&& cpuTex)
	: _cpuTex(std::move(cpuTex))
	, _glTex(std::move(glTex))
{
}

WomboTexture::WomboTexture(Texture8&& cpuTex, bool isAlpha)
	: _cpuTex(std::move(cpuTex))
	, _glTex(_cpuTex, isAlpha)
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
		int flippedY = bitmap.height() - y - 1;
		auto data = rowData;
		for (int x = 0; x < bitmap.width(); ++x)
		{
			if (data[3] == 0)
				_cpuTex.setPixel(x, flippedY, Colors::Black8);
			else
				_cpuTex.setPixel(x, flippedY, Color8{ (unsigned char)data[0], (unsigned char)data[1], (unsigned char)data[2] });
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
	if (_glTex.isAlphaTag())
	{
		std::vector<unsigned char> data(width * height * 4);

		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				auto color = src.getPixel(x, y);
				data[(long long)y * width * 4 + x * 4 + 0] = color.R;
				data[(long long)y * width * 4 + x * 4 + 1] = color.G;
				data[(long long)y * width * 4 + x * 4 + 2] = color.B;
				data[(long long)y * width * 4 + x * 4 + 3] = (color.R > 0 && color.G > 0 && color.B > 0) ? 255 : 0;
			}
		}

		glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)data.data());
	}
	else
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _cpuTex.getWidth(), _cpuTex.getHeight(), GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)_cpuTex.getData());
}

void WomboTexture::setPixel(int x, int y, Color8 color)
{
	if (x < 0 || y < 0 || x >= _cpuTex.getWidth() || y >= _cpuTex.getHeight())
		throw std::invalid_argument("Can not set out of bounds pixel");

	_cpuTex.setPixel(x, y, color);

	_glTex.bindTo(GL_TEXTURE_2D);

	if (_glTex.isAlphaTag())
	{
		unsigned char dat[4] = { color.R, color.G, color.B, (color.R > 0 || color.G > 0 || color.B > 0) ? 255 : 0 };
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)dat);
	}
	else
	{
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &color);
	}
}

void WomboTexture::setArea(int x, int y, int width, int height, Color8 color)
{
	// Create temporary texture that contains just the color we are setting
	auto tmpTex = Texture8(color, width, height);

	updateImage(tmpTex, x, y, width, height);
}

