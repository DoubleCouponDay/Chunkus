#pragma once

#include <vector>
#include <string>

#include <GL/freeglut.h>

struct Color8
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
};

struct Color32
{
	float R;
	float G;
	float B;
};

namespace Colors
{
	constexpr Color8 Red8 = { 0xff, 0x00, 0x00 };
	constexpr Color8 Green8 = { 0x00, 0xff, 0x00 };
	constexpr Color8 Blue8 = { 0x00, 0x00, 0xff };
	constexpr Color8 Orange8 = { 0xb0, 0x40, 0x10 };
}

template<class TStorage>
class Texture
{
	std::vector<TStorage> _data;
	GLuint _width;
	GLuint _height;
public:
	Texture();
	Texture(std::string fileName, bool flipY);
	Texture(TStorage* source, GLuint width, GLuint height);
	Texture(TStorage color, GLuint width, GLuint height);
	Texture(Texture<TStorage>&& other);
	Texture(const Texture<TStorage>& other) = delete;

	Texture<TStorage>& operator=(Texture<TStorage>&& other);
	Texture<TStorage>& operator=(const Texture<TStorage>& other) = delete;

	const TStorage* getData() const;
	const unsigned char* getBytes() const;
	GLuint getWidth() const;
	GLuint getHeight() const;

	void clear();

	TStorage getPixel(int x, int y) const;
	void setPixel(int x, int y, TStorage color);
	void setArea(const Texture<TStorage>& other, int x, int y, int width, int height);
	void setArea(const Texture<TStorage>& other, int x, int y);
};

using Texture8 = Texture<Color8>;
using Texture32 = Texture<Color32>;

class GLTexture
{
	GLuint _texName;

public:
	GLTexture();
	GLTexture(const Texture8& tex);
	GLTexture(GLTexture&& other);
	GLTexture(const GLTexture& other) = delete;
	~GLTexture();

	GLTexture& operator=(GLTexture&& other);
	GLTexture& operator=(const GLTexture& other) = delete;

	void clear();

	GLuint getName() const;

	void bindTo(GLenum target) const;
};

class WomboTexture
{
	// Ordering in this case is helpful, we want CPU Texture to load before the OpenGL texture as the OpenGL texture gets its data from the cpu texture
	Texture8 _cpuTex;
	GLTexture _glTex;
public:
	WomboTexture();
	WomboTexture(std::string fileName, bool flipY);
	WomboTexture(GLTexture&& glTex, Texture8&& cpuTex);
	WomboTexture(WomboTexture&& other);
	WomboTexture(const WomboTexture& other) = delete;
	~WomboTexture() = default;

	WomboTexture& operator=(WomboTexture&& other);
	WomboTexture& operator=(const WomboTexture& other) = delete;

	inline const Texture8& getCpuTex() const { return _cpuTex; }
	inline const GLTexture& getGLTex() const { return _glTex; }

	inline GLuint getName() const { return _glTex.getName(); }

	inline void bindTo(GLenum target) const { _glTex.bindTo(target); }

	void clear();


	// Meaty functions
	// These functions update internal cpu data, and the OpenGL representation

	void updateImage(const Texture8& src, int xoffset, int yoffset);
	void updateImage(const Texture8& src, int xoffset, int yoffset, int width, int height);

	void setPixel(int x, int y, Color8 color);
	void setArea(int x, int y, int width, int height, Color8 color);
};