#pragma once

#include <vector>
#include <string>

#include <GL/freeglut.h>

#include <lunasvg.h>

#include "color.h"

struct RawPixelData
{
	std::vector<unsigned char> data;
	int width;
	int height;
};

RawPixelData loadPixelsFromC(const std::string& filename);

template<class ColorT>
class Texture
{
	std::vector<ColorT> _data;
	GLuint _width;
	GLuint _height;
public:
	Texture();
	Texture(std::string fileName, bool flipY);
	Texture(ColorT* source, GLuint width, GLuint height);
	Texture(ColorT color, GLuint width, GLuint height);
	Texture(Texture<ColorT>&& other);
	Texture(const Texture<ColorT>& other) = delete;

	Texture<ColorT>& operator=(Texture<ColorT>&& other);
	Texture<ColorT>& operator=(const Texture<ColorT>& other) = delete;

	const ColorT* getData() const;
	const unsigned char* getBytes() const;
	GLuint getWidth() const;
	GLuint getHeight() const;

	void clear();

	ColorT getPixel(int x, int y) const;
	void setPixel(int x, int y, ColorT color);
	void setArea(const Texture<ColorT>& other, int x, int y, int width, int height);
	void setArea(const Texture<ColorT>& other, int x, int y);

	void writeToBmp(const std::string& fileName) const;
};

using Texture8 = Texture<Color8>;
using Texture32 = Texture<Color32>;

class GLTexture
{
	GLuint _texName;
	float _xScale = 1.f;
	float _yScale = 1.f;
	bool _alphaTag = false;

public:
	GLTexture();
	GLTexture(const Texture8& tex, bool alphaTag = false);
	GLTexture(GLTexture&& other);
	GLTexture(const GLTexture& other) = delete;
	~GLTexture();

	GLTexture& operator=(GLTexture&& other);
	GLTexture& operator=(const GLTexture& other) = delete;

	void clear();

	GLuint getName() const;
	float getXScale() const;
	float getYScale() const;

	void bindTo(GLenum target) const;
	inline bool isAlphaTag() const { return _alphaTag; };
};

/// <summary>
/// Class that contains an OpenGL texture and a CPU Texture
/// </summary>
class WomboTexture
{
	// Ordering in this case is helpful, we want CPU Texture to load before the OpenGL texture as the OpenGL texture gets its data from the cpu texture
	Texture8 _cpuTex;
	GLTexture _glTex;
public:
	WomboTexture();
	WomboTexture(std::string fileName, bool flipY);
	WomboTexture(GLTexture&& glTex, Texture8&& cpuTex);
	WomboTexture(Texture8&& cpuTex, bool isAlphaTag = false);
	WomboTexture(const lunasvg::Bitmap& bitmap);
	WomboTexture(WomboTexture&& other);
	WomboTexture(const WomboTexture& other) = delete;
	~WomboTexture() = default;

	WomboTexture& operator=(WomboTexture&& other);
	WomboTexture& operator=(const WomboTexture& other) = delete;

	inline const Texture8& getCpuTex() const { return _cpuTex; }
	inline const GLTexture& getGLTex() const { return _glTex; }

	inline GLuint getName() const { return _glTex.getName(); }

	inline GLuint getWidth() const { return _cpuTex.getWidth(); }
	inline GLuint getHeight() const { return _cpuTex.getHeight(); }

	inline void bindTo(GLenum target) const { _glTex.bindTo(target); }

	void clear();


	// Meaty functions
	// These functions update internal cpu data, and the OpenGL representation

	void updateImage(const Texture8& src, int xoffset, int yoffset);
	void updateImage(const Texture8& src, int xoffset, int yoffset, int width, int height);
	void updateImage(); // Updates OpenGL texture with data from cpu texture

	void setPixel(int x, int y, Color8 color);
	void setArea(int x, int y, int width, int height, Color8 color);
};