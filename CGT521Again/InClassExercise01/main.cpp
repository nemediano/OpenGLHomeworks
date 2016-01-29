#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cstdlib>
#include <iostream>

#include <IL/il.h>
#include <IL/ilu.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "opengl/HelperFunctions.h"
#include "opengl/OpenGLProgram.h"

opengl::OpenGLProgram* program_ptr = nullptr;

using namespace std;
//Glut window pointer
int window = 0;

//Variables for GPU side
GLint a_position_loc = -1;
GLint a_textCoord_loc = -1;
GLint u_PVM_loc = -1;

//Manage the Vertex Buffer Object
GLuint vbo;
GLuint indexBuffer;

//Two math constants (New glm uses radians as default)
const float TAU = 6.28318f;
const float PI = 3.14159f;
int nTriangles;

//Program management
void exit_glut();
void init_OpenGL();
void init_program();
void create_glut_window();
void create_glut_callbacks();
void reload_shaders();

//Scene creation
void create_primitives();

// Define a helpful macro for handling offsets into buffer objects
#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))
#define OFFSET_OF(type, member) ((GLvoid*)(offsetof(type, member)))

struct Vertex {
	glm::vec2 position;
};

//Callback function
void display();
void reshape(int new_window_width, int new_window_height);
void keyboard(unsigned char key, int mouse_x, int mouse_y);
void idle();

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
	delete program_ptr;
	glutDestroyWindow(window);
	exit(EXIT_SUCCESS);
}

void init_OpenGL() {
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
	}
	opengl::get_OpenGL_info();
	opengl::get_error_log();
	reload_shaders();
	
	//Activate antialliasing
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	//initialize some basic rendering state
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	opengl::gl_error("At scene creation");
}

void create_glut_window() {
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	window = glutCreateWindow("Lab 01: Draw using fragment shader");
}

void create_glut_callbacks() {
	glutDisplayFunc(display);
	//glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
}

void reshape(int new_window_width, int new_window_height) {
	glViewport(0, 0, new_window_width, new_window_height);
}

void idle() {
	//timers for time-based animation
	static int last_time = 0;
	int time = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = time - last_time;
	float delta_seconds = 0.001f * elapsed;
	last_time = time;

	

	opengl::gl_error("At idle"); //check for errors and print error strings
	glutPostRedisplay();
}

void init_program() {
	create_primitives();
}


void display() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	program_ptr->use_program();
	glm::mat4 I(1.0f);

	//Model
	glm::mat4 M = I;

	//View
	/* Camera rotation must be accumulated: base rotation then new rotation */
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::mat4 V = glm::lookAt(position, center, camera_up);
	

	//Projection
	const GLfloat xLeft = -1.0f;
	const GLfloat xRight = 1.0f;
	const GLfloat yBottom = -1.0f;
	const GLfloat yTop = 1.0f;
	const GLfloat zNear = -1.0;
	const GLfloat zFar = 1.0;
	glm::mat4 P = glm::ortho(xLeft, xRight, yBottom, yTop, zNear, zFar);
	
	//This is really fishy, I should ask someone
	glm::mat4 PVM = glm::transpose(P * V * M);

	if (u_PVM_loc != -1) {
		glUniformMatrix4fv(u_PVM_loc, 1, GL_FALSE, glm::value_ptr(PVM));
	}

	/* Bind */
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	if (a_position_loc != -1) {
		glEnableVertexAttribArray(a_position_loc);
		glVertexAttribPointer(a_position_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSET_OF(Vertex, position));
	}
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	
	/* Draw */
	glDrawElements(GL_TRIANGLES, 3 * nTriangles, GL_UNSIGNED_SHORT, BUFFER_OFFSET(3 * 0 * sizeof(unsigned short)));
	

	/* Unbind and clean */
	if (a_position_loc != -1) {
		glDisableVertexAttribArray(a_position_loc);
	}
	

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);

	glutSwapBuffers();
	//opengl::gl_error("At the end of display");
}


void create_primitives() {
	const unsigned int nVertex = 4;
	const unsigned int nIndices = 6;
	nTriangles = 2;

	Vertex points[nVertex] = {
		{ { -1.0f, -1.0f } }, { { 1.0f, -1.0f } }, { { 1.0f, 1.0f } }, { { -1.0f, 1.0f } },
	};

	unsigned short indices[nIndices] = { 
		0, 1, 2, 0, 2, 3,
	};

	//Create the buffers
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &indexBuffer);

	//Send data to GPU
	//First send the vertices
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, nVertex * sizeof(Vertex), points, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//Now, the indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, nIndices * sizeof(unsigned short), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void keyboard(unsigned char key, int mouse_x, int mouse_y) {
	if (key == 27) {//press ESC to exit
		exit_glut();
	}
	else if (key == 'r' || key == 'R') {
		reload_shaders();
	}
	glutPostRedisplay();
}

void reload_shaders() {
	program_ptr = new opengl::OpenGLProgram("shaders/vertexShader.glsl", "shaders/fragmentShader.glsl");

	if (!program_ptr->is_ok()) {
		cerr << "Error at GL program creation" << endl;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
		opengl::gl_error();
	} else {
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	}

	//Create attribute for the location
	a_position_loc = program_ptr->get_attrib_location("Position");

	//Ask for the uniform location
	u_PVM_loc = program_ptr->get_uniform_location("PVM");

	return;
}