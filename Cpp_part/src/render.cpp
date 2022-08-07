#include "render.h"

#include "gl.h"

#include "gui_data.h"
#include "c_wrappers.h"

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

void Sidebar::updateFromVisuals(const visual_algorithm_data& data)
{
	Buttons.clear();

	Vector2u dimensions{ Bounds.width() - 2 * margin - COLOR_THING_SIZE, 40 };
	for (auto& group : data.data.groups)
	{
		Buttons.push_back(SidebarButton{ dimensions, std::to_string(group.pixels.size()), Colors::Grey32, group.color });
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

void renderAlgorithm(const visual_algorithm_data& data, float scale, Box box, int selectedGroup, Vector2i windowSize)
{
	checkForGlError("Before rendering algorithm");

	auto texWidth = data.complete_texture.getWidth();
	auto texHeight = data.complete_texture.getHeight();
	glScissor(box.lower.x, box.lower.y, box.width(), box.height());
	{
		auto mat = GLMatrix(GL_MODELVIEW);
		glTranslatef((float)windowSize.x * 0.5f, (float)windowSize.y, 0.f);
		glScalef(scale, scale, scale);
		glTranslatef(0.f, -((float)texHeight) * 0.5f, 0.f);

		int halfWidth = texWidth / 2;
		int halfHeight = texHeight / 2;

		data.complete_texture.bindTo(GL_TEXTURE_2D);
		glBegin(GL_QUADS);

		Vector3i lowLeft = { -halfWidth, -halfHeight, 0 };  // Bottom Left Of The Texture and Quad
		Vector3i lowRight = { +halfWidth, -halfHeight, 0 };  // Bottom Right Of The Texture and Quad
		Vector3i upRight = { +halfWidth, +halfHeight, 0 };  // Top Right Of The Texture and Quad
		Vector3i upLeft = { -halfWidth, +halfHeight, 0 };  // Top Left Of The Texture and Quad

		auto xScale = data.complete_texture.getGLTex().getXScale();
		auto yScale = data.complete_texture.getGLTex().getYScale();

		if (selectedGroup > -1)
			glColor3f(0.2f, 0.2f, 0.2f); // Reduce the color of the main texture if we have a selected group

		glTexCoord2f(0.0f * xScale, 0.0f * yScale); glVertex3i(lowLeft.x, lowLeft.y, lowLeft.z);  // Bottom Left
		glTexCoord2f(1.0f * xScale, 0.0f * yScale); glVertex3i(lowRight.x, lowRight.y, lowRight.z);  // Bottom Right
		glTexCoord2f(1.0f * xScale, 1.0f * yScale); glVertex3i(upRight.x, upRight.y, upRight.z);  // Top Right
		glTexCoord2f(0.0f * xScale, 1.0f * yScale); glVertex3i(upLeft.x, upLeft.y, upLeft.z);  // Top Left


		glEnd();
	}
	if (selectedGroup > -1 && selectedGroup < data.textures.size())
	{
		auto& tex = data.textures[selectedGroup];
		auto& gl = tex.texture.getGLTex();
		auto& cpu = tex.texture.getCpuTex();
		gl.bindTo(GL_TEXTURE_2D);
		auto modelMat = GLMatrix(GL_MODELVIEW);
		glTranslatef((float)windowSize.x * 0.5f, windowSize.y, 0.f);
		glScalef(scale, scale, scale);

		auto parentTopMidPoint = Vector2f{ (float)texWidth / 2.f, (float)texHeight };
		auto myTopMidPoint = Vector2f{ (float)tex.position.x + ((float)cpu.getWidth() * 0.5f), (float)tex.position.y + (float)cpu.getHeight() };

		auto translate = myTopMidPoint - parentTopMidPoint;
		glTranslatef(translate.x, translate.y, 0.f);

		glTranslatef(0.f, -((float)cpu.getHeight()) * 0.5f, 0.f);

		glColor3f(1.f, 1.f, 1.f); // Draw the selected group as 'white' to give it full color (white is multiplied by the color in the texture)

		glBegin(GL_QUADS);

		auto xScale = gl.getXScale();
		auto yScale = gl.getYScale();

		int upperX = cpu.getWidth() / 2;
		int upperY = cpu.getHeight() / 2;
		int lowerX = -upperX;
		int lowerY = -upperY;

		glTexCoord2f(0.0f * xScale, 0.0f * yScale); glVertex3i(lowerX, lowerY, 0);  // Bottom Left
		glTexCoord2f(1.0f * xScale, 0.0f * yScale); glVertex3i(upperX, lowerY, 0);  // Bottom Right
		glTexCoord2f(1.0f * xScale, 1.0f * yScale); glVertex3i(upperX, upperY, 0);  // Top Right
		glTexCoord2f(0.0f * xScale, 1.0f * yScale); glVertex3i(lowerX, upperY, 0);  // Top Left
		glEnd();
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glScissor(0, 0, windowSize.x, windowSize.y);
}