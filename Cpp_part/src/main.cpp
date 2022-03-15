#include <memory>
#include <fstream>
#include <iostream>
#include <string>
#include <string.h>

#include <GL/glut.h>

class Texture
{
public:
	std::unique_ptr<char[]> Data;
	GLuint Width;
	GLuint Height;
};

// sauce: stackoverflow
char const* gl_error_string(GLenum const err) noexcept
{
  switch (err)
  {
    // opengl 2 errors (8)
    case GL_NO_ERROR:
      return "GL_NO_ERROR";

    case GL_INVALID_ENUM:
      return "GL_INVALID_ENUM";

    case GL_INVALID_VALUE:
      return "GL_INVALID_VALUE";

    case GL_INVALID_OPERATION:
      return "GL_INVALID_OPERATION";

    case GL_STACK_OVERFLOW:
      return "GL_STACK_OVERFLOW";

    case GL_STACK_UNDERFLOW:
      return "GL_STACK_UNDERFLOW";

    case GL_OUT_OF_MEMORY:
      return "GL_OUT_OF_MEMORY";

    case GL_TABLE_TOO_LARGE:
      return "GL_TABLE_TOO_LARGE";

    // opengl 3 errors (1)
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      return "GL_INVALID_FRAMEBUFFER_OPERATION";

    // gles 2, 3 and gl 4 error are handled by the switch above
    default:
      return "UNKNOWN_ERROR";
  }
}

void printGlErr()
{
	GLenum err = glGetError();

	if (err != GL_NO_ERROR)
	{
		std::cout << "GL error! " << gl_error_string(err) << std::endl;
	}
}

// sauce: http://nehe.gamedev.net/
Texture loadTextureFromFile(std::string fileName, bool flipY)
{
	Texture out{};
	printf("Reading image %s\n", fileName.c_str());
	out.Width = -1;
	out.Height = -1;
	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;


	FILE * file = fopen(fileName.c_str(),"rb");
	if (!file)							    {printf("Image could not be opened\n"); return Texture{};}


	if ( fread(header, 1, 54, file)!=54 ){	
		printf("Not a correct BMP file\n");
		return Texture{};
	}

	if ( header[0]!='B' || header[1]!='M' ){
		printf("Not a correct BMP file\n");
		return Texture{};
	}
	if ( *(int*)&(header[0x1E])!=0  )         {printf("Not a correct BMP file\n");		return Texture{};}
	if ( *(int*)&(header[0x1C])!=24 )         {printf("Not a correct BMP file\n");		return Texture{};}


	dataPos    = *(int*)&(header[10]);
	imageSize  = *(int*)&(header[34]);
	out.Width      = *(int*)&(header[18]);
	out.Height     = *(int*)&(header[22]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize==0)    imageSize=out.Width*out.Height*3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos==0)      dataPos=54; // The BMP header is done that way

	// Create a buffer
	out.Data = std::make_unique<char[]>(imageSize);

	// Read the actual data from the file into the buffer
	fread(out.Data.get(),1,imageSize,file);

	// Everything is in memory now, the file wan be closed
	fclose (file);

	if (flipY){
		// swap y-axis
		unsigned char * tmpBuffer = new unsigned char[out.Width*3];
		int size = out.Width*3;
		for (int i=0;i<out.Height/2;i++){

			memcpy(tmpBuffer,out.Data.get()+out.Width*3*i,size);

			memcpy(out.Data.get()+out.Width*3*i, out.Data.get()+out.Width*3*(out.Height-i-1), size);

			memcpy(out.Data.get()+out.Width*3*(out.Height-i-1), tmpBuffer, size);
		}
		delete [] tmpBuffer;
	}

	std::cout << "Loaded image with dims: " << out.Width << "x" << out.Height << std::endl;
	return out;
}

GLuint genGLTexture(const Texture& tex)
{
	GLuint glTex[] = { 0, 0 };
	printGlErr();
	glGenTextures(2, &glTex[0]);
	printGlErr();
	glBindTexture(GL_TEXTURE_2D, glTex[0]);
   
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex.Width, tex.Height, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)tex.Data.get());
	

	std::cout << "Generated epic texture: " << glTex[0] << ", " << glTex[1] << std::endl;
	return glTex[0];
}

void drawTexture(GLuint glTex)
{
	glBindTexture(GL_TEXTURE_2D, glTex);
	glBegin(GL_QUADS);
	// Front Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.35f, -0.6f,  0.f);  // Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.0f);  // Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.6f,  0.3f,  0.0f);  // Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f,  0.0f);  // Top Left Of The Texture and Quad

	glEnd();
}

GLuint myTex = 0;

void my_init()
{
	auto tex = loadTextureFromFile("epic.bmp", false);
	myTex = genGLTexture(tex);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
}

void display()
{	
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(0.f, 0.f, 0.f);
	glColor3f(1.f, 0.f, 1.f);
	glutSolidCube(0.4);
	drawTexture(myTex);

	glutSwapBuffers();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitWindowSize(800, 600);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

	glutCreateWindow(argv[0]);

	my_init();

	glutDisplayFunc(display);

	glutMainLoop();

	return 0;
}

