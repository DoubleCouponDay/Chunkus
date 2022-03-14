#include <GL/glut.h>


void my_init()
{
	glDisable(GL_LIGHTING);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(0.f, 0.f, 0.f);
	glColor3f(1.f, 0.f, 0.f);
	glutSolidCube(0.1);

	glutSwapBuffers();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitWindowSize(800, 600);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

	my_init();

	glutCreateWindow(argv[0]);

	glutDisplayFunc(display);

	glutMainLoop();

	return 0;
}

