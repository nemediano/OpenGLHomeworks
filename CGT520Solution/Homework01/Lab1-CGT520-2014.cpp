#include <GL/glew.h>
#include <GL/freeglut.h>

//This is a bug on GLM in 32 bits I hope they solve it soon
#define GLM_FORCE_PURE
#define GLM_MESSAGES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdlib>
#include <iostream>

#include "OpenGLProgram.h"

using namespace shaders;
using namespace std;

static int win = 0;
OpenGLProgram* program_handler_ptr = nullptr;

//Menu handling
GLint rotate_direction_menu;
GLint background_color_menu;
void change_rotation_direction(int id);
void change_background_color(int id);
void main_menu_handling(int id);

//Callback function
void mouse (int button, int state, int mouse_x, int mouse_y);

//Program logic handler
GLboolean rotation_sense;

GLint uM_loc = -1;
GLint uColor_loc = -1;
float angle = 0.0f;

void create_menus();

void exitGlut() {
   glutDestroyWindow(win);
   exit(EXIT_SUCCESS);
}

void initOpenGL() {
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
	}
	cout << "Hardware specification: " << endl;
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "Software specification: " << endl;
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;
	cout << "Using OpenGL " << glGetString(GL_VERSION) << endl;
	cout << "Using GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	int ver = glutGet(GLUT_VERSION);
	cout << "Using freeglut version: " <<  ver / 10000 << "." << (ver / 100) % 100 << "." << ver % 100 << endl;
	
	program_handler_ptr = new OpenGLProgram("lab1_vs.glsl", "lab1_fs.glsl");
	if (!program_handler_ptr->is_ok()) {
		cerr << "Problem at OpenGL program creation" << endl;
		exit(EXIT_FAILURE);
	}

	uM_loc = program_handler_ptr->get_uniform_location("uM");
	uColor_loc = program_handler_ptr->get_uniform_location("uColor");

	//initialize some basic rendering state
	change_background_color(2);
	glEnable(GL_DEPTH_TEST);
}

void display() {
	program_handler_ptr->use_program();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//set color to draw with
	glUniform4f(uColor_loc, 1.0f, 0.0f, 0.0f, 1.0f);

	//draw orbiting square
	glm::mat4 I(1.0);
	glm::mat4 S, T, R;

    S = glm::scale(I, glm::vec3(0.5f, 0.5f, 0.5f));
	T = glm::translate(I, glm::vec3(0.5f, 0.0f, 0.0f));
	R = glm::rotate(I, rotation_sense ? angle : -angle, glm::vec3(0.0f, 0.0f, 1.0f));
	
	glUniformMatrix4fv(uM_loc, 1, GL_FALSE, glm::value_ptr(R * T * S));
	glutSolidCube(1.0);

	//draw grid
	glUniform4f(uColor_loc, 0.0f, 0.0f, 1.0f, 1.0f);
	S = glm::scale(I, glm::vec3(0.05f, 0.05f, 0.05f));

	for (float x = -1.0; x <= +1.0; x += 0.125f) {
		for (float y = -1.0; y <= +1.0; y += 0.125f) {
			T = glm::translate(I, glm::vec3(x, y, 1.0f));
			glUniformMatrix4fv(uM_loc, 1, GL_FALSE, glm::value_ptr(T * S));
			glutSolidCube(1.0);
		}
	}
	glutSwapBuffers();
	shaders::gl_error("At the end of display");

}

void keyboard(unsigned char key, int x, int y) {
   if(key == 27) { //press ESC to exit
	  exitGlut();
   }  
}


void idle() {  
   //timers for time-based animation
   static int last_time = 0;
   int time = glutGet(GLUT_ELAPSED_TIME);
   int elapsed = time - last_time;
   float delta_seconds = 0.001f * elapsed;
   last_time = time;

   angle = angle + 180.0f * delta_seconds;

   shaders::gl_error("At the end of idle");
   glutPostRedisplay();
}

void createGlutWindow() {
	glutInitDisplayMode (GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize (512, 512);
	win = glutCreateWindow ("Jorge Garcia, Project 1");
	create_menus();
}

void createGlutCallbacks() {
	glutDisplayFunc		(display);
	glutIdleFunc		(idle);
	glutKeyboardFunc	(keyboard);
	glutMouseFunc		(mouse);
}

int main (int argc, char* argv[]) {

	glutInit(&argc, argv);
	createGlutWindow();
	createGlutCallbacks();
	
	initOpenGL();	
	glutMainLoop();
	glutDestroyWindow(win);

	return EXIT_SUCCESS;
}

void create_menus() {
	
	rotate_direction_menu = glutCreateMenu(change_rotation_direction);
    glutAddMenuEntry("Clockwise", 1);
    glutAddMenuEntry("Counterclockwise", 2);
    
	background_color_menu = glutCreateMenu(change_background_color);
    glutAddMenuEntry("Black", 1);
    glutAddMenuEntry("White", 2);

    glutCreateMenu(main_menu_handling);
    glutAddSubMenu("Direction sense", rotate_direction_menu);
    glutAddSubMenu("Background color", background_color_menu);
    glutAddMenuEntry("Quit", 42);

    glutAttachMenu(GLUT_RIGHT_BUTTON);
}


void change_rotation_direction(int id) {
	switch(id) {
		case 1:
			rotation_sense = false;
		break;

		case 2:
			rotation_sense = true;
		break;
	}
}

void change_background_color(int id) {
	switch(id) {
		case 1:
			glClearColor(0.0, 0.0, 0.0, 1.0);
		break;

		case 2:
			glClearColor(1.0, 1.0, 1.0, 1.0);
		break;
	}
}

void main_menu_handling(int id) {
	switch(id) {
		case 42:
			exitGlut();
		break;
	}
}

void mouse (int button, int state, int mouse_x, int mouse_y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		std::wcout << "Mouse coordinates: (" << mouse_x << ", " << mouse_y << ")" << std::endl;
	}
	glutPostRedisplay();
}
