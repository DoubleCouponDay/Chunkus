#include <memory>
#include <fstream>
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <cmath>
#include <array>
#include <filesystem>

#include <GL/freeglut.h>

#include "texture.h"
#include "gl.h"

#include "../../C_part/src/entrypoint.h"

#include "global.h"
#include "interop.h"

interop platform{};

void doVectorize()
{
	vectorizer_data data;
	data.chunk_size = 1;
	data.filename = "input.png";
	data.outputfilename = "output.svg";
	data.threshold = 100.f;

	int code = platform.doTheVectorize(data);

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

constexpr int textureAreaStart = 150;
constexpr int buttonSize = 40;

void sizeButtons()
{
	myData.quitButton = Button{ Vector2i{ myData.windowSize.x - 50, 0 }, Vector2u{ 50, 32 }, "Quit", Colors::Grey32 };
	myData.leftButton = Button{ Vector2i{ myData.windowSize.x / 2 - buttonSize, textureAreaStart - buttonSize }, Vector2u{ buttonSize, buttonSize }, "<", Colors::Grey32 };
	myData.rightButton = Button{ Vector2i{ myData.windowSize.x / 2, textureAreaStart - buttonSize }, Vector2u{ buttonSize, buttonSize }, ">", Colors::Grey32 };

	int switchInputLength = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)"Input") + 6;
	int switchIntermediateLength = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)"Intermediate") + 6;
	int switchOutputLength = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)"Output") + 6;
	int beginReloadLength = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)"Begin Reload") + 6;
	int finishReloadLength = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)"Finish Reload") + 6;

	int firstSwitchPosition = myData.windowSize.x / 2 - (switchIntermediateLength / 2) - switchInputLength;
	int secondSwitchPosition = firstSwitchPosition + switchInputLength;
	int thirdSwitchPosition = secondSwitchPosition + switchIntermediateLength;

	myData.switchInputButton = Button{ Vector2i{ firstSwitchPosition, textureAreaStart - buttonSize * 2},	Vector2u{ (unsigned int)switchInputLength, buttonSize},	"Input", Colors::Grey32 };
	myData.switchInterButton = Button{ Vector2i{ secondSwitchPosition, textureAreaStart - buttonSize * 2 }, Vector2u{ (unsigned int)switchIntermediateLength, buttonSize }, "Intermediate", Colors::Grey32 };
	myData.switchVectorButton = Button{ Vector2i{ thirdSwitchPosition, textureAreaStart - buttonSize * 2 },	Vector2u{ (unsigned int)switchOutputLength, buttonSize }, "Output", Colors::Grey32 };
	myData.beginReloadButton = Button{ Vector2i{ myData.windowSize.x / 2 - beginReloadLength, textureAreaStart - buttonSize * 3 },	Vector2u{ (unsigned int)beginReloadLength, buttonSize }, "Begin Reload", Colors::Grey32 };
	myData.finishReloadButton = Button{ Vector2i{ myData.windowSize.x / 2, textureAreaStart - buttonSize * 3 },	Vector2u{ (unsigned int)finishReloadLength, buttonSize }, "Finish Reload", Colors::Grey32 };
}

void my_init()
{
	checkForGlError("Beginning of my_init");
	myData.windowSize = { glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT) };
	myData.textureArea = Box{ Vector2i{ 50, textureAreaStart }, Vector2i{ myData.windowSize.x - 50, myData.windowSize.y } };

	myData.vectorizeButton = Button{ Vector2i{ 15, 35 }, Vector2u{ 100, 32 }, "Vectorize", Colors::Grey32 };
	myData.selectFileButton = Button{ Vector2i{ 15, 70 }, Vector2u{ 140, 32 }, "Select Image", Colors::Grey32 };

	sizeButtons();
	checkForGlError("Post make buttons");

	myData.data.filename = "";

	auto buttons = {
		&myData.vectorizeButton, 
		&myData.quitButton, 
		&myData.switchInputButton, 
		&myData.switchInterButton, 
		&myData.switchVectorButton, 
		&myData.leftButton, 
		&myData.rightButton, 
		&myData.beginReloadButton, 
		&myData.finishReloadButton,
		&myData.selectFileButton
	};

	myData.buttons.insert(myData.buttons.begin(), buttons.begin(), buttons.end());

	checkForGlError("Post Init");
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_SCISSOR_TEST);
	checkForGlError("Post set gl things");
}

void onResize(int w, int h)
{
	myData.windowSize.x = w;
	myData.windowSize.y = h;
	myData.textureArea = Box{ Vector2i{ 50, textureAreaStart }, Vector2i{ myData.windowSize.x - 50, myData.windowSize.y } };

	sizeButtons();

	glViewport(0, 0, w, h);
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
		int activeTexStrLen = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)myData.getCurrentText());
		int fontHeight = glutBitmapHeight(GLUT_BITMAP_HELVETICA_18);
		renderString(Box{ Vector2i{ myData.windowSize.x / 2 - activeTexStrLen / 2, textureAreaStart - buttonSize * 2 - fontHeight - 3 }, Vector2u{ (unsigned int)activeTexStrLen, (unsigned int)fontHeight + 6 } }, GLUT_BITMAP_HELVETICA_18, myData.getCurrentText(), Colors::White32);


		// Draw status string
		renderString(5, 5, GLUT_BITMAP_HELVETICA_18, myData.statusString, Colors::White32);

		// Draw file string
		std::string vectorizeString;
		if (strlen(myData.data.filename))
			vectorizeString = std::string("Vectorizing '") + myData.data.filename + "' with chunk size: " + std::to_string(myData.data.chunk_size) + " and threshold: " + std::to_string(myData.data.threshold);
		else
			vectorizeString = std::string("Not vectorizing anything");
		auto statusLen = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)vectorizeString.c_str());
		renderString(myData.windowSize.x / 2 - (statusLen / 2), myData.windowSize.y - fontHeight, GLUT_BITMAP_HELVETICA_18, vectorizeString, Colors::Grey32);

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
		std::cout << "uwu r key pressed" << std::endl;
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
	bool withinVectorize = myData.vectorizeButton.isWithin(glCoords);
	bool withinQuit = myData.quitButton.isWithin(glCoords);
	bool withinLeft = myData.leftButton.isWithin(glCoords);
	bool withinRight = myData.rightButton.isWithin(glCoords);
	bool withinInput = myData.switchInputButton.isWithin(glCoords);
	bool withinInter = myData.switchInterButton.isWithin(glCoords);
	bool withinVector = myData.switchVectorButton.isWithin(glCoords);
	bool withinBeginLoad = myData.beginReloadButton.isWithin(glCoords);
	bool withinFinishLoad = myData.finishReloadButton.isWithin(glCoords);

	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			if (withinVectorize)
			{
				std::cout << "Vectorize Button Clicked" << std::endl;
				std::cout << "Temporary action: call begin_vectorization" << std::endl;
				myData.progress = platform.beginVectorization(myData.data);
			}
			if (withinQuit)
			{
				std::cout << "Quit Button clicked" << std::endl;
				glutExit();
			}
			if (withinLeft)
			{
				std::cout << "Left Button clicked" << std::endl;
				std::cout << "Temporary action: call reverse_vectorization" << std::endl;
				platform.reverseVectorization(&myData.progress);
			}
			if (withinRight)
			{
				std::cout << "Right Button clicked" << std::endl;
				std::cout << "Temporary action: call step_vectorization" << std::endl;
				platform.stepVectorization(&myData.progress);
			}
			if (withinInput)
			{
				std::cout << "Switch to Input Button clicked" << std::endl;
				myData.activeTexture = ActiveTexture::INPUT;
				glutPostRedisplay();
			}
			if (withinInter)
			{
				std::cout << "Switch to Intermediate Button clicked" << std::endl;
				myData.activeTexture = ActiveTexture::INTERMEDIATE;
				glutPostRedisplay();
			}
			if (withinVector)
			{
				std::cout << "Switch to Vectorized output Button clicked" << std::endl;
				myData.activeTexture = ActiveTexture::VECTORIZED;
				glutPostRedisplay();
			}
			if (withinBeginLoad)
			{
				std::cout << "Begin Reloading Button was clicked" << std::endl;
				platform.release_shared_lib();
			}
			if (withinFinishLoad)
			{
				std::cout << "Finish Reloading Button was clicked" << std::endl;
				platform.hot_reload();
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
	auto exe_path = std::filesystem::path(argv[0]);
	auto exe_dir = exe_path.parent_path();
	auto exe_dir_name = exe_dir.string();

	platform.setExeFolder(exe_dir_name);
	platform.hot_reload();

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

	checkForGlError("Before Glut Main");
	glutMainLoop();

	return 0;
}

