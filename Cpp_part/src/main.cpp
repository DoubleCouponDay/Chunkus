#include <memory>
#include <fstream>
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <cmath>
#include <array>
#include <algorithm>
#include <filesystem>

#include <GL/freeglut.h>

#include <lunasvg.h>

#include "texture.h"
#include "gl.h"

#include "../../C_part/src/entrypoint.h"

#include "gui_data.h"
#include "interop.h"

interop platform{};

WomboTexture renderStep(vectorize_options options, int width, int height)
{
	// Eventually have this cache the output and avoid rerunning for a cached step
	std::cout << "Vectorizing to step: " << options.step_index << " with threshold: " << options.threshold << std::endl;

	int code = execute_program(options);

	if (code != 0)
	{
		std::cout << "Error vectorizing: " << code << std::endl;
		return WomboTexture{};
	}
	
	std::cout << "Successfully vectorized to step " << options.step_index << std::endl;

	std::string expected_output_path = "output.svg"; // TODO: replace with real output path

	auto doc = lunasvg::Document::loadFromFile(expected_output_path);

	if (!doc)
	{
		std::cout << "Error: Failed to load document, likely caused by invalid svg or non-existent file" << std::endl;
		return WomboTexture{};
	}

	lunasvg::Bitmap bitmap = doc->renderToBitmap(width, height);

	if (!bitmap.valid())
	{
		std::cout << "Error: Failed to render document" << std::endl;
		return WomboTexture{};
	}

	std::cout << "Vectorized and rendered step " << options.step_index << std::endl;
	return WomboTexture(bitmap);
}

void doVectorize(std::string image_path, GUIData& guiData)
{
	std::cout << "About to vectorize: " << image_path << std::endl;
	guiData.file_path = image_path;
	vectorize_options& options = guiData.options;
	options.chunk_size = 1;
	options.file_path = guiData.file_path.c_str();
	options.threshold = 1.f;
	options.step_index = 0;
	options.num_colours = 255;

	std::cout << "Vectorizing '" << image_path << "' with chunk size: " << options.chunk_size << ", threshold: " << options.threshold << " and num colors: " << 256 << std::endl;

		Texture8 tex = Texture8{ image_path };
		if (tex.getBytes() == nullptr)
		{
			std::cout << "Input image was not loaded correctly" << std::endl;
			return;
		}
		
	guiData.inputTexture = WomboTexture{ std::move(tex) };
	options.step_index = 1;
	guiData.intermediateTexture = WomboTexture{ renderStep(options, guiData.inputTexture.getWidth(), guiData.inputTexture.getHeight()) };
	options.step_index = 0;
	guiData.vectorizedTexture = WomboTexture{ renderStep(options, guiData.inputTexture.getWidth(), guiData.inputTexture.getHeight()) };
	options.step_index = 1;
}

void updateVisuals(GUIData& data)
{
	data.intermediateTexture = renderStep(data.options, data.inputTexture.getWidth(), data.inputTexture.getHeight());
	glutPostRedisplay();
}

void doStep(GUIData& data)
{
	data.inputField.setText(std::to_string(data.options.step_index));
	data.inputFieldSelected = false;
	updateVisuals(data);
}

void stepForward(GUIData& data)
{
	data.options.step_index++;
	updateVisuals(data);
}

void stepBackward(GUIData& data)
{
	data.options.step_index = std::max((int64_t)0, data.options.step_index - 1);
	updateVisuals(data);
}

constexpr int textureAreaStart = 150;
constexpr int buttonSize = 40;

void sizeButtons(GUIData& data)
{
	data.quitButton = Button{ Vector2i{ data.windowSize.x - 50, 0 }, Vector2u{ 50, 32 }, "Quit", Colors::Grey32 };
	data.leftButton = Button{ Vector2i{ data.windowSize.x / 2 - buttonSize, textureAreaStart - buttonSize }, Vector2u{ buttonSize, buttonSize }, "<", Colors::Grey32 };
	data.rightButton = Button{ Vector2i{ data.windowSize.x / 2, textureAreaStart - buttonSize }, Vector2u{ buttonSize, buttonSize }, ">", Colors::Grey32 };

	int switchInputLength = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)"Input") + 6;
	int switchIntermediateLength = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)"Intermediate") + 6;
	int switchOutputLength = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)"Output") + 6;
	int writeToBmpLength = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)"Write to bmp") + 6;

	int firstSwitchPosition = data.windowSize.x / 2 - (switchIntermediateLength / 2) - switchInputLength;
	int secondSwitchPosition = firstSwitchPosition + switchInputLength;
	int thirdSwitchPosition = secondSwitchPosition + switchIntermediateLength;

	data.switchInputButton = Button{ Vector2i{ firstSwitchPosition, textureAreaStart - buttonSize * 2},	Vector2u{ (unsigned int)switchInputLength, buttonSize},	"Input", Colors::Grey32 };
	data.switchInterButton = Button{ Vector2i{ secondSwitchPosition, textureAreaStart - buttonSize * 2 }, Vector2u{ (unsigned int)switchIntermediateLength, buttonSize }, "Intermediate", Colors::Grey32 };
	data.switchVectorButton = Button{ Vector2i{ thirdSwitchPosition, textureAreaStart - buttonSize * 2 },	Vector2u{ (unsigned int)switchOutputLength, buttonSize }, "Output", Colors::Grey32 };
	data.writeToBmpButton = Button{ Vector2i{ data.windowSize.x / 2 - writeToBmpLength / 2, textureAreaStart - buttonSize * 3 }, Vector2u{ (unsigned int)writeToBmpLength, buttonSize }, "Write to bmp", Colors::Grey32 };
}

void my_init(GUIData& guiData)
{
	checkForGlError("Beginning of my_init");
	guiData.windowSize = { glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT) };
	guiData.textureArea = Box{ Vector2i{ 50, textureAreaStart }, Vector2i{ guiData.windowSize.x - 50, guiData.windowSize.y } };

	sizeButtons(guiData);
	checkForGlError("Post make buttons");

	auto buttons = {
		&guiData.quitButton, 
		&guiData.switchInputButton, 
		&guiData.switchInterButton, 
		&guiData.switchVectorButton, 
		&guiData.leftButton, 
		&guiData.rightButton, 
		&guiData.writeToBmpButton
	};

	guiData.buttons.insert(guiData.buttons.begin(), buttons.begin(), buttons.end());

	guiData.sidebar = Sidebar{ Box{ (Vector2i)windowToGLClipped(Vector2i{ 5, 400 }, guiData.windowSize), windowToGLClipped(Vector2i{ 150, 20 }, guiData.windowSize) }, std::vector<SidebarButton>(), Colors::Grey32 };
	guiData.sidebar.addButton(
		SidebarButton{ Vector2u{ 135, 32 + 10 }, "150", Colors::Orange32, Colors::Pink32 }
	);
	guiData.sidebar.addButton(
		SidebarButton{ Vector2u{ 135, 32 + 10 }, "32", Colors::Yellow32, Colors::Green32 }
	);

	guiData.inputField = TextField(Vector2i{ 50, 0 }, Vector2u{ 150, 32 }, "0", Colors::Grey32, Colors::White32, Colors::White32, true);
	checkForGlError("Create TextField");

	checkForGlError("Post Init");
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_SCISSOR_TEST);
	checkForGlError("Post set gl things");
}


void onResize(int w, int h)
{
	auto& guiData = *reinterpret_cast<GUIData*>(glutGetWindowData());

	guiData.windowSize.x = w;
	guiData.windowSize.y = h;
	guiData.textureArea = Box{ Vector2i{ 50, textureAreaStart }, Vector2i{ guiData.windowSize.x - 50, guiData.windowSize.y } };

	sizeButtons(guiData);

	glViewport(0, 0, w, h);
}

/// <summary>
/// Display callback, called whenever the window needs to be redrawn
/// </summary>
void display()
{
	auto& data = *reinterpret_cast<GUIData*>(glutGetWindowData());

	// Things to draw:
	// - Active texture
	// - Buttons
	// - Status string
	// - Filename string
	data.windowSize.x = glutGet(GLUT_WINDOW_WIDTH);
	data.windowSize.y = glutGet(GLUT_WINDOW_HEIGHT);
	checkForGlError("Displaying");
	glClear(GL_COLOR_BUFFER_BIT);
	checkForGlError("Cleared the back buffer");

	{
		auto proj = GLMatrix(GL_PROJECTION);

		gluOrtho2D(0.0, data.windowSize.x, 0.0, data.windowSize.y);

		// Draw active texture
		glColor3f(data.texColor.R, data.texColor.G, data.texColor.B);
		
		auto& tex = data.getActiveTexture();
		drawVecTextureArea(tex.getGLTex(), tex.getCpuTex().getWidth(), tex.getCpuTex().getHeight(), { 0,0,0 }, pow(1.1, data.scrollage), data.textureArea, data.windowSize);

		// Draw active texture string
		int activeTexStrLen = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)data.getCurrentText());
		int fontHeight = glutBitmapHeight(GLUT_BITMAP_HELVETICA_18);
		renderString(Box{ Vector2i{ data.windowSize.x / 2 - activeTexStrLen / 2, 0 }, Vector2u{ (unsigned int)activeTexStrLen, (unsigned int)fontHeight + 6 } }, GLUT_BITMAP_HELVETICA_18, data.getCurrentText(), Colors::White32);
		checkForGlError("Render active texture string");
	
		// Draw status string
		renderString(5, 5, GLUT_BITMAP_HELVETICA_18, data.statusString, Colors::White32);
		checkForGlError("Render status string");

		// Draw buttons
		for (auto& button : data.buttons)
			renderButton(*button, data.windowSize);
		checkForGlError("Render buttons");

		// Draw text field
		renderString(0, 5, GLUT_BITMAP_HELVETICA_18, "Step:", Colors::White32);
		data.inputField.render(data.windowSize, data.inputFieldSelected);

		//data.sidebar.render(data.windowSize);
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
	auto& data = *reinterpret_cast<GUIData*>(glutGetWindowData());

	if (data.inputFieldSelected)
	{
		if (key == 13) // 13 is the ASCII Carriage Return control code
		{
			data.inputFieldSelected = false;
			try
			{
				data.options.step_index = std::stoll(data.inputField.getText());
				updateVisuals(data);
				glutPostRedisplay();
			}
			catch (std::exception& e)
			{}
			return;
		}
		if (key == 27) // 27 is the ASCII control code for Escape
		{
			data.inputFieldSelected = false;
			return;
		}
		data.inputField.insert(key);
		glutPostRedisplay();
		return;
	}

	if (key == 'r')
	{
		std::cout << "uwu r key pressed" << std::endl;
	}
	if (key == '1')
	{
		std::cout << "Switching to input image" << std::endl;

		data.activeTexture = ActiveTexture::INPUT;
		glutPostRedisplay();
	}
	if (key == '2')
	{
		std::cout << "Switching to intermediate image" << std::endl;

		data.activeTexture = ActiveTexture::INTERMEDIATE;
		glutPostRedisplay();
	}
	if (key == '3')
	{
		std::cout << "Switching to output image" << std::endl;

		data.activeTexture = ActiveTexture::VECTORIZED;
		glutPostRedisplay();
	}
}

void onSpecialKey(int key, int mouseX, int mouseY)
{
	auto& data = *reinterpret_cast<GUIData*>(glutGetWindowData());

	if (data.inputFieldSelected)
	{
		if (key == GLUT_KEY_LEFT)
		{
			data.inputField.moveLeft();
			glutPostRedisplay();
		}
		else if (key == GLUT_KEY_RIGHT)
		{
			data.inputField.moveRight();
			glutPostRedisplay();
		}
	}
}

void onMouseButton(int button, int state, int mouseX, int mouseY)
{
	auto& guiData = *reinterpret_cast<GUIData*>(glutGetWindowData());

	auto glCoords = windowToGL({ mouseX, mouseY }, guiData.windowSize);
	bool withinTextField = guiData.inputField.isWithin(glCoords);
	bool withinQuit = guiData.quitButton.isWithin(glCoords);
	bool withinLeft = guiData.leftButton.isWithin(glCoords);
	bool withinRight = guiData.rightButton.isWithin(glCoords);
	bool withinInput = guiData.switchInputButton.isWithin(glCoords);
	bool withinInter = guiData.switchInterButton.isWithin(glCoords);
	bool withinVector = guiData.switchVectorButton.isWithin(glCoords);
	bool withinWrite = guiData.writeToBmpButton.isWithin(glCoords);
	bool withinSidebar = guiData.sidebar.isWithin(glCoords);

	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			if (withinQuit)
			{
				std::cout << "Quit Button clicked" << std::endl;
				glutExit();
			}
			if (withinTextField)
			{
				std::cout << "Text Field clicked" << std::endl;
				guiData.inputFieldSelected = true;
				guiData.inputField.update(guiData.windowSize, glCoords, button, state);
			}
			else
			{
				guiData.inputFieldSelected = false;
			}
			if (withinInput)
			{
				std::cout << "Switch to Input Button clicked" << std::endl;
				guiData.activeTexture = ActiveTexture::INPUT;
				glutPostRedisplay();
			}
			if (withinInter)
			{
				std::cout << "Switch to Intermediate Button clicked" << std::endl;
				guiData.activeTexture = ActiveTexture::INTERMEDIATE;
				glutPostRedisplay();
			}
			if (withinVector)
			{
				std::cout << "Switch to Vectorized output Button clicked" << std::endl;
				guiData.activeTexture = ActiveTexture::VECTORIZED;
				glutPostRedisplay();
			}
			if (withinWrite)
			{
				std::cout << "Write to BMP was clicked" << std::endl;
				guiData.getActiveTexture().getCpuTex().writeToBmp("output.bmp");
			}
			if (withinLeft)
			{
				std::cout << "Left button clicked" << std::endl;
				stepBackward(guiData);
				guiData.inputField.setText(std::to_string(guiData.options.step_index));
			}
			if (withinRight)
			{
				std::cout << "Right button clicked" << std::endl;
				stepForward(guiData);
				guiData.inputField.setText(std::to_string(guiData.options.step_index));
			}
		}
	}
}

void onMouseWheel(int button, int dir, int x, int y)
{
	auto& guiData = *reinterpret_cast<GUIData*>(glutGetWindowData());

	guiData.scrollage += dir;
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

	GUIData data;

	my_init(data);
	doVectorize(image_path.string(), data);
	
	glutSetWindowData(reinterpret_cast<void*>(&data));

	glutDisplayFunc(display);

	glutTimerFunc(3000, setTexPixel, 0);

	glutKeyboardFunc(onKeyboardButton);
	glutSpecialFunc(onSpecialKey);
	glutMouseFunc(onMouseButton);
	glutMouseWheelFunc(onMouseWheel);
	glutReshapeFunc(onResize);

	checkForGlError("Before Glut Main");
	glutMainLoop();

	return 0;
}

