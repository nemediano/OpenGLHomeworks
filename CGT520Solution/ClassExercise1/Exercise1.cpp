#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>

#include <cstdlib>
#include <iostream>

//This is a bug on GLM in 32 bits I hope they solve it soon
#define GLM_FORCE_PURE
#define GLM_MESSAGES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include "OpenGLProgram.h"

using namespace std;
using namespace shaders;

static int win = 0;

OpenGLProgram* gl_program_ptr = nullptr;

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
	cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Software specification: " << endl;
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;
	cout << "Using OpenGL " << glGetString(GL_VERSION) << endl;
	cout << "Using GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	int ver = glutGet(GLUT_VERSION);
	cout << "Using freeglut version: " <<  ver / 10000 << "." << (ver / 100) % 100 << "." << ver % 100 << endl;

	gl_program_ptr = new OpenGLProgram("exe1_vs.glsl", "exe1_fs.glsl");
	if (!gl_program_ptr->is_ok()) {
		cerr << "Error at GL program creation" << endl;
		gl_error("At program creation");
		exit(EXIT_FAILURE);
	}

	uM_loc = gl_program_ptr->get_uniform_location("uM");
	uColor_loc = gl_program_ptr->get_uniform_location("uColor");

	//initialize some basic rendering state
	change_background_color(2);
	glEnable(GL_DEPTH_TEST);
}

void display() {
	gl_program_ptr->use_program();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//set color to draw with
	glUniform4f(uColor_loc, 1.0f, 0.0f, 0.0f, 1.0f);

	//draw orbiting square
	glm::mat4 I = glm::mat4(1.0f);
	glm::mat4 S = glm::scale(I, glm::vec3(0.25f, 0.25f, 0.25f));
	glm::mat4 T = glm::translate(I, glm::vec3(0.5f, 0.0f, 0.0f));
	glm::mat4 R = glm::rotate(I, rotation_sense ? angle : -angle, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(uM_loc, 1, GL_FALSE,  glm::value_ptr(R * T * S));
	glutSolidTeapot(1.0f);

	//draw grid
	glUniform4f(uColor_loc, 0.0f, 0.0f, 1.0f, 1.0f);
	glm::vec4 black = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 blue = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	glm::vec4 red = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	S = glm::scale(I, glm::vec3(0.05f, 0.05f, 0.05f));
	for (float x = -1.0; x <= +1.0; x += 0.125f) {
		glm::vec4 color = ((x + 1.0f) / 2.0f) * red + x * black;
		glUniform4fv(uColor_loc, 1, glm::value_ptr(color));
		for (float y = -1.0; y <= +1.0; y += 0.125f) {
			R =glm::rotate(I, rotation_sense ? angle : -angle, glm::vec3(0.0f, 0.0f, 1.0f));
			T = glm::translate(I, glm::vec3(x, y, 1.0f));
			glUniformMatrix4fv(uM_loc, 1, GL_FALSE, glm::value_ptr(T * S * R));
			glutSolidCube(1.0);
		}
	}
	glutSwapBuffers();
	glUseProgram(0);

	gl_error("At display");

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

   gl_error("At idle");
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
