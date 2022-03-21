#include <memory>
#include <fstream>
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <cmath>
#include <array>

#include <GL/freeglut.h>

#include "texture.h"
#include "gl.h"

#include <entrypoint.h>

struct Vector2i
{
	int x;
	int y;

	inline Vector2i operator-() const { return { -x, -y }; }
	inline Vector2i operator-(const Vector2i& other) const { return { x - other.x, y - other.y }; }
	inline Vector2i operator+(const Vector2i& other) const { return { x + other.x, y + other.y }; }
};

struct Vector2u
{
	unsigned int x;
	unsigned int y;
};

struct Vector3i
{
	int x, y, z;

	inline std::string toString() const
	{
		char buf[80] = { 0 };

		auto len = snprintf(buf, sizeof(buf), "(%00u, %00u, %00u)", x, y, z);
		return std::string(buf, buf + len);
	}
};

struct Box
{
	Vector2i lower;
	Vector2i upper;

	inline Vector2i dimensions() const
	{
		return upper - lower;
	}

	inline int width() const { return dimensions().x; }
	inline int height() const { return dimensions().y; }
};

struct Button
{
	Vector2i position;
	Vector2u dimensions;
	std::string text;
	Color32 color;

	inline bool isWithin(Vector2i pos) const
	{
		return pos.x >= position.x 
			&& pos.y >= position.y
			&& pos.x < position.x + dimensions.x
			&& pos.y < position.y + dimensions.y;
	}
};

struct GUIData
{
	Vector2i windowSize = { 800, 600 };
	Vector3i lowLeft = { 0, 0, 0 }, lowRight = { 0, 0, 0, }, upRight = { 0, 0, 0 }, upLeft = { 0, 0, 0 };
	WomboTexture texture;
	Color32 texColor = Colors::White32;
	Button button1;
	Button button2;
	Button button3;
	int scrollage = 0;
};


GUIData myData{};


void renderString(int x, int y, void* font, std::string str, Color32 color)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(1.f, 1.f, 1.f);
	glRasterPos2i(x, y);

	for (auto& ch : str)
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, ch);
	}
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void renderButton(const Button& button)
{
	glScissor(button.position.x, button.position.y, button.dimensions.x, button.dimensions.y);
	
	{
		auto mat = GLMatrix(GL_MODELVIEW);


		glColor3f(button.color.R, button.color.G, button.color.B);

		glBegin(GL_QUADS);
		// Counter Clockwise order
		glVertex3i(button.position.x + 0,					button.position.y + 0,						0); // Lower Left
		glVertex3i(button.position.x + button.dimensions.x, button.position.y + 0,						0); // Lower Right
		glVertex3i(button.position.x + button.dimensions.x, button.position.y + button.dimensions.y,	0); // Upper Right
		glVertex3i(button.position.x + 0,					button.position.y + button.dimensions.y,	0); // Upper Left

		glEnd();
	}

	int yMargin = (button.dimensions.y - 18) / 2;
	renderString(button.position.x + 5, button.position.y + yMargin, GLUT_BITMAP_HELVETICA_18, button.text, Colors::White32);
}

Vector2i windowToGL(Vector2i windowCoords)
{
	return Vector2i{ windowCoords.x, myData.windowSize.y - windowCoords.y };
}

void drawTexture(const GLTexture& tex, int texWidth, int texHeight)
{
	int width = myData.windowSize.x;
	int height = myData.windowSize.y;

	float percentWidth = (float)texWidth * 2 / (float)width;
	float percentHeight = (float)texHeight * 2 / (float)height;

	tex.bindTo(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	// Front Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f( percentWidth, 1.f - percentHeight * 2.f	,  0.f);  // Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-percentWidth, 1.f - percentHeight * 2.f	,  0.f);  // Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-percentWidth, 1.f							,  0.f);  // Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( percentWidth, 1.f							,  0.f);  // Top Left Of The Texture and Quad

	glEnd();
}

void drawTextureOrtho(const GLTexture& tex, int texWidth, int texHeight, Vector3i translate, float scale)
{
	auto mat = GLMatrix(GL_MODELVIEW);
	glTranslatef((float)myData.windowSize.x * 0.5f, myData.windowSize.y - 15, 0.f);
	glScalef(scale, scale, scale);
	glTranslatef(0.f, -((float)texHeight * 0.5f), 0.f);
	//glTranslatef(translate.x, translate.y, translate.z);
	int width = myData.windowSize.x;
	int height = myData.windowSize.y;

	int halfWidth = texWidth;
	int halfHeight = texHeight;

	int midPoint = width / 2;

	tex.bindTo(GL_TEXTURE_2D);
	glBegin(GL_QUADS);

	myData.lowLeft	= { -halfWidth, -halfHeight, 0 };  // Bottom Left Of The Texture and Quad
	myData.lowRight = { +halfWidth, -halfHeight, 0 };  // Bottom Right Of The Texture and Quad
	myData.upRight	= { +halfWidth, +halfHeight, 0 };  // Top Right Of The Texture and Quad
	myData.upLeft	= { -halfWidth, +halfHeight, 0 };  // Top Left Of The Texture and Quad

    glTexCoord2f(0.0f, 0.0f); glVertex3i(myData.lowLeft.x,	myData.lowLeft.y,	myData.lowLeft.z);  // Bottom Left
    glTexCoord2f(1.0f, 0.0f); glVertex3i(myData.lowRight.x, myData.lowRight.y,	myData.lowRight.z);  // Bottom Right
    glTexCoord2f(1.0f, 1.0f); glVertex3i(myData.upRight.x,	myData.upRight.y,	myData.upRight.z);  // Top Right
    glTexCoord2f(0.0f, 1.0f); glVertex3i(myData.upLeft.x,	myData.upLeft.y,	myData.upLeft.z);  // Top Left

	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void onResize(int w, int h)
{
	myData.windowSize.x = w;
	myData.windowSize.y = h;
	glViewport(0, 0, w, h);
}

void my_init()
{
	myData.windowSize = { glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT) };

	myData.texture = WomboTexture("epic.bmp", false);

	myData.button1 = Button{ Vector2i{ 15, 15 }, Vector2u{ 100, 32 }, "Button 1", Colors::Grey32 };
	myData.button2 = Button{ Vector2i{ 125, 15 }, Vector2u{ 100, 32 }, "Button 2", Colors::Grey32 };
	myData.button3 = Button{ Vector2i{ 235, 15 }, Vector2u{ 100, 32 }, "Button 3", Colors::Grey32 };

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
}

void display()
{	
	myData.windowSize.x = glutGet(GLUT_WINDOW_WIDTH);
	myData.windowSize.y = glutGet(GLUT_WINDOW_HEIGHT);
	checkForGlError("Displaying");
	glClear(GL_COLOR_BUFFER_BIT);
	checkForGlError("Cleared the back buffer");

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.f, 0.f, 0.f);
	glColor3f(1.f, 0.f, 1.f);
	glutSolidCube(0.4);
	checkForGlError("Drew cube");
	glColor3f(1.f, 1.f, 1.f);
	//drawTexture(epicTex.getGLTex(), epicTex.getCpuTex().getWidth(), epicTex.getCpuTex().getHeight());

	checkForGlError("Drew texture");
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	{
		auto proj = GLMatrix(GL_PROJECTION);

		gluOrtho2D(0.0, myData.windowSize.x, 0.0, myData.windowSize.y);

		glColor3f(myData.texColor.R, myData.texColor.G, myData.texColor.B);
		drawTextureOrtho(myData.texture.getGLTex(), myData.texture.getCpuTex().getWidth(), myData.texture.getCpuTex().getHeight(), { 0,0,0 }, pow(1.1, myData.scrollage));

		const char text[] = "the holy fk please just draw fox jumped over the lazy ass fkin pc";
		renderString(300, 200, GLUT_BITMAP_TIMES_ROMAN_24, text, Colors::White32);
		renderString(10, 120 - 0, GLUT_BITMAP_TIMES_ROMAN_24, "Lower Left: " + myData.lowLeft.toString(), Colors::White32);
		renderString(10, 120 - 24, GLUT_BITMAP_TIMES_ROMAN_24, "Lower Right: " + myData.lowRight.toString(), Colors::White32);
		renderString(10, 120 - 48, GLUT_BITMAP_TIMES_ROMAN_24, "Upper Right: " + myData.upRight.toString(), Colors::White32);
		renderString(10, 120 - 72, GLUT_BITMAP_TIMES_ROMAN_24, "Upper Left: " + myData.upLeft.toString(), Colors::White32);

		std::vector<Button*> buttons{};
		buttons.push_back(&myData.button1);
		buttons.push_back(&myData.button2);
		buttons.push_back(&myData.button3);

		for (auto& button : buttons)
			renderButton(*button);

		checkForGlError("Rendered string");
	}


	glutSwapBuffers();
	checkForGlError("Swapped Buffers");
}

void setTexPixel(int value)
{
	(void)value;

	std::cout << "Setting pixels" << std::endl;
	checkForGlError("Before setting pixel");
	myData.texture.setArea(15, 15, 25, 25, Colors::Orange8);
	checkForGlError("After setting pixel");
	glutPostRedisplay();
}

void onKeyboardButton(unsigned char key, int mouseX, int mouseY)
{
	if (key == 'r')
	{
		std::cout << "Resetting texture" << std::endl;

		auto t = get_test_struct();

		Texture8 tex{ Colors::Black8, (GLuint)t.width, (GLuint)t.height };

		for (int x = 0; x < t.width; ++x)
		{
			for (int y = 0; y < t.height; ++y)
			{
				auto dat = t.data[y * t.width + x];
				tex.setPixel(x, y, Color8{ dat, dat, dat });
			}
		}

		myData.texture = WomboTexture{ std::move(tex) };
	}
}

void onMouseButton(int button, int state, int mouseX, int mouseY)
{
	auto glCoords = windowToGL({ mouseX, mouseY });
	bool within1 = myData.button1.isWithin(glCoords);
	bool within2 = myData.button2.isWithin(glCoords);
	bool within3 = myData.button3.isWithin(glCoords);

	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			if (within1)
			{
				std::cout << "Button 1 clicked" << std::endl;
				myData.texColor = Colors::Orange32;
			}
			if (within2)
			{
				std::cout << "Button 2 clicked" << std::endl;
				myData.texColor = Colors::Pink32;
			}
			if (within3)
			{
				std::cout << "Button 3 clicked" << std::endl;
				myData.texColor = Colors::Green32;
			}
		}
	}
}

void onMouseWheel(int button, int dir, int x, int y)
{
	std::cout << "Scroll: " << dir << std::endl;
	myData.scrollage += dir;
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	epic_exported_function();

	glutInit(&argc, argv);

	glutInitWindowSize(800, 600);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

	glutCreateWindow(argv[0]);

	my_init();

	glutDisplayFunc(display);

	glutTimerFunc(3000, setTexPixel, 0);

	glutKeyboardFunc(onKeyboardButton);
	glutMouseFunc(onMouseButton);
	glutMouseWheelFunc(onMouseWheel);
	glutReshapeFunc(onResize);

	glutMainLoop();

	return 0;
}

