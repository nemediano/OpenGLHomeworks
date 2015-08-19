#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>

//This is a bug on GLM in 32 bits I hope they solve it soon
#define GLM_FORCE_PURE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include <cstdlib>
#include <cmath>
#include <iostream>

#include "OpenGLProgram.h"

using namespace std;
using namespace shaders;

static int win = 0;

OpenGLProgram* pProgram; 
GLint M1_loc = -1;
GLint M2_loc = -1;
GLint M3_loc = -1;
GLint vColor_loc = -1;
float angle = 0.0f;

//Program specific state variables
int polygon_mode;
int shape_type;
float point_size;
float line_width;
int grid_resolution_x, grid_resolution_y;
bool pause;
void draw_shape();

// Menu related globals
int polygon_mode_menu;
int point_size_menu;
int line_width_menu;
int shape_type_menu;
int grid_resolution_menu;
int col_number_menu;
int row_number_menu;
void main_menu_handling (int id);
void polygon_mode_menu_handling (int id);
void point_size_menu_handling (int id);
void line_width_handling (int id);
void shape_type_handling (int id);
void grid_resolution_handling (int id);
void rows_number_handling (int id);
void cols_number_handling (int id);

void create_menus();

void exitGlut() {
   glutDestroyWindow(win);
   exit(0);
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
	cout << "Using freeglut version: " << ver / 10000 << "." << (ver / 100) % 100 << "." << ver % 100 << endl;

	pProgram = new OpenGLProgram("vertexShader.glsl", "fragmentShader.glsl");

	if (!pProgram->is_ok()) {
		cerr << "Error at GL program creation" << endl;
		gl_error();
		exit(EXIT_FAILURE);
	}

	M1_loc = pProgram->get_uniform_location("M1");
	M2_loc = pProgram->get_uniform_location("M2");
	M3_loc = pProgram->get_uniform_location("M3");
	vColor_loc = pProgram->get_uniform_location("vColor");

	//Initializing some basic rendering state
	polygon_mode = GL_FILL;
	shape_type = 2;
	line_width = 1.0f;
	point_size = 2.0f;
	grid_resolution_x = grid_resolution_y = 9;

	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

	glPointSize(point_size);
	glLineWidth(line_width);

	glEnable(GL_DEPTH_TEST);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 S(1.0f);
	glm::mat4 T(1.0f);
	glm::mat4 R(1.0f);
	glm::mat4 I(1.0f);

	pProgram->use_program();

	//Rotating shape
	glUniform4f(vColor_loc, 1.0f, 0.0f, 0.0f, 1.0f);
	S = glm::scale(I, glm::vec3(0.125f, 0.125f, 0.125f));
	glUniformMatrix4fv(M1_loc, 1, GL_FALSE, glm::value_ptr(S));

	T = glm::translate(I, glm::vec3(0.5f, 0.5f, 0.0f));

	R = glm::rotate(I, angle, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(M2_loc, 1, GL_FALSE, glm::value_ptr(R));
	glUniformMatrix4fv(M3_loc, 1, GL_FALSE, glm::value_ptr(T));
	draw_shape();

	//The grid of shapes
	{
		float scale_x, scale_y, scale_z;
		float translate_x, translate_y, translate_z;
		//Set the invariants
		scale_x = scale_y = scale_z = 0.05f;
		translate_z = 0.0f;
		//Setting the grid parameters
		float start_x = -1.0f, end_x = 0.95f, start_y = 0.95f, end_y = -1.0f;
		float dx = (end_x - start_x) / (grid_resolution_x - 1);
		float dy = (end_y - start_y) / (grid_resolution_y - 1);

		//Draw the grid of shapes
		translate_x = start_x;
		for (int i = 0; i < grid_resolution_x; i++) {
			translate_y = start_y;
			for (int j = 0; j < grid_resolution_y; j++) {
				if (i == grid_resolution_x / 2 && j == grid_resolution_y / 2) {
					glUniform4f(vColor_loc, 0.0f, 0.0f, 1.0f, 1.0f);
				}
				else {
					glUniform4f(vColor_loc, 0.0f, 1.0f, 0.0f, 1.0f);
				}
				S = glm::scale(I, glm::vec3(scale_x, scale_y, scale_z));
				T = glm::translate(I, glm::vec3(translate_x, translate_y, translate_z));

				glUniformMatrix4fv(M1_loc, 1, GL_FALSE, glm::value_ptr(S));
				glUniformMatrix4fv(M2_loc, 1, GL_FALSE, glm::value_ptr(T));
				glUniformMatrix4fv(M3_loc, 1, GL_FALSE, glm::value_ptr(I));
				draw_shape();
				translate_y += dy;
			}
			translate_x += dx;
		}
	}

	//Orbiting shape
	glUniform4f(vColor_loc, 1.0f, 0.0f, 0.0f, 1.0f);
	{
		float scale_x, scale_y, scale_z;
		scale_x = scale_y = scale_z = 0.125f;
		S = glm::scale(I, glm::vec3(scale_x, scale_y, scale_z));
		glUniformMatrix4fv(M1_loc, 1, GL_FALSE, glm::value_ptr(S));
		float translate_x, translate_y, translate_z;
		translate_x = 0.5f;	 translate_y = 0.0f; translate_z = 0.0f;
		glUniformMatrix4fv(M2_loc, 1, GL_FALSE, glm::value_ptr(T));
		glUniformMatrix4fv(M3_loc, 1, GL_FALSE, glm::value_ptr(R));
		draw_shape();
	}
	gl_error("After rendering");
	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
   switch (key) {
	case 'p':
	case 'P':
	   pause = !pause;
	break;

	case 27:
		exitGlut();
	break;
   }
   
   glutPostRedisplay();
}


void idle() {  
   //timers for time-based animation
   if (!pause) {
	   static int last_time = 0;
	   int time = glutGet(GLUT_ELAPSED_TIME);
	   int elapsed = time - last_time;
	   float delta_seconds = 0.001f * elapsed;
	   last_time = time;

	   angle = angle + 1.57f * delta_seconds;
   }
   gl_error("After idle function"); //check for errors and print error strings
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
	//Creating the menus
	
	polygon_mode_menu = glutCreateMenu(polygon_mode_menu_handling);
	glutAddMenuEntry("Points", 1);
	glutAddMenuEntry("Lines", 2);
	glutAddMenuEntry("Fill", 3);
	
	point_size_menu = glutCreateMenu(point_size_menu_handling);
	glutAddMenuEntry("Small points", 1);
	glutAddMenuEntry("Big points", 2);

	line_width_menu = glutCreateMenu(line_width_handling);
	glutAddMenuEntry("Thin lines", 1);
	glutAddMenuEntry("Thick lines", 2);

	shape_type_menu = glutCreateMenu(shape_type_handling);
	glutAddMenuEntry("Teapot", 1);
	glutAddMenuEntry("Cube", 2);
	glutAddMenuEntry("Sphere", 3);
	glutAddMenuEntry("Cone", 4);
	glutAddMenuEntry("Torus", 5);
	glutAddMenuEntry("Tetrahedra", 6);
	glutAddMenuEntry("Hexahedra", 42); //Hexahedra is just a cube!!
	glutAddMenuEntry("Octahedra", 7);
	glutAddMenuEntry("Dedecahedra", 8);
	glutAddMenuEntry("Icosahedra", 9);

	row_number_menu = glutCreateMenu(rows_number_handling);
	glutAddMenuEntry("5", 1);
	glutAddMenuEntry("9", 2);
	glutAddMenuEntry("11", 3);

	col_number_menu = glutCreateMenu(cols_number_handling);
	glutAddMenuEntry("5", 1);
	glutAddMenuEntry("9", 2);
	glutAddMenuEntry("11", 3);

	grid_resolution_menu = glutCreateMenu(grid_resolution_handling);
	glutAddSubMenu("Number of rows:", row_number_menu);
	glutAddSubMenu("Number of cols:", col_number_menu);
	
	glutCreateMenu(main_menu_handling);
	glutAddSubMenu("Polygon mode", polygon_mode_menu);
	glutAddSubMenu("Point size", point_size_menu);
	glutAddSubMenu("Line width", line_width_menu);
	glutAddSubMenu("Choose shape", shape_type_menu);
	glutAddSubMenu("Grid resolution", grid_resolution_menu);
	glutAddMenuEntry("Quit", 42);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//Function declaration
void main_menu_handling (int id) {
	switch (id) {
		case 42:
			exitGlut();
		break;

		default:
			cerr << "Case not handled!" << endl;
		break;
	}
}

void polygon_mode_menu_handling (int id) {
	switch (id) {
		case 1:
			polygon_mode = GL_POINT;
		break;

		case 2:
			polygon_mode = GL_LINE;
		break;

		case 3:
			polygon_mode = GL_FILL;
		break;

		default:
			cerr << "Case not handled!" << endl;
		break;
	}
	glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);
	glutPostRedisplay();
}

void point_size_menu_handling (int id) {
	switch (id) {
		case 1:
			point_size = 2.0f;
		break;

		case 2:
			point_size = 4.0f;
		break;

		default:
			cerr << "Case not handled!" << endl;
		break;
	}
	
	if (polygon_mode != GL_POINT) {
		cout << "Polygon mode must be points for this option to take effect!" << endl;
	}

	glPointSize(point_size);
	glutPostRedisplay();

}

void line_width_handling (int id) {
	switch (id) {
		case 1:
			line_width = 1.0;
		break;

		case 2:
			line_width = 3.0;
		break;

		default:
			cerr << "Case not handled!" << endl;
		break;
	}

	if (polygon_mode != GL_LINE) {
		cout << "Polygon mode must be lines for this option to take effect!" << endl;
	}

	glLineWidth(line_width);
	glutPostRedisplay();
}

void shape_type_handling (int id) {
	switch (id) {
		case 42:
			cout << "Hexahedra is just a cube!!" << endl;
			shape_type = 2;
		break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			shape_type = id;
		break;

		default:
			std::cerr << "Case not handled!" << endl;
		break;
	}

	if (shape_type < 1 || shape_type > 9) {
		cerr << "Invalid shape type!" << endl;
	}
	glutPostRedisplay();
}

void cols_number_handling(int id) {
	switch(id) {
		case 1:
			grid_resolution_x = 5;
		break;

		case 2:
			grid_resolution_x = 9;
		break;

		case 3:
			grid_resolution_x = 11;
		break;

		default:
			std::cerr << "Case not handled!" << endl;
		break;
	}
	glutPostRedisplay();
}

void rows_number_handling(int id) {
	switch(id) {
		case 1:
			grid_resolution_y = 5;
		break;

		case 2:
			grid_resolution_y = 9;
		break;

		case 3:
			grid_resolution_y = 11;
		break;

		default:
			std::cerr << "Case not handled!" << endl;
		break;
	}
	glutPostRedisplay();
}

void grid_resolution_handling (int id) {
	
	switch (id) {
		case 1:
			grid_resolution_x = 5;
		break;

		case 2:
			grid_resolution_x = 7;
		break;

		case 3:
			grid_resolution_x = 9;
		break;

		case 4:
			grid_resolution_y = 5;
		break;

		case 5:
			grid_resolution_y = 7;
		break;

		case 6:
			grid_resolution_y = 9;
		break;

		default:
			std::cerr << "Case not handled!" << endl;
		break;
	}

	glutPostRedisplay();
}

void draw_shape() {
	switch (shape_type) {
		case 1:
			glutSolidTeapot(1.0);
		break;

		case 2:
			glutSolidCube(1.0);
		break;

		case 3:
			glutSolidSphere(0.5, 10, 10);
		break;

		case 4:
			glutSolidCone(1.0, 1.0, 10, 10);
		break;

		case 5:
			glutSolidTorus(0.25, 1.0, 10, 10);
		break;

		case 6:
			glutSolidTetrahedron();
		break;

		case 7:
			glutSolidOctahedron();
		break;

		case 8:
			glutSolidDodecahedron();
		break;

		case 9:
			glutSolidIcosahedron();
		break;
	}
	glutPostRedisplay();
}
