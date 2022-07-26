#include "render.h"

#include "gl.h"

#include "global.h"

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

void renderButton(const Button& button)
{
	glScissor(button.position.x, button.position.y, button.dimensions.x, button.dimensions.y); // Scissor Rect

	{
		auto mat = GLMatrix(GL_MODELVIEW);


		glColor3f(button.color.R, button.color.G, button.color.B);

		glBegin(GL_QUADS);
		// Counter Clockwise order
		glVertex3i(button.position.x + 0                  , button.position.y + 0                  , 0); // Lower Left
		glVertex3i(button.position.x + button.dimensions.x, button.position.y + 0                  , 0); // Lower Right
		glVertex3i(button.position.x + button.dimensions.x, button.position.y + button.dimensions.y, 0); // Upper Right
		glVertex3i(button.position.x + 0                  , button.position.y + button.dimensions.y, 0); // Upper Left

		glEnd();
	}

	int yMargin = (button.dimensions.y - glutBitmapHeight(GLUT_BITMAP_HELVETICA_18));
	int xMargin = (button.dimensions.x - glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)button.text.c_str())) / 2;
	renderString(button.position.x + xMargin, button.position.y + yMargin, GLUT_BITMAP_HELVETICA_18, button.text, Colors::White32);

	glScissor(0, 0, myData.windowSize.x, myData.windowSize.y);
}

void renderSidebar(const Sidebar& sidebar)
{
	glScissor(sidebar.Bounds.lower.x, sidebar.Bounds.lower.y, sidebar.Bounds.width(), sidebar.Bounds.height()); // Clip stuff drawn outside bounds
	
	constexpr float BORDER_OFFSET = 4.f;
	constexpr float COLOR_THING_SIZE = 32.f;

	for (auto& sidbutton : sidebar.Buttons)
	{
		auto& button = sidbutton.Button;
		renderButton(button);
		float rightEdge = button.position.x + button.dimensions.x - BORDER_OFFSET;
		float topEdge = button.position.y + BORDER_OFFSET;
		renderArea(Box{ Vector2i{ rightEdge - COLOR_THING_SIZE, topEdge }, Vector2i{ COLOR_THING_SIZE, COLOR_THING_SIZE } }, sidbutton.GroupColor);
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

Vector2i windowToGL(Vector2i windowCoords)
{
	return Vector2i{ windowCoords.x, myData.windowSize.y - windowCoords.y };
}

void drawVecTextureArea(const GLTexture& tex, int texWidth, int texHeight, Vector3i translate, float scale, Box box)
{
	glScissor(box.lower.x, box.lower.y, box.width(), box.height());
	auto mat = GLMatrix(GL_MODELVIEW);
	glTranslatef((float)myData.windowSize.x * 0.5f, myData.windowSize.y, 0.f);
	glScalef(scale, scale, scale);
	glTranslatef(0.f, -((float)texHeight), 0.f);
	//glTranslatef(translate.x, translate.y, translate.z);
	int width = myData.windowSize.x;
	int height = myData.windowSize.y;

	int halfWidth = texWidth;
	int halfHeight = texHeight;

	int midPoint = width / 2;

	tex.bindTo(GL_TEXTURE_2D);
	glBegin(GL_QUADS);

	myData.lowLeft = { -halfWidth, -halfHeight, 0 };  // Bottom Left Of The Texture and Quad
	myData.lowRight = { +halfWidth, -halfHeight, 0 };  // Bottom Right Of The Texture and Quad
	myData.upRight = { +halfWidth, +halfHeight, 0 };  // Top Right Of The Texture and Quad
	myData.upLeft = { -halfWidth, +halfHeight, 0 };  // Top Left Of The Texture and Quad

	auto xScale = tex.getXScale();
	auto yScale = tex.getYScale();

	glTexCoord2f(0.0f * xScale, 0.0f * yScale); glVertex3i(myData.lowLeft.x, myData.lowLeft.y, myData.lowLeft.z);  // Bottom Left
	glTexCoord2f(1.0f * xScale, 0.0f * yScale); glVertex3i(myData.lowRight.x, myData.lowRight.y, myData.lowRight.z);  // Bottom Right
	glTexCoord2f(1.0f * xScale, 1.0f * yScale); glVertex3i(myData.upRight.x, myData.upRight.y, myData.upRight.z);  // Top Right
	glTexCoord2f(0.0f * xScale, 1.0f * yScale); glVertex3i(myData.upLeft.x, myData.upLeft.y, myData.upLeft.z);  // Top Left

	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glScissor(0, 0, myData.windowSize.x, myData.windowSize.y);
}