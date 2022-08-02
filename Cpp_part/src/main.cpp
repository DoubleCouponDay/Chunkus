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

#include <lunasvg.h>

#include "texture.h"
#include "gl.h"

#include "../../C_part/src/entrypoint.h"

#include "global.h"
#include "interop.h"

interop platform{};

void doVectorize(std::string image_path)
{
	std::cout << "About to vectorize: " << image_path << std::endl;
	vectorizer_data data;
	data.chunk_size = 1;
	data.filename = image_path.c_str();
	data.outputfilename = "output.svg";
	data.threshold = 100.f;

	std::cout << "Vectorizing '" << image_path << "' to " << data.outputfilename << " with chunk size: " << data.chunk_size << ", threshold: " << data.threshold << " and num colors: " << 256 << std::endl;
	int code = do_vectorize(data.filename, data.outputfilename, data.chunk_size, data.threshold, 256);

	if (code != SUCCESS_CODE)
	{
		std::cout << "Error when vectorizing: " << code << std::endl;
		glutExit();
	}

	else
	{
		Texture8 tex = Texture8{ image_path, false };
		if (tex.getBytes() == nullptr)
		{
			using namespace lunasvg;

			auto doc = Document::loadFromData(
				"<?xml version = \"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
				"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n"
				"<svg width=\"400\" height=\"400\" viewBox=\"0 0 400 400\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n"
				"<rect fill=\"#8ff\" stroke=\"#000\" x=\"0\" y=\"0\" width=\"400\" height=\"400\"/>\n"
				"<g opacity=\"0.8\">\n"
				"    <rect x=\"50\" y=\"50\" width=\"300\" height=\"300\" fill=\"lime\" stroke-width=\"4\" stroke=\"pink\" />\n"
				"    <circle cx=\"200\" cy=\"200\" r=\"100\" fill=\"orange\" />\n"
				"    <polyline points=\"100, 233 100, 300 300, 300 300, 166\" stroke=\"red\" stroke-width=\"4\" fill=\"none\" />\n"
				"    <line x1=\"100\" y1=\"100\" x2=\"300\" y2=\"300\" stroke=\"blue\" stroke-width=\"4\" />\n"
				"</g>\n"
				"</svg>\n"
			);

			auto bitmap = doc->renderToBitmap(396, 531);

			if (!bitmap.valid())
			{
				std::cout << "Failed to render svg bitmap" << std::endl;
				myData.vectorizedTexture	= WomboTexture("placeholder.bmp", false);
			}
			else
			{
				myData.vectorizedTexture	= WomboTexture(bitmap);
			}
			myData.inputTexture		   = WomboTexture("placeholder.bmp", false);
			myData.intermediateTexture = WomboTexture(Texture8{ Colors::Orange8, 369, 342 });
			return;
		}
		else
		{
			myData.inputTexture = WomboTexture(std::move(tex));
			myData.intermediateTexture = WomboTexture(Texture8{ Colors::Orange8, 369, 342 });

			auto doc = lunasvg::Document::loadFromFile("output.svg");

			auto bitmap = doc->renderToBitmap((uint32_t)myData.inputTexture.getWidth(), (uint32_t)myData.inputTexture.getHeight());

			if (!bitmap.valid())
			{
				std::cout << "Failed to render the svg output" << std::endl;
				myData.vectorizedTexture.clear();
			}
			else
			{
				myData.vectorizedTexture = WomboTexture(bitmap);
			}
		}
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
	int writeToBmpLength = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)"Write to bmp") + 6;

	int firstSwitchPosition = myData.windowSize.x / 2 - (switchIntermediateLength / 2) - switchInputLength;
	int secondSwitchPosition = firstSwitchPosition + switchInputLength;
	int thirdSwitchPosition = secondSwitchPosition + switchIntermediateLength;

	myData.switchInputButton = Button{ Vector2i{ firstSwitchPosition, textureAreaStart - buttonSize * 2},	Vector2u{ (unsigned int)switchInputLength, buttonSize},	"Input", Colors::Grey32 };
	myData.switchInterButton = Button{ Vector2i{ secondSwitchPosition, textureAreaStart - buttonSize * 2 }, Vector2u{ (unsigned int)switchIntermediateLength, buttonSize }, "Intermediate", Colors::Grey32 };
	myData.switchVectorButton = Button{ Vector2i{ thirdSwitchPosition, textureAreaStart - buttonSize * 2 },	Vector2u{ (unsigned int)switchOutputLength, buttonSize }, "Output", Colors::Grey32 };
	myData.writeToBmpButton = Button{ Vector2i{ myData.windowSize.x / 2 - writeToBmpLength / 2, textureAreaStart - buttonSize * 3 }, Vector2u{ (unsigned int)writeToBmpLength, buttonSize }, "Write to bmp", Colors::Grey32 };
}

void my_init()
{
	checkForGlError("Beginning of my_init");
	myData.windowSize = { glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT) };
	myData.textureArea = Box{ Vector2i{ 50, textureAreaStart }, Vector2i{ myData.windowSize.x - 50, myData.windowSize.y } };

	sizeButtons();
	checkForGlError("Post make buttons");

	myData.data.filename = "";

	auto buttons = {
		&myData.quitButton, 
		&myData.switchInputButton, 
		&myData.switchInterButton, 
		&myData.switchVectorButton, 
		&myData.leftButton, 
		&myData.rightButton, 
		&myData.writeToBmpButton
	};

	myData.buttons.insert(myData.buttons.begin(), buttons.begin(), buttons.end());

	myData.sidebar = Sidebar{ Box{ windowToGL(Vector2i{ 5, 400 }), windowToGL(Vector2i{ 150, 20 }) }, std::vector<SidebarButton>(), Colors::Grey32 };
	myData.sidebar.addButton(
		SidebarButton{ Vector2u{ 135, 32 + 10 }, "150", Colors::Orange32, Colors::Pink32 }
	);
	myData.sidebar.addButton(
		SidebarButton{ Vector2u{ 135, 32 + 10 }, "32", Colors::Yellow32, Colors::Green32 }
	);

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
		checkForGlError("Render active texture string");

		// Draw status string
		renderString(5, 5, GLUT_BITMAP_HELVETICA_18, myData.statusString, Colors::White32);
		checkForGlError("Render status string");

		// Draw file string
		std::string vectorizeString;
		if (strlen(myData.data.filename))
			vectorizeString = std::string("Vectorizing '") + myData.data.filename + "' with chunk size: " + std::to_string(myData.data.chunk_size) + " and threshold: " + std::to_string(myData.data.threshold);
		else
			vectorizeString = std::string("Not vectorizing anything");
		auto statusLen = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)vectorizeString.c_str());
		renderString(myData.windowSize.x / 2 - (statusLen / 2), myData.windowSize.y - fontHeight, GLUT_BITMAP_HELVETICA_18, vectorizeString, Colors::Grey32);
		checkForGlError("Render vectorizing string");

		// Draw buttons
		for (auto& button : myData.buttons)
			renderButton(*button);
		checkForGlError("Render buttons");

		myData.sidebar.render();
		checkForGlError("Render sidebar");
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
	bool withinQuit = myData.quitButton.isWithin(glCoords);
	bool withinLeft = myData.leftButton.isWithin(glCoords);
	bool withinRight = myData.rightButton.isWithin(glCoords);
	bool withinInput = myData.switchInputButton.isWithin(glCoords);
	bool withinInter = myData.switchInterButton.isWithin(glCoords);
	bool withinVector = myData.switchVectorButton.isWithin(glCoords);
	bool withinWrite = myData.writeToBmpButton.isWithin(glCoords);
	bool withinSidebar = myData.sidebar.isWithin(glCoords);

	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			if (withinQuit)
			{
				std::cout << "Quit Button clicked" << std::endl;
				glutExit();
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
			if (withinWrite)
			{
				std::cout << "Write to BMP was clicked" << std::endl;
				myData.getActiveTexture().getCpuTex().writeToBmp("test.bmp");
			}
			if (withinLeft)
			{
				std::cout << "Left button clicked" << std::endl;
			}
			if (withinRight)
			{
				std::cout << "Right button clicked" << std::endl;
			}
			if (withinSidebar)
			{
				std::cout << "Sidebar clicked" << std::endl;
				int button = myData.sidebar.getButtonClicked(glCoords);
				if (button != -1)
				{
					std::cout << "Sidebar button " << button << " clicked" << std::endl;
				}
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
	if (argc < 1)
		return -1;

	auto exe_path = std::filesystem::path(argv[0]);
	auto exe_dir = exe_path.parent_path();
	auto exe_dir_name = exe_dir.string();

	if(argc < 2) {
		std::cerr << "error: missing absolute path to image." << std::endl; 
		exit(1);
	}
	auto image_path = std::filesystem::path(argv[1]);

	platform.setExeFolder(exe_dir_name);
	platform.hot_reload();

	std::cout << "Testing lunasvg" << std::endl;

	glutInit(&argc, argv);

	glutInitWindowSize(800, 600);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

	glutCreateWindow(argv[0]);

	my_init();
	doVectorize(image_path.string());

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

