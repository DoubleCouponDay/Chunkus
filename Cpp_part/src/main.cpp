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

#include "gui_data.h"
#include "interop.h"

GUIData data{};
interop platform{};

void doVectorize(std::string image_path, GUIData& guiData)
{
	std::cout << "About to vectorize: " << image_path << std::endl;
	vectorize_options options;
	options.chunk_size = 1;
	options.file_path = (char*)image_path.c_str();
	options.threshold = 1.f;

	std::cout << "Vectorizing '" << image_path << "' with chunk size: " << options.chunk_size << ", threshold: " << options.threshold << " and num colors: " << 256 << std::endl;
	int code = 0;//do_vectorize(data.filename, data.outputfilename, data.chunk_size, data.threshold, 256);
	std::cout << "Vectorized with code: " << code << std::endl;

	guiData.algorithmData = get_algorithm_data();
	guiData.visuals = generate_visuals(data.algorithmData);
	guiData.sidebar.updateFromVisuals(data.visuals);

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
			std::cout << "Input image was not loaded correctly" << std::endl;
			return;
		}
		else
		{
			data.inputTexture = WomboTexture(std::move(tex));
			data.intermediateTexture = WomboTexture(Texture8{ Colors::Orange8, 369, 342 });

			auto doc = lunasvg::Document::loadFromFile("output.svg");
			lunasvg::Bitmap bitmap{};
			if (doc)
				bitmap = doc->renderToBitmap((uint32_t)data.inputTexture.getWidth(), (uint32_t)data.inputTexture.getHeight());

			if (!bitmap.valid())
			{
				std::cout << "Failed to render the svg output" << std::endl;
				data.vectorizedTexture.clear();
			}
			else
			{
				data.vectorizedTexture = WomboTexture(bitmap);
			}
		}
	}
}

void updateVisuals(GUIData& data)
{
	data.visuals = generate_visuals(data.algorithmData);
	data.sidebar.updateFromVisuals(data.visuals);
}

void stepForward(GUIData& data)
{

}

void stepBackward(GUIData& data)
{

}

constexpr int textureAreaStart = 150;
constexpr int buttonSize = 40;

void sizeButtons()
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
	data.windowSize = { glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT) };
	data.textureArea = Box{ Vector2i{ 50, textureAreaStart }, Vector2i{ data.windowSize.x - 50, data.windowSize.y } };

	sizeButtons();
	checkForGlError("Post make buttons");

	auto buttons = {
		&data.quitButton, 
		&data.switchInputButton, 
		&data.switchInterButton, 
		&data.switchVectorButton, 
		&data.leftButton, 
		&data.rightButton, 
		&data.writeToBmpButton
	};

	data.buttons.insert(data.buttons.begin(), buttons.begin(), buttons.end());

	data.sidebar = Sidebar{ Box{ windowToGL(Vector2i{ 5, 400 }, guiData.windowSize), windowToGL(Vector2i{ 150, 20 }, guiData.windowSize) }, std::vector<SidebarButton>(), Colors::Grey32 };
	data.sidebar.addButton(
		SidebarButton{ Vector2u{ 135, 32 + 10 }, "150", Colors::Orange32, Colors::Pink32 }
	);
	data.sidebar.addButton(
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
	data.windowSize.x = w;
	data.windowSize.y = h;
	data.textureArea = Box{ Vector2i{ 50, textureAreaStart }, Vector2i{ data.windowSize.x - 50, data.windowSize.y } };

	sizeButtons();

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
		if (data.activeTexture == ActiveTexture::INTERMEDIATE)
		{
			auto& tex = data.getActiveTexture();
			renderAlgorithm(data.visuals, pow(1.1, data.scrollage), data.textureArea, data.selectedGroup, data.windowSize);
		}
		else
		{
			auto& tex = data.getActiveTexture();
			drawVecTextureArea(tex.getGLTex(), tex.getCpuTex().getWidth(), tex.getCpuTex().getHeight(), { 0,0,0 }, pow(1.1, data.scrollage), data.textureArea, data.windowSize);
		}

		// Draw active texture string
		int activeTexStrLen = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)data.getCurrentText());
		int fontHeight = glutBitmapHeight(GLUT_BITMAP_HELVETICA_18);
		renderString(Box{ Vector2i{ data.windowSize.x / 2 - activeTexStrLen / 2, textureAreaStart - buttonSize * 2 - fontHeight - 3 }, Vector2u{ (unsigned int)activeTexStrLen, (unsigned int)fontHeight + 6 } }, GLUT_BITMAP_HELVETICA_18, data.getCurrentText(), Colors::White32);
		checkForGlError("Render active texture string");

		// Draw status string
		renderString(5, 5, GLUT_BITMAP_HELVETICA_18, data.statusString, Colors::White32);
		checkForGlError("Render status string");

		// Draw buttons
		for (auto& button : data.buttons)
			renderButton(*button, data.windowSize);
		checkForGlError("Render buttons");

		data.sidebar.render(data.windowSize);
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

void onMouseButton(int button, int state, int mouseX, int mouseY)
{
	auto& guiData = *reinterpret_cast<GUIData*>(glutGetWindowData());

	auto glCoords = windowToGL({ mouseX, mouseY }, guiData.windowSize);
	bool withinQuit = data.quitButton.isWithin(glCoords);
	bool withinLeft = data.leftButton.isWithin(glCoords);
	bool withinRight = data.rightButton.isWithin(glCoords);
	bool withinInput = data.switchInputButton.isWithin(glCoords);
	bool withinInter = data.switchInterButton.isWithin(glCoords);
	bool withinVector = data.switchVectorButton.isWithin(glCoords);
	bool withinWrite = data.writeToBmpButton.isWithin(glCoords);
	bool withinSidebar = data.sidebar.isWithin(glCoords);

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
				data.activeTexture = ActiveTexture::INPUT;
				glutPostRedisplay();
			}
			if (withinInter)
			{
				std::cout << "Switch to Intermediate Button clicked" << std::endl;
				data.activeTexture = ActiveTexture::INTERMEDIATE;
				glutPostRedisplay();
			}
			if (withinVector)
			{
				std::cout << "Switch to Vectorized output Button clicked" << std::endl;
				data.activeTexture = ActiveTexture::VECTORIZED;
				glutPostRedisplay();
			}
			if (withinWrite)
			{
				std::cout << "Write to BMP was clicked" << std::endl;
				data.getActiveTexture().getCpuTex().writeToBmp("test.bmp");
			}
			if (withinLeft)
			{
				std::cout << "Left button clicked" << std::endl;
				stepBackward(guiData);
			}
			if (withinRight)
			{
				std::cout << "Right button clicked" << std::endl;
				stepForward(guiData);
			}
			if (withinSidebar)
			{
				std::cout << "Sidebar clicked" << std::endl;
				int button = guiData.sidebar.getButtonClicked(glCoords);
				if (button != -1)
				{
					std::cout << "Sidebar button " << button << " clicked" << std::endl;
				}
				guiData.selectedGroup = button;
			}
		}
	}
}

void onMouseWheel(int button, int dir, int x, int y)
{
	data.scrollage += dir;
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
	glutMouseFunc(onMouseButton);
	glutMouseWheelFunc(onMouseWheel);
	glutReshapeFunc(onResize);

	checkForGlError("Before Glut Main");
	glutMainLoop();

	return 0;
}

