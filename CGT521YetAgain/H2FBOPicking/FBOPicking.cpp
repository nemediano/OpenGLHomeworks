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
#include <glm/gtc/random.hpp>

/* Include heders and namespaces from my CGT Library */
#include "OGLHelpers.h"
#include "MathConstants.h"
#include "OGLProgram.h"
#include "Geometries.h"
#include "Camera.h"
#include "Trackball.h"
using namespace ogl;
using namespace math;
using namespace mesh;
using namespace camera;
using namespace std;
/*Camera related function*/
Camera cam;
Trackball ball;


/* CGT Library related*/
OGLProgram* programGeomPassPtr = nullptr;
Mesh* meshPtr = nullptr;
GLint window = 0;

struct Locations {
	GLint u_PV_location = -1;
	GLint u_NormMat_location = -1;
	GLint u_Time_location = -1;
	GLint a_position_loc = -1;
	GLint a_normal_loc = -1;
	GLint a_texture_loc = -1;
	//Instance attribute
	GLint a_color_loc = -1;
	GLint a_transformation_loc = -1;
};

Locations loc;


// Location for shader variables

//VBO for instanced attribute
GLuint color_buffer_id = 0;
GLuint transformation_buffer_id = 0;
//Global variables for the program logic
float seconds_elapsed;
glm::vec3 meshCenter;
float scaleFactor;
bool instanciated;
const unsigned int MAX_INSTANCES = 3000;
unsigned int instace_number = 100;

vector<glm::vec3> colors;
vector<glm::mat4> transformations;

void create_glut_window();
void init_program();
void init_OpenGL();
void create_glut_callbacks();
void exit_glut();
void reload_shaders();
void geomPass();
void drawUnInstanciated();

//Glut callback functions
void display();
void reshape(int new_window_width, int new_window_height);
void idle();
/*I am forced to implement all this callbacks, for the GUI to work propertly*/
void keyboard(unsigned char key, int mouse_x, int mouse_y);
void special(int key, int mouse_x, int mouse_y);
void mouse(int button, int state, int x, int y);
void mouseWheel(int button, int dir, int mouse_x, int mouse_y);
void mouseDrag(int mouse_x, int mouse_y);
void mousePasiveMotion(int mouse_x, int mouse_y);

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
	delete programGeomPassPtr;
	/* Delete window (freeglut) */
	glutDestroyWindow(window);
	exit(EXIT_SUCCESS);
}

void create_glut_window() {
	//Set number of samples per pixel
	glutSetOption(GLUT_MULTISAMPLE, 8);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(800, 600);
	window = glutCreateWindow("Jorge Garcia, Homework 1");
}

void init_program() {
	using glm::vec3;
	using glm::mat4;
	/* Initialize global variables for program control */
	seconds_elapsed = 0.0f;
	/* Load a mesh from file and send it to GPU */
	meshPtr = new Mesh("../models/Amago.obj");
	if (meshPtr) {
		meshCenter = meshPtr->getBBCenter();
		scaleFactor = meshPtr->scaleFactor();
		meshPtr->sendToGPU();
	}
	//Set the default position of the camera
	cam.setLookAt(vec3(0.0f, 0.0f, 3.0f), vec3(0.0f));
	cam.setAspectRatio(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	cam.setFovY(PI / 4.0f);
	cam.setDepthView(0.1f, 9.0f);
	//Create trackball camera
	ball.setWindowSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	/*Create a buffers for the instanced atributes */
	glGenBuffers(1, &color_buffer_id);
	glGenBuffers(1, &transformation_buffer_id);
	/* Generate a bunch of random colors*/
	vec3 color;
	for (int i = 0; i < MAX_INSTANCES; ++i) {
		color = glm::linearRand(vec3(0.2f), vec3(1.0f));
		colors.push_back(color);
	}
	/* Generate the transformation matrices */
	//Model
	mat4 I = mat4(1.0f);
	glm::mat4 M;
	M = glm::scale(I, 0.5f * vec3(1.0f));
	M = glm::scale(M, scaleFactor * vec3(1.0f));
	M = glm::translate(M, -meshCenter);
	for (int i = 0; i < MAX_INSTANCES; ++i) {
		mat4 T = glm::scale(I, glm::linearRand(0.5f, 1.0f) * vec3(1.0f));
		T = glm::translate(T, glm::ballRand(3.0f));
		transformations.push_back(T * M);
	}

	/* Send the colors and transformation to GPU*/
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(vec3), colors.data(), GL_STATIC_DRAW);
	//This set this attribute as an instanced attribute
	glVertexAttribDivisor(loc.a_color_loc, 1);

	glBindBuffer(GL_ARRAY_BUFFER, transformation_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, transformations.size() * sizeof(mat4), transformations.data(), GL_STATIC_DRAW);

	//Making instanciated
	for (int i = 0; i < 4; ++i) {
		glVertexAttribDivisor(loc.a_transformation_loc + i, 1);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
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
	ogl::getErrorLog();


	reload_shaders();

	//Activate antialliasing
	glEnable(GL_MULTISAMPLE);

	//Initialize some basic rendering state
	glEnable(GL_DEPTH_TEST);
	//Dark black background color
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

}

void reload_shaders() {
	using std::cerr;
	using std::endl;
	/************************************************************************/
	/*                   OpenGL program creation                            */
	/************************************************************************/
	auto tmpProgram = new OGLProgram("shaders/hm02GeomPass.vert", "shaders/hm02GeomPass.frag");

	if (!tmpProgram || !tmpProgram->isOK()) {
		cerr << "Something wrong in shader" << endl;
		delete tmpProgram;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	}
	else {
		programGeomPassPtr = tmpProgram;
		glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	}

	/* Geometry pass first */
	loc.u_PV_location = programGeomPassPtr->uniformLoc("PV");
	loc.u_Time_location = programGeomPassPtr->uniformLoc("time");
	loc.u_NormMat_location = programGeomPassPtr->uniformLoc("NormMat");
	loc.a_position_loc = programGeomPassPtr->attribLoc("Position");
	loc.a_normal_loc = programGeomPassPtr->attribLoc("Normal");
	loc.a_texture_loc = programGeomPassPtr->attribLoc("TextCoord");
	loc.a_color_loc = programGeomPassPtr->attribLoc("Color");
	loc.a_transformation_loc = programGeomPassPtr->attribLoc("M");
}


void create_glut_callbacks() {
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMouseFunc(mouse);
	glutMouseWheelFunc(mouseWheel);
	glutMotionFunc(mouseDrag);
	glutPassiveMotionFunc(mousePasiveMotion);
}

void reshape(int new_window_width, int new_window_height) {
	glViewport(0, 0, new_window_width, new_window_height);
	cam.setAspectRatio(new_window_width, new_window_height);
	ball.setWindowSize(new_window_width, new_window_height);
}

void display() {
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;
	using namespace std;
	
	geomPass();
	

	//Unbind an clean
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	glutSwapBuffers();
}

void geomPass() {
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	programGeomPassPtr->use();

	//Model
	mat4 M;
	mat4 I = mat4(1.0f);
	M = glm::scale(I, 0.5f * vec3(1.0f));
	M = glm::scale(M, scaleFactor * vec3(1.0f));
	M = glm::translate(M, -meshCenter);
	//View
	mat4 V = cam.getViewMatrix() * ball.getRotation();
	//Projection
	mat4 P = cam.getProjectionMatrix();

	/************************************************************************/
	/* Send uniform values to shader                                        */
	/************************************************************************/

	if (loc.u_PV_location != -1) {
		glUniformMatrix4fv(loc.u_PV_location, 1, GL_FALSE, glm::value_ptr(P * V));
	}
	if (loc.u_NormMat_location != -1) {
		glUniformMatrix4fv(loc.u_NormMat_location, 1, GL_FALSE, glm::value_ptr(glm::inverse(glm::transpose(V * M))));
	}
	if (loc.u_Time_location != -1) {
		glUniform1f(loc.u_Time_location, seconds_elapsed);
	}
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer_id);
	if (loc.a_color_loc != -1) {
		glEnableVertexAttribArray(loc.a_color_loc);
		//Colors in this buffer are thigly packes so the zero at the end
		glVertexAttribPointer(loc.a_color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);
	}
	/* Matrix atribute location actually uses four location one per column
	This code is taken form the OpenGL programming guide pg 130*/
	glBindBuffer(GL_ARRAY_BUFFER, transformation_buffer_id);
	if (loc.a_transformation_loc != -1) {
		for (int i = 0; i < 4; i++) {
			// Set up the vertex attribute
			if (loc.a_transformation_loc + i != -1) {
				// Enable it
				glEnableVertexAttribArray(loc.a_transformation_loc + i);
				glVertexAttribPointer(loc.a_transformation_loc + i, // Location
					4, GL_FLOAT, GL_FALSE, // vec4
					sizeof(mat4), // Stride
					(void *)(sizeof(vec4) * i)); // Start offset
			}
		}
	}

	/* Draw */
	meshPtr->drawTriangles(loc.a_position_loc, loc.a_position_loc, loc.a_position_loc, instace_number);

	if (loc.a_transformation_loc != -1) {
		for (int i = 0; i < 4; i++) {
			if (loc.a_transformation_loc + i != -1) {
				glDisableVertexAttribArray(loc.a_transformation_loc + i);
			}
		}
	}

	if (loc.a_color_loc != -1) {
		glDisableVertexAttribArray(loc.a_color_loc);
	}
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

		case 'i':
		case 'I':
			instanciated = !instanciated;
		break;

		default:
		break;
	}

	glutPostRedisplay();
}

void special(int key, int mouse_x, int mouse_y) {
	
	glutPostRedisplay();
}

void mouse(int button, int state, int mouse_x, int mouse_y) {
	/* Camera trackball*/
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			ball.startDrag(glm::vec2(mouse_x, mouse_y));
		}
		else {
			ball.endDrag(glm::vec2(mouse_x, mouse_y));
		}
	}

	glutPostRedisplay();
}

void mouseWheel(int button, int dir, int mouse_x, int mouse_y) {
	
	/* Camera zoom in-out*/
	const float DELTA_ANGLE = PI / 30.0f;
	if (button == 0) {
		cam.addFovY(dir > 0 ? DELTA_ANGLE : -DELTA_ANGLE);
	}

	glutPostRedisplay();
}

void mouseDrag(int mouse_x, int mouse_y) {
	/*Trackball camera*/
	ball.drag(glm::ivec2(mouse_x, mouse_y));
	glutPostRedisplay();
}

void mousePasiveMotion(int mouse_x, int mouse_y) {
	/*Now, the app*/
	glutPostRedisplay();
}
