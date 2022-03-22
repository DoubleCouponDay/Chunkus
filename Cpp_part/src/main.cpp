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

#include "global.h"

void onResize(int w, int h)
{
	myData.windowSize.x = w;
	myData.windowSize.y = h;
	myData.textureArea = Box{ Vector2i{ 50, 300 }, Vector2i{ myData.windowSize.x - 50, myData.windowSize.y } };
	glViewport(0, 0, w, h);
}

void doVectorize()
{
	vectorizer_data data;
	data.chunk_size = 1;
	data.filename = "input.png";
	data.outputfilename = "output.svg";
	data.threshold = 100.f;

	int code = do_the_vectorize(data);

	if (code != SUCCESS_CODE)
	{

	}
	else
	{
		Texture8 tex = Texture8{ "input.png", false };
		if (tex.getBytes() == nullptr)
		{
			myData.inputTexture			= WomboTexture("placeholder.bmp", false);
			myData.intermediateTexture	= WomboTexture("placeholder.bmp", false);
			myData.vectorizedTexture	= WomboTexture("placeholder.bmp", false);
			return;
		}
		myData.inputTexture = WomboTexture(std::move(tex));
	}
}

void my_init()
{
	constexpr int textureAreaStart = 150;
	myData.windowSize = { glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT) };
	myData.textureArea = Box{ Vector2i{ 50, textureAreaStart }, Vector2i{ myData.windowSize.x - 50, myData.windowSize.y } };

	myData.inputTexture			= WomboTexture("placeholder.bmp", false);
	myData.intermediateTexture	= WomboTexture("placeholder.bmp", false);
	myData.vectorizedTexture	= WomboTexture("placeholder.bmp", false);

	myData.button1 = Button{ Vector2i{ 15, 15 }, Vector2u{ 100, 32 }, "Button 1", Colors::Grey32 };
	myData.button2 = Button{ Vector2i{ 125, 15 }, Vector2u{ 100, 32 }, "Button 2", Colors::Grey32 };
	myData.button3 = Button{ Vector2i{ 235, 15 }, Vector2u{ 100, 32 }, "Button 3", Colors::Grey32 };
	myData.button4 = Button{ Vector2i{ 345, 15 }, Vector2u{ 80, 32 }, "Swap Tex", Colors::Grey32 };
	constexpr int buttonSize = 40;
	myData.leftButton = Button{ Vector2i{ myData.windowSize.x / 2 - buttonSize, textureAreaStart - buttonSize }, Vector2u{ buttonSize, buttonSize }, "<", Colors::Grey32 };
	myData.rightButton = Button{ Vector2i{ myData.windowSize.x / 2, textureAreaStart - buttonSize }, Vector2u{ buttonSize, buttonSize }, ">", Colors::Grey32 };

	myData.data.filename = "uwu.png";

	auto buttons = { &myData.button1, &myData.button2, &myData.button3, &myData.button4, &myData.leftButton, &myData.rightButton };
	myData.buttons.insert(myData.buttons.begin(), buttons.begin(), buttons.end());

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_SCISSOR_TEST);
}

/// <summary>
/// Display callback, called whenever the window needs to be redrawn
/// </summary>
void display()
{
	// Things to draw:
	// - Active texture
	// - Buttons
	// - Status string
	// - Filename string
	myData.windowSize.x = glutGet(GLUT_WINDOW_WIDTH);
	myData.windowSize.y = glutGet(GLUT_WINDOW_HEIGHT);
	checkForGlError("Displaying");
	glClear(GL_COLOR_BUFFER_BIT);
	checkForGlError("Cleared the back buffer");

	{
		auto proj = GLMatrix(GL_PROJECTION);

		gluOrtho2D(0.0, myData.windowSize.x, 0.0, myData.windowSize.y);

		// Draw active texture
		glColor3f(myData.texColor.R, myData.texColor.G, myData.texColor.B);
		auto& tex = myData.getActiveTexture();
		drawVecTextureArea(tex.getGLTex(), tex.getCpuTex().getWidth(), tex.getCpuTex().getHeight(), { 0,0,0 }, pow(1.1, myData.scrollage), myData.textureArea);

		// Draw active texture string
		renderString(50, 300, GLUT_BITMAP_HELVETICA_18, myData.getCurrentText(), Colors::White32);

		// Draw random other textures
		renderString(10, 120 - 0, GLUT_BITMAP_TIMES_ROMAN_24, "Lower Left: " + myData.lowLeft.toString(), Colors::White32);
		renderString(10, 120 - 24, GLUT_BITMAP_TIMES_ROMAN_24, "Lower Right: " + myData.lowRight.toString(), Colors::White32);
		renderString(10, 120 - 48, GLUT_BITMAP_TIMES_ROMAN_24, "Upper Right: " + myData.upRight.toString(), Colors::White32);
		renderString(10, 120 - 72, GLUT_BITMAP_TIMES_ROMAN_24, "Upper Left: " + myData.upLeft.toString(), Colors::White32);

		// Draw status string
		renderString(5, 5, GLUT_BITMAP_HELVETICA_18, myData.statusString, Colors::White32);

		// Draw file string
		std::string vectorizeString = std::string("Vectorizing '") + myData.data.filename + "'";
		auto statusLen = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)vectorizeString.c_str());
		renderString(myData.windowSize.x / 2 - (statusLen / 2), myData.windowSize.y - 50 - 18, GLUT_BITMAP_HELVETICA_18, vectorizeString, Colors::Grey32);

		// Draw buttons
		for (auto& button : myData.buttons)
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

		free_test_struct(&t);

		myData.getActiveTexture() = WomboTexture{ std::move(tex) };
	}
	if (key == '1')
	{
		std::cout << "Switching to input image" << std::endl;

		myData.activeTexture = ActiveTexture::INPUT;
		glutPostRedisplay();
	}
	if (key == '2')
	{
		std::cout << "Switching to intermediate image" << std::endl;

		myData.activeTexture = ActiveTexture::INTERMEDIATE;
		glutPostRedisplay();
	}
	if (key == '3')
	{
		std::cout << "Switching to output image" << std::endl;

		myData.activeTexture = ActiveTexture::VECTORIZED;
		glutPostRedisplay();
	}
}

void onMouseButton(int button, int state, int mouseX, int mouseY)
{
	auto glCoords = windowToGL({ mouseX, mouseY });
	bool within1 = myData.button1.isWithin(glCoords);
	bool within2 = myData.button2.isWithin(glCoords);
	bool within3 = myData.button3.isWithin(glCoords);
	bool within4 = myData.button4.isWithin(glCoords);

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
			if (within4)
			{
				std::cout << "Swapping textures" << std::endl;
			}
		}
	}
}

void onMouseWheel(int button, int dir, int x, int y)
{
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

