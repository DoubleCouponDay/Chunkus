#pragma once

#include <string>
#include <exception>

#include <GL/glut.h>


class GLMatrix
{
private:
	GLenum _matMode;
public:
	GLMatrix(GLenum matrixMode) : _matMode(matrixMode) { glMatrixMode(matrixMode); glPushMatrix(); glLoadIdentity(); }
	~GLMatrix() { glMatrixMode(_matMode); glPopMatrix(); }
};

char const* glErrorString(GLenum const err) noexcept;

bool checkForGlError(std::string description);

class GLException : public std::exception
{
	std::string _err;
public:
	GLException(std::string err) : _err(std::move(err)) {}

	inline const char* what() const override
	{
		return _err.c_str();
	}
};