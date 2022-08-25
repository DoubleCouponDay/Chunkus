#include "render.h"

#include "gl.h"

#include "gui_data.h"

void renderString(int x, int y, void* font, std::string str, Color32 color)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(1.f, 1.f, 1.f);
	glRasterPos2i(x, y);

	glutBitmapString(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)str.c_str());
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void renderString(Box box, void* font, std::string str, Color32 color)
{
	int yMargin = box.height() - glutBitmapHeight(font);
	int xMargin = box.width() - glutBitmapLength(font, (const unsigned char*)str.c_str());

	renderString(box.lower.x + xMargin / 2, box.lower.y + yMargin / 2, font, str, color);
}

void renderButton(const Button& button, Vector2i windowSize)
{
	checkForGlError("Before button rendering");
	glScissor(button.position.x, button.position.y, button.dimensions.x, button.dimensions.y); // Scissor Rect

	{
		auto mat = GLMatrix(GL_MODELVIEW);


		glColor3f(button.color.R, button.color.G, button.color.B);
		checkForGlError("Set color");

		glBegin(GL_QUADS);
		
		// Counter Clockwise order
		glVertex3i(button.position.x + 0                  , button.position.y + 0                  , 0); // Lower Left
		glVertex3i(button.position.x + button.dimensions.x, button.position.y + 0                  , 0); // Lower Right
		glVertex3i(button.position.x + button.dimensions.x, button.position.y + button.dimensions.y, 0); // Upper Right
		glVertex3i(button.position.x + 0                  , button.position.y + button.dimensions.y, 0); // Upper Left

		glEnd();
		checkForGlError("After button rendering");
	}

	int yMargin = (button.dimensions.y - glutBitmapHeight(GLUT_BITMAP_HELVETICA_18));
	int xMargin = (button.dimensions.x - glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)button.text.c_str())) / 2;
	renderString(button.position.x + xMargin, button.position.y + yMargin, GLUT_BITMAP_HELVETICA_18, button.text, Colors::White32);
	checkForGlError("After button string");

	glScissor(0, 0, windowSize.x, windowSize.y);
}

int Sidebar::getButtonClicked(Vector2i pos) const
{
	auto runningPos = Vector2i{ Bounds.lower.x, Bounds.upper.y } + Vector2i{ margin, -margin };

	auto withinButton = [](const SidebarButton& b, Vector2i p, Vector2i runningPos)
	{
		return p.x >= runningPos.x && p.y >= (runningPos.y - b.dimensions.y) && p.x < (runningPos.x + b.dimensions.x) && p.y < runningPos.y;
	};

	for (int i = 0; i < Buttons.size(); i++)
	{
		if (withinButton(Buttons[i], pos, runningPos))
		{
			return i;
		}
		runningPos.y -= Buttons[i].dimensions.y + spacing;
	}
	return -1;
}

constexpr int BORDER_OFFSET = 4;
constexpr int COLOR_THING_SIZE = 32;

void Sidebar::render(Vector2i windowSize) const
{
	checkForGlError("Before sidebar rendering");
	glScissor(Bounds.lower.x, Bounds.lower.y, Bounds.width(), Bounds.height()); // Clip stuff drawn outside bounds
	checkForGlError("Sidebar scissor");
	renderArea(Bounds, BackgroundColor);
	checkForGlError("Sidebar background");
	
	Vector2i pos = Vector2i{ Bounds.lower.x, Bounds.upper.y } + Vector2i{ margin, -margin };

	for (auto& button : Buttons)
	{
		renderButton(button.asButton(pos - Vector2u{ 0, button.dimensions.y }), windowSize);
		checkForGlError("Render sidebar button");
		int rightEdge = pos.x + button.dimensions.x - BORDER_OFFSET;
		int topEdge = pos.y - BORDER_OFFSET;
		renderArea(Box(Vector2i{ rightEdge - COLOR_THING_SIZE, topEdge - COLOR_THING_SIZE }, Vector2u{ (unsigned int)COLOR_THING_SIZE, (unsigned int)COLOR_THING_SIZE }), button.GroupColor);
		checkForGlError("Render sidebar color");
		pos.y -= button.dimensions.y + spacing;
	}
}

void renderArea(Box box, Color32 color)
{
	auto mat = GLMatrix(GL_MODELVIEW);

	glColor3f(color.R, color.G, color.B);

	glBegin(GL_QUADS);

	glVertex3i(box.lower.x, box.lower.y, 0);
	glVertex3i(box.upper.x, box.lower.y, 0);
	glVertex3i(box.upper.x, box.upper.y, 0);
	glVertex3i(box.lower.x, box.upper.y, 0);

	glEnd();
}

Vector2i windowToGL(Vector2i windowCoords, Vector2i windowSize)
{
	return Vector2i{ windowCoords.x, windowSize.y - windowCoords.y };
}

Vector2u windowToGLClipped(Vector2i windowCoords, Vector2i windowSize)
{
	auto glCoords = windowToGL(windowCoords, windowSize);
	return Vector2u{ (unsigned int)std::min(std::max(glCoords.x, 0), windowSize.x), (unsigned int)std::min(std::max(glCoords.y, 0), windowSize.y) };
}

void drawVecTextureArea(const GLTexture& tex, int texWidth, int texHeight, Vector3i translate, float scale, Box box, Vector2i windowSize)
{
	glScissor(box.lower.x, box.lower.y, box.width(), box.height());
	auto mat = GLMatrix(GL_MODELVIEW);
	glTranslatef((float)windowSize.x * 0.5f, windowSize.y, 0.f);
	glScalef(scale, scale, scale);
	glTranslatef(0.f, -((float)texHeight) * 0.5f, 0.f);

	int halfWidth = texWidth / 2;
	int halfHeight = texHeight / 2;

	tex.bindTo(GL_TEXTURE_2D);
	glBegin(GL_QUADS);

	Vector3i lowLeft = { -halfWidth, -halfHeight, 0 };
	Vector3i lowRight = { halfWidth, -halfHeight, 0 };
	Vector3i upRight = { halfWidth, halfHeight, 0 };
	Vector3i upLeft = { -halfWidth, halfHeight, 0 };

	auto xScale = tex.getXScale();
	auto yScale = tex.getYScale();

	glTexCoord2f(0.0f * xScale, 0.0f * yScale); glVertex3i(lowLeft.x, lowLeft.y, lowLeft.z);  // Bottom Left
	glTexCoord2f(1.0f * xScale, 0.0f * yScale); glVertex3i(lowRight.x, lowRight.y, lowRight.z);  // Bottom Right
	glTexCoord2f(1.0f * xScale, 1.0f * yScale); glVertex3i(upRight.x, upRight.y, upRight.z);  // Top Right
	glTexCoord2f(0.0f * xScale, 1.0f * yScale); glVertex3i(upLeft.x, upLeft.y, upLeft.z);  // Top Left

	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glScissor(0, 0, windowSize.x, windowSize.y);
}

Sidebar::Sidebar(std::vector<SidebarButton> buttons)
	: Buttons(buttons)
	, Bounds(Vector2i{ 0, 0 }, Vector2u{ 1, 1 })
	, BackgroundColor(Colors::Black32)
	, spacing(0)
	, margin(0)
{
}

Sidebar::Sidebar(Box bounds, std::vector<SidebarButton> buttons, Color32 backgroundColor, int margin, int spacing)
	: Bounds(bounds)
	, Buttons(buttons)
	, BackgroundColor(backgroundColor)
	, margin(margin)
	, spacing(spacing)
{
}

void Sidebar::addButton(SidebarButton button)
{
	Buttons.push_back(button);
}

TextField::TextField(Vector2i position, Vector2u dimensions, std::string initialText, Color32 color, Color32 textColor, Color32 cursorColor, bool numberOnly)
	: _position(position)
	, _dimensions(dimensions)
	, _text(initialText)
	, _color(color)
	, _textColor(textColor)
	, _numberOnly(numberOnly)
{

}

void TextField::render(Vector2i windowSize, bool selected) const
{
	auto box = Box(_position, _dimensions);
	renderArea(box, _color);
	int height = glutBitmapHeight(GLUT_BITMAP_HELVETICA_18);
	renderString(_position.x, _position.y + (_dimensions.y - height) / 2, GLUT_BITMAP_HELVETICA_18, _text, _textColor);
	if (selected)
	{
		auto cursorBox = Box(_position, Vector2u{ 2, _dimensions.y });
		for (int i = 0; i < _cursorPosition && i < _text.size(); i++)
		{
			auto width = glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, _text[i]);
			cursorBox.lower.x += width;
			cursorBox.upper.x += width;
		}
		renderArea(cursorBox, _cursorColor);
	}
}

void TextField::update(Vector2i windowSize, Vector2i mousePos, int mouseButton, int mouseState)
{
	auto textStart = _position;
	auto mouseRel = mousePos - textStart;

	if (mouseRel.x < 0 || mouseRel.x > _dimensions.x || mouseRel.y < 0 || mouseRel.y > _dimensions.y)
		return;

	for (int i = 0; i < _text.size(); i++)
	{
		auto width = glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, _text[i]);
		
		if (mouseRel.x < 0)
		{
			_cursorPosition = i;
			return;
		}

		mouseRel.x -= width;
	}
	_cursorPosition = _text.size();
}

void TextField::moveLeft()
{
	if (_cursorPosition > 0)
	{
		_cursorPosition--;
	}
}

void TextField::moveRight()
{
	if (_cursorPosition < (_text.size()))
	{
		_cursorPosition++;
	}
}

void TextField::insert(unsigned char key)
{
	if (key == 8) // 8 is the ASCII 'backspace' control key code
	{
		if (_text.size() && _cursorPosition != 0)
		{
			_text.erase(_text.begin() + (_cursorPosition < _text.size() ? _cursorPosition : _text.size() - 1));
			_cursorPosition = std::min(_cursorPosition - 1, (int)_text.size());
		}
	}
	else if (key == 127) // 127 is the ASCII 'Delete' control key code
	{
		if (_text.size() && _cursorPosition < (_text.size() + 1))
		{
			_text.erase(_text.begin() + _cursorPosition);
		}
	}
	else if (key >= 32 && key <= 126) // 32 is the lowest ASCII character code, and 126 is the highest. everything in between is a printable character.
	{
		if (!_numberOnly || (key >= 48 && key <= 57)) // 48 is the lowest ASCII number code, and 57 is the highest. everything in between is a number.
		{
			_text.insert(_text.begin() + _cursorPosition, key);
			_cursorPosition++;
		}
	}
}

bool TextField::isWithin(Vector2i position) const
{
	return position.x >= _position.x && position.x <= _position.x + _dimensions.x && position.y >= _position.y && position.y <= _position.y + _dimensions.y;
}

void TextField::setText(std::string text)
{
	_text = text;
}

