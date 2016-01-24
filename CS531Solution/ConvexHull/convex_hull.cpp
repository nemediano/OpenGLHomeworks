#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cstdlib>
#include <iostream>
#include <vector>

#define GLM_FORCE_PURE
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
GLint u_PVM_location = -1;

GLint a_position_loc = -1;
GLint a_color_loc = -1;


//Manage the Vertex Buffer Objects
GLuint vbo;
GLuint indexBufferPoints;
GLuint indexBufferContours;
GLuint indexBufferPolygons;

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

//Scene creation
void create_primitives();

// Define a helpful macro for handling offsets into buffer objects
#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))
#define OFFSET_OF(type, member) ((GLvoid*)(offsetof(type, member)))

struct Vertex {
	glm::vec2 position;
	glm::vec3 color;
};

//Program logic
vector<Vertex> vertices;
vector<unsigned short> indices_points;
vector<unsigned short> indices_contours;
vector<unsigned short> indices_polygons;

//Callback function
void display();
void reshape(int new_window_width, int new_window_height);
void keyboard(unsigned char key, int mouse_x, int mouse_y);
//void special_keyboard(int key, int mouse_x, int mouse_y);
void mouse_active(int mouse_x, int mouse_y);
void mouse(int button, int state, int mouse_x, int mouse_y);
void mouse_wheel(int wheel, int direction, int mouse_x, int mouse_y);
void idle();

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);

	create_glut_window();
	init_program();
	init_OpenGL();
	//Create scene
	create_primitives();

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

	program_ptr = new opengl::OpenGLProgram("shaders/vertexShader.glsl", "shaders/fragmentShader.glsl");
	
	if (!program_ptr->is_ok()) {
		cerr << "Error at GL program creation" << endl;
		opengl::gl_error();
		exit(EXIT_FAILURE);
	}

	opengl::get_error_log();
	
	u_PVM_location = program_ptr->get_uniform_location("PVM");
	
	a_position_loc = program_ptr->get_attrib_location("Position");
	a_color_loc = program_ptr->get_attrib_location("Color");
	
	//Activate antialliasing
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	//initialize some basic rendering state
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

	//Make the point a little bigger
	glEnable(GL_PROGRAM_POINT_SIZE);//Use shader
	//glPointSize(4.0);

	opengl::gl_error("At scene creation");
}

void create_glut_window() {
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	window = glutCreateWindow("Convex Hull");
}

void create_glut_callbacks() {
	glutDisplayFunc(display);
	//glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutMouseWheelFunc(mouse_wheel);
	/*
	glutSpecialFunc(special_keyboard);
	*/
	glutMouseFunc(mouse);
	glutMotionFunc(mouse_active);
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

}


void display() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	program_ptr->use_program();
	glm::mat4 I(1.0f);
	
	//Model
	glm::mat4 M = I;
	
	//View
	glm::vec3 position = glm::vec3(0.0, 0.0, 1.0f);
	glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::mat4 V = glm::lookAt(position, center, camera_up);

	//Projection
	GLfloat xLeft = -1.0f;
	GLfloat xRight = 1.0f;
	GLfloat yBottom = -1.0f;
	GLfloat yTop = 1.0f;
	GLfloat zNear = -1.0f;
	GLfloat zFar = 1.0f;
	glm::mat4 P = glm::ortho(xLeft, xRight, yBottom, yTop, zNear, zFar);


	if (u_PVM_location != -1) {
		glUniformMatrix4fv(u_PVM_location, 1, GL_FALSE, glm::value_ptr(I));
	}
	
	/* Bind */
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	if (a_position_loc != -1) {
		glEnableVertexAttribArray(a_position_loc);
		glVertexAttribPointer(a_position_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSET_OF(Vertex, position));
	}
	if (a_color_loc != -1) {
		glEnableVertexAttribArray(a_color_loc);
		glVertexAttribPointer(a_color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSET_OF(Vertex, color));
	}
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferPoints);
	
	
	/* Draw */
	glDrawElements(GL_POINTS, 3, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
		
	/* Unbind and clean */
	if (a_position_loc != -1) {
		glDisableVertexAttribArray(a_position_loc);
	}
	if (a_color_loc != -1) {
		glDisableVertexAttribArray(a_color_loc);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
	
	glutSwapBuffers();
	opengl::gl_error("At the end of display");
}


void create_primitives() {
	//const unsigned int nVertex = 3;
	//const unsigned int nIndices = 3;
	nTriangles = 1;

	//Vertex points[nVertex] = {
	//	{ {-0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, }, //0
	//	{ { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, }, //1
	//	{ { 0.0f,  0.5f }, { 0.0f, 0.0f, 1.0f }, }, //2
	//};

	//unsigned short indices[nIndices] = { 
	//	                                 0, 1, 2,
	//								    };
	GLuint tmp[4] = {0};
	//Create the buffers
	glGenBuffers(4, tmp);
	vbo = tmp[0];
	indexBufferPoints = tmp[1];
	indexBufferContours = tmp[2];
	indexBufferPolygons = tmp[3];

	indices_points.push_back(0);
	indices_points.push_back(1);
	indices_points.push_back(2);

	glm::vec3 colors[] = { 
		                   glm::vec3(1.0f, 0.0f, 0.0f),
						   glm::vec3(0.0f, 1.0f, 0.0f),
						   glm::vec3(0.0f, 0.0f, 1.0f), 
	                     };
	
	Vertex tmpVertex;
	float delta_angle = TAU / 3.0f;
	float angle = TAU / 4;
	glm::vec2 p;
	float radius = 0.5f;
	for (auto i = 0; i < 3; ++i) {
		p.x = radius * cos(angle);
		p.y = radius * sin(angle);
		angle += delta_angle;
		tmpVertex.position = p;
		tmpVertex.color = colors[i % 3];
		vertices.push_back(tmpVertex);
	}


	//Send data to GPU
	//First send the vertices
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//Now, the indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferPoints);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_points.size() * sizeof (unsigned short), indices_points.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void keyboard(unsigned char key, int mouse_x, int mouse_y) {
	if (key == 27) {//press ESC to exit
		exit_glut();
	} else if (key == 'c' || key == 'C') {
		
	}
}

void mouse_wheel(int wheel, int direction, int mouse_x, int mouse_y) {
	
	if (wheel == 0) {
		if (direction > 0) {
			
		} else {
			
		}
	}
}

void mouse_active(int mouse_x, int mouse_y) {
	
	glutPostRedisplay();
}

void mouse(int button, int state, int mouse_x, int mouse_y) {

	if (state == GLUT_DOWN) {
		
	} else {
		
	}
	glutPostRedisplay();
}
