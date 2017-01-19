#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GL/freeglut.h>

#define GLM_FORCE_PURE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/* Include heders and namespaces from my CGT Library */
#include "OGLHelpers.h"
#include "MathConstants.h"
#include "OGLProgram.h"
#include "Geometries.h"
using namespace ogl;
using namespace math;
using namespace mesh;

/* CGT Library related*/
OGLProgram* programPtr = nullptr;
Mesh* meshPtr = nullptr;

GLint window = 0;
// Location for shader variables
GLint u_PVM_location = -1;
GLint u_Time_location = -1;
GLint a_position_loc = -1;
GLint a_normal_loc = -1;
GLint a_texture_loc = -1;

//Global variables for the program logic
float seconds_elapsed;

void create_glut_window();
void init_program();
void init_OpenGL();
void create_glut_callbacks();
void exit_glut();
void reload_shaders();

//Glut callback functions
void display();
void reshape(int new_window_width, int new_window_height);
void idle();
void keyboard(unsigned char key, int mouse_x, int mouse_y);
void special(int key, int mouse_x, int mouse_y);

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);

	create_glut_window();
	init_OpenGL();

	init_program();

	create_glut_callbacks();
	glutMainLoop();

	return EXIT_SUCCESS;
}


void exit_glut() {
	delete meshPtr;
	delete programPtr;
	/* Delete window (freeglut) */
	glutDestroyWindow(window);
	exit(EXIT_SUCCESS);
}

void create_glut_window() {
	//Set number of samples per pixel
	glutSetOption(GLUT_MULTISAMPLE, 8);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(800, 600);
	window = glutCreateWindow("Render in Fragment");
}

void init_program() {
	using glm::vec3;
	/* Initialize global variables for program control */
	seconds_elapsed = 0.0f;
	/* Create simple plane geometry and send it to GPU */
	meshPtr = new Mesh(Geometries::plane());
	if (meshPtr) {
		meshPtr->sendToGPU();
	}
}

void init_OpenGL() {
	using std::cout;
	using std::cerr;
	using std::endl;
	/************************************************************************/
	/*                    Init OpenGL context   info                        */
	/************************************************************************/
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		cerr << "Error: " << glewGetErrorString(err) << endl;
	}
	cout << getOpenGLInfo() << endl;
	/* Create a default program that should always work */
	programPtr = new OGLProgram("shaders/simpleVertex.vert", "shaders/simple.frag");

	reload_shaders();
	
	//Activate antialliasing
	glEnable(GL_MULTISAMPLE);

	//Initialize some basic rendering state
	glEnable(GL_DEPTH_TEST);
	//Dark black background color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

}

void reload_shaders() {
	using std::cerr;
	using std::endl;
	/************************************************************************/
	/*                   OpenGL program creation                            */
	/************************************************************************/
	auto tmpProgram = new OGLProgram("shaders/simpleVertex.vert", "shaders/experiment.frag");
	
	if (!tmpProgram || !tmpProgram->isOK()) {
		cerr << "Something wrong in shader" << endl;
		delete tmpProgram;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	} else {
		programPtr = tmpProgram;
	}

	/************************************************************************/
	/* Allocating variables for shaders                                     */
	/************************************************************************/
	u_PVM_location = programPtr->uniformLoc("PVM");
	u_Time_location = programPtr->uniformLoc("time");
	a_position_loc = programPtr->attribLoc("Position");
	a_normal_loc = programPtr->attribLoc("Normal");
	a_texture_loc = programPtr->attribLoc("TextCoord");
}


void create_glut_callbacks() {
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);

}

void reshape(int new_window_width, int new_window_height) {
	glViewport(0, 0, new_window_width, new_window_height);
}

void display() {
	using glm::vec3;
	using glm::vec4;
	using glm::vec2;
	using glm::mat4;
	using namespace std;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	programPtr->use();

	mat4 I(1.0f);
	//Model
	mat4 M = glm::scale(I, vec3(2.0f));
	//View
	mat4 V = glm::lookAt(vec3(0.0f, 0.0f, 1.0f), vec3(0.0f), vec3(0.0, 1.0, 0.0));
	//Projection
	/*In ortho as in orthoRH, the far and near coordinate are inverted. 
	Either pass it inverted or use orthoLH*/
	mat4 P = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f);
	
	/************************************************************************/
	/* Send uniform values to shader                                        */
	/************************************************************************/
	if (u_PVM_location != -1) {
		glUniformMatrix4fv(u_PVM_location, 1, GL_FALSE, glm::value_ptr(P * V * M));
	}
	if (u_Time_location != -1) {
		glUniform1f(u_Time_location, seconds_elapsed);
	}
	/* Draw */
	meshPtr->drawTriangles(a_position_loc, a_normal_loc, a_texture_loc);

	//Unbind an clean
	glUseProgram(0);

	glutSwapBuffers();
}

void idle() {
	static int last_time = 0;

	int time = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = time - last_time;
	last_time = time;
	float delta_seconds = 0.001f * elapsed;
	seconds_elapsed += delta_seconds;

	glutPostRedisplay();
}

void keyboard(unsigned char key, int mouse_x, int mouse_y) {
	switch (key) {

	case 'R':
	case 'r':
		reload_shaders();
		break;

	case 27:
		exit_glut();
		break;
	default:
		break;
	}

	glutPostRedisplay();
}



void special(int key, int mouse_x, int mouse_y) {
	
	/* Now, the app*/
	glutPostRedisplay();
}


