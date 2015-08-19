#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>

//This is a bug on GLM in 32 bits I hope they solve it soon
#define GLM_FORCE_PURE
#define GLM_MESSAGES
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

OpenGLProgram* gl_program_ptr = nullptr;

//Program management
void exit_glut();
void init_OpenGL();
void init_program();
void create_glut_window();
void create_glut_callbacks();
void reset_camera();

//Callback function
void display();
void reshape(int new_window_width, int new_window_height);
void mouse (int button, int state, int mouse_x, int mouse_y);
void keyboard(unsigned char key, int mouse_x, int mouse_y);
void special_keyboard(int key, int mouse_x, int mouse_y);
void mouse_active(int mouse_x, int mouse_y);
void mouse(int button, int state, int mouse_x, int mouse_y);
void mouse_wheel(int wheel, int direction, int mouse_x, int mouse_y);
void idle();

//Progran logic handler
GLint u_M_location = -1;
GLint u_V_location = -1;
GLint u_P_location = -1;
GLint u_color_location = -1;

float seconds;
int window_width;
int window_height;
double aspect_ratio;
float world_radious;
float cube_rotation_angle;
float camera_rotation_angle;
bool cube_rotations;

//Camera control
glm::vec3 eye;
glm::vec3 at;
glm::vec3 up;
bool mouse_dragging;
glm::vec2 mouse_start_drag;
glm::vec2 base_rotation_angles;
glm::vec2 new_rotation_angles;
GLdouble scale_perspective;
GLdouble scale_ortho;
bool projection_type;
bool contours;
bool special_camera;
bool orbit_camera;
int grid_size;

//Debug functions
void print_matrix_debug (glm::mat4 A);
void print_matrix_debug (glm::mat3 A);
void print_vector_debug (glm::vec4 u);
void print_vector_debug (glm::vec3 u);

void init_OpenGL() {
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

	gl_program_ptr = new shaders::OpenGLProgram("proy2_vs.glsl", "proy2_fs.glsl");

	if (!gl_program_ptr->is_ok()) {
		cerr << "Error at program creation!" << endl;
		exit(EXIT_FAILURE);
	}

	u_M_location = gl_program_ptr->get_uniform_location("M");
	u_V_location = gl_program_ptr->get_uniform_location("V");
	u_P_location = gl_program_ptr->get_uniform_location("P");
	
	u_color_location = gl_program_ptr->get_uniform_location("Color");
	//Activate antialliasing
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	//initialize some basic rendering state
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	gl_error("At the end of display");
}

void display() {
	gl_program_ptr->use_program();
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//Init all matrices at Identity to be sure
	glm::mat4 M = glm::mat4(1.0);
	glm::mat4 V = glm::mat4(1.0);
	glm::mat4 P = glm::mat4(1.0);
	glm::mat4 I = glm::mat4(1.0);

	//View transformation
	V = glm::rotate(I, base_rotation_angles.y + new_rotation_angles.y, glm::vec3(1.0, 0.0, 0.0));
	V = glm::rotate(V, base_rotation_angles.x + new_rotation_angles.x, glm::vec3(0.0, 1.0, 0.0));
	if (projection_type) {
		eye = glm::vec3(0.0, 0.0, -2.0 * world_radious);
		at = glm::vec3(0.0, 0.0, 0.0);
		up = glm::vec3(0.0, 1.0, 0.0);
		V = glm::lookAt(eye, at, up) * V;
	} else {
		V = glm::scale(V, glm::vec3(scale_ortho, scale_ortho, scale_ortho)) * V;
	}

	if (special_camera) {
		eye = glm::vec3(0.0, 0.0, -1.1 * world_radious);
		at = glm::vec3(0.0, 0.0, 0.0);
		up = glm::vec3(0.0, 1.0, 0.0);
		V = glm::lookAt(eye, at, up);
	}

	if (orbit_camera) {
		glm::vec4 eye_position = glm::rotate(I, camera_rotation_angle, glm::vec3(0.0, 1.0, 0.0)) * glm::vec4(0.0, 0.0, -2.0 * world_radious, 1.0);
		eye = glm::vec3(eye_position);
		at = glm::vec3(0.0, 0.0, 0.0);
		up = glm::vec3(0.0, 1.0, 0.0);
		V = glm::lookAt(eye, at, up);
	}

	glUniformMatrix4fv(u_V_location, 1, GL_FALSE, glm::value_ptr(V));

	//Projection transformation
	if (projection_type) {
		P = glm::perspective(scale_perspective, aspect_ratio, 1.0 * world_radious, 3.0 * world_radious);
	} else {
		double z_distance = scale_ortho > 1.0 ? scale_ortho * world_radious : world_radious;
		double x_distance = window_width >= window_height ? world_radious * aspect_ratio: world_radious;
		//double x_distance = window_width >= window_height ? 1.0 * aspect_ratio: world_radious;
		double y_distance = window_width >= window_height ? world_radious : world_radious /  aspect_ratio;
		//double y_distance = window_width >= window_height ? 1.0 : 1.0 /  aspect_ratio;
		P = glm::ortho(-x_distance, x_distance, -y_distance, y_distance, z_distance, -z_distance);
	}

	if (special_camera) {
		P = glm::perspective(90.0, aspect_ratio, 1.0 * world_radious, 3.0 * world_radious);
	}
	
	if (orbit_camera) {
		P = glm::perspective(40.0, aspect_ratio, 1.0 * world_radious, 3.0 * world_radious);
	}

	glUniformMatrix4fv(u_P_location, 1, GL_FALSE, glm::value_ptr(P));
	
	//const int GRID_SIZE = 3;
	glm::vec3 position = glm::vec3(-1.0, -1.0, -1.0);
	glm::vec4 color = glm::vec4(0.0, 0.0, 0.0, 1.0);
	glm::vec4 contour_color = glm::vec4(0.0, 0.0, 0.0, 1.0);
	glm::vec3 rotation_axis = glm::vec3(1.0, 1.0, 1.0);

	float delta = 2.0f / (grid_size + 1);
	float delta_color = 1.0f / (grid_size + 1);
	for (int k = 0; k < grid_size; k++) {
		position.z += delta;
		color.b += delta_color;
		for (int j = 0; j < grid_size; j++) {
			position.y += delta;
			color.g += delta_color;
			for (int i = 0; i < grid_size; i++) {
				position.x += delta;
				color.r += delta_color;
				M = glm::translate(I, position);
				if (cube_rotations) {
					M = glm::rotate(M, cube_rotation_angle, rotation_axis);
				}
				M = glm::scale(M, 0.25f * glm::vec3(1.0f));
				glUniform4fv(u_color_location, 1, glm::value_ptr(color));
				glUniformMatrix4fv(u_M_location, 1, GL_FALSE, glm::value_ptr(M));
				glutSolidCube(1.0);
				if (contours) {
					glUniform4fv(u_color_location, 1, glm::value_ptr(contour_color));
					glutWireCube(1.005);
				}
			}
			position.x = -1.0;
			color.r = 0.0;
		}
		position.y = -1.0;
		color.g = 0.0;
	}
	/* For symmetry but never really executed
	position.z = -1.0;
	color.b = 0.0;*/

	glutSwapBuffers();
	glUseProgram(0);
	gl_error();
}

int main (int argc, char* argv[]) {

	glutInit(&argc, argv);
	create_glut_window();
	create_glut_callbacks();
	init_program();
	init_OpenGL();	
	glutMainLoop();
	
	return EXIT_SUCCESS;
}

void init_program() {
	seconds = 0.0;
	cube_rotation_angle = 0.0;
	camera_rotation_angle = 0.0;
	window_width = window_height = 512;
	world_radious = sqrtf(3.0);
	grid_size = 3;
	projection_type = false;
	reset_camera();
	contours = true;
	cube_rotations = false;
	special_camera = false;
}

void reset_camera() {
	base_rotation_angles = glm::vec2(0.0, 0.0);
	new_rotation_angles = glm::vec2(0.0, 0.0);
	scale_perspective = 60.0;
	orbit_camera = false;
	scale_ortho = 1.0;
}

void create_glut_window() {
	glutInitDisplayMode (GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize (512, 512);
	win = glutCreateWindow ("Jorge Garcia, Project 2");
}

void create_glut_callbacks() {
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special_keyboard);
	glutMouseWheelFunc(mouse_wheel);
	glutMouseFunc(mouse);
	glutMotionFunc(mouse_active);
	glutReshapeFunc(reshape);
}

void exit_glut() {
	delete gl_program_ptr;
	glutDestroyWindow(win);
	exit(EXIT_SUCCESS);
}

void reshape(int new_window_width, int new_window_height) {
	window_width = new_window_width == 0 ? 1 : new_window_width;
	window_height = new_window_height == 0 ? 1 : new_window_height;
	aspect_ratio = static_cast<double>(window_width) / static_cast<double>(window_height);

	glViewport(0, 0, window_width, window_height);
}

void special_keyboard(int key, int mouse_x, int mouse_y) {
	switch (key) {
		case GLUT_KEY_F1:
			cube_rotations = !cube_rotations;
		break;

		case GLUT_KEY_F2:
			special_camera = !special_camera;
			if (special_camera) {
				orbit_camera = false;
			}
		break;

		case GLUT_KEY_F3:
			orbit_camera = !orbit_camera;
			if (orbit_camera) {
				special_camera = false;
			}
		break;
	}

	glutPostRedisplay();
}

void keyboard(unsigned char key, int mouse_x, int mouse_y) {
	switch (key) {
		case 'q':
		case 'Q':
		case 27:
			exit_glut();
		break;

		case 'p':
		case 'P':
			projection_type = !projection_type;
			reset_camera();
		break;

		case 'R':
		case 'r':
			reset_camera();
		break;

		case 'C':
		case 'c':
			contours = !contours;
		break;

		case '+':
			grid_size++;
		break;

		case '-':
			if (grid_size > 1) {
				grid_size--;
			}
		break;
	} 
	glutPostRedisplay();
}

void idle() {  
   //timers for time-based animation
   static int last_time = 0;
   int time = glutGet(GLUT_ELAPSED_TIME);
   int elapsed = time - last_time;
   float delta_seconds = 0.001f * elapsed;
   last_time = time;

   cube_rotation_angle += 1.0f * delta_seconds;
   camera_rotation_angle += 1.0f * delta_seconds;

   gl_error("At the end of idle");
   glutPostRedisplay();
}

void mouse_active(int mouse_x, int mouse_y) {
	glm::vec2 mouse_current;
	
	mouse_current.x = static_cast<float>(mouse_x);
	mouse_current.y = static_cast<float>(mouse_y);
	
	glm::vec2 deltas = mouse_start_drag - mouse_current;

	new_rotation_angles.x = deltas.x / glutGet(GLUT_WINDOW_WIDTH) * 180;
	new_rotation_angles.y = deltas.y / glutGet(GLUT_WINDOW_HEIGHT) * 180;

	glutPostRedisplay();
}

void mouse_wheel(int wheel, int direction, int mouse_x, int mouse_y) {
	if (projection_type) {
		if (direction > 0 && scale_perspective < 160.0) {
			scale_perspective += 5.0;
		} else if (direction < 0 && scale_perspective > 10.0) {
			scale_perspective -= 5.0;
		}
		//std::cout << "Scale: " << scale_perspective << std::endl;
	} else {
		if (direction > 0 && scale_ortho < 64.0) {
			scale_ortho *= 2.0;
		} else if (direction < 0 && scale_ortho > 0.125) {
			scale_ortho /= 2.0;
		}
		//std::cout << "Scale: " << scale_ortho << std::endl;
	}
	glutPostRedisplay();
}

void mouse(int button, int state, int mouse_x, int mouse_y) {
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
		mouse_dragging = true;
		mouse_start_drag.x = static_cast<float>(mouse_x);
		mouse_start_drag.y = static_cast<float>(mouse_y);
	} else if (state == GLUT_UP && button == GLUT_LEFT_BUTTON) {
		mouse_dragging = false;
		base_rotation_angles += new_rotation_angles;
		new_rotation_angles = glm::vec2(0.0, 0.0);
	}
	glutPostRedisplay();
}

void print_matrix_debug (glm::mat4 A) {
	cout << A[0][0] << "\t" << A[0][1] << "\t" << A[0][2] << "\t" << A[0][3] << endl;
	cout << A[1][0] << "\t" << A[1][1] << "\t" << A[1][2] << "\t" << A[1][3] << endl;
	cout << A[2][0] << "\t" << A[2][1] << "\t" << A[2][2] << "\t" << A[2][3] << endl;
	cout << A[3][0] << "\t" << A[3][1] << "\t" << A[3][2] << "\t" << A[3][3] << endl;
}

void print_matrix_debug (glm::mat3 A) {
	cout << A[0][0] << "\t" << A[0][1] << "\t" << A[0][2] << endl;
	cout << A[1][0] << "\t" << A[1][1] << "\t" << A[1][2] << endl;
	cout << A[2][0] << "\t" << A[2][1] << "\t" << A[2][2] << endl;
}

void print_vector_debug (glm::vec3 u) {
	cout << "(" << u.x << ", " << u.y << ", " << u.z << ")" << endl;
}

void print_vector_debug (glm::vec4 u) {
	cout << "(" << u.x << ", " << u.y << ", " << u.z << ", " << u.w << ")" << endl;
}