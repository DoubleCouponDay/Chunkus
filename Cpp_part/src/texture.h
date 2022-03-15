#pragma once

#include <memory>
#include <string>

#include <GL/glut.h>

class Texture
{
	std::unique_ptr<unsigned char[]> _data;
	GLuint _width;
	GLuint _height;
public:
	Texture();
	Texture(std::string fileName, bool flipY);
	Texture(unsigned char* source, GLuint width, GLuint height);
	Texture(Texture&& other);
	Texture(const Texture& other) = delete;

	Texture& operator=(Texture&& other);
	Texture& operator=(const Texture& other) = delete;

	const unsigned char* getData() const;
	GLuint getWidth() const;
	GLuint getHeight() const;
};