#include <memory>
#include <fstream>
#include <iostream>
#include <string>
#include <string.h>

#include <GL/glut.h>

#include "texture.h"
#include "gl.h"



GLuint genGLTexture(const Texture& tex)
{
	GLuint glTex[] = { 0, 0 };
	checkForGlError("blah");
	glGenTextures(2, &glTex[0]);
	checkForGlError("blah");
	glBindTexture(GL_TEXTURE_2D, glTex[0]);
   
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex.getWidth(), tex.getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)tex.getData());
	

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
	auto tex = Texture("epic.bmp", false);
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

