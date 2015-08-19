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

#include <fstream> //For text files outputs
#include <vector>

// Define a helpful macro for handling offsets into buffer objects
#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

#include "OpenGLProgram.h"
#include "GraphPoint.h"

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
//Evaluates the mesh function at index i, j
glm::vec3 surface(int i, int j);
glm::vec3 normal(int i, int j);
//Create mesh of this size on GPU
void populate_indexed_vbo();
void populate_unindexed_vbo();
//Draw current mesh
void draw_indexed_vbo();
void draw_unindexed_vbo();
//Add vertex
void add_vertex();
void remove_vertex();
void delete_vbo();
//Record experiment
void save_experiment();
void change_size(int id);

//Program logic globals
unsigned int vertex_grid_side;
glm::vec3 light_direction;
float light_angle;

//For recording experiment
std::vector<GraphPoint> points;

bool use_index;
bool use_normal_color;
bool use_triangle_strip;
bool rotate_light;
bool use_gouraud;
bool wireframe_mode;
GLint change_size_menu;


GLuint VBO_indexed_id;
GLuint index_buffer_id;
GLuint VBO_unidexed_id;

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
void make_experiment();

void create_menus();

//Progran logic handler
GLint u_M_location = -1;
GLint u_V_location = -1;
GLint u_P_location = -1;
GLint u_color_location = -1;
GLint u_normal_color_opt_loc = -1;
GLint u_light_direction_loc = -1;

GLint a_position_location = -1;
GLint a_normal_location = -1;

float seconds;
int window_width;
int window_height;
double aspect_ratio;
float world_radious;

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

	gl_program_ptr = new OpenGLProgram("proy3_vs.glsl", "proy3_fs.glsl");
	if (!gl_program_ptr->is_ok()) {
		cerr << "Something wrong at the program creation!!" << endl;
		exit(EXIT_FAILURE);
	}

	u_M_location = gl_program_ptr->get_uniform_location("M");
	u_V_location = gl_program_ptr->get_uniform_location("V");
	u_P_location = gl_program_ptr->get_uniform_location("P");
	
	u_color_location = gl_program_ptr->get_uniform_location("Color");
	u_normal_color_opt_loc = gl_program_ptr->get_uniform_location("normColorOpt");
	u_light_direction_loc = gl_program_ptr->get_uniform_location("lightDirection");

	a_position_location = gl_program_ptr->get_attrib_location("aPosition");
	a_normal_location = gl_program_ptr->get_attrib_location("aNormal");
	
	//Create VBOS
	glGenBuffers(1, &VBO_indexed_id);
	glGenBuffers(1, &index_buffer_id);
	glGenBuffers(1, &VBO_unidexed_id);

	populate_indexed_vbo();
	populate_unindexed_vbo();
	
	//Activate antialliasing
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	//initialize some basic rendering state
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	gl_error("At initialization of OpenGL");
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
	M = glm::rotate(I, base_rotation_angles.y + new_rotation_angles.y, glm::vec3(1.0, 0.0, 0.0));
	M = glm::rotate(M, base_rotation_angles.x + new_rotation_angles.x, glm::vec3(0.0, 1.0, 0.0));
	
	glUniformMatrix4fv(u_M_location, 1, GL_FALSE, glm::value_ptr(M));
	if (projection_type) {
		eye = glm::vec3(0.0, 0.0, -2.0 * world_radious);
		at = glm::vec3(0.0, 0.0, 0.0);
		up = glm::vec3(0.0, 1.0, 0.0);
		V = glm::lookAt(eye, at, up);
	} else {
		V = glm::scale(V, glm::vec3(scale_ortho, scale_ortho, scale_ortho));
	}

	glUniformMatrix4fv(u_V_location, 1, GL_FALSE, glm::value_ptr(V));

	//Projection transformation
	if (projection_type) {
		P = glm::perspective(scale_perspective, aspect_ratio, 1.0 * world_radious, 3.0 * world_radious);
	} else {
		double z_distance = scale_ortho > 1.0 ? scale_ortho * world_radious : world_radious;
		double x_distance = window_width >= window_height ? world_radious * aspect_ratio: world_radious;
		double y_distance = window_width >= window_height ? world_radious : world_radious /  aspect_ratio;
		P = glm::ortho(-x_distance, x_distance, -y_distance, y_distance, z_distance, -z_distance);
	}

	glUniformMatrix4fv(u_P_location, 1, GL_FALSE, glm::value_ptr(P));
	
	glUniform1i(u_normal_color_opt_loc, use_normal_color ? 1 : 0);

	glm::vec4 color = glm::vec4(0.0, 0.6, 0.0, 1.0);
	glUniform4fv(u_color_location, 1, glm::value_ptr(color));

	//Pass light direction
	if (rotate_light) {
		glUniform4fv(u_light_direction_loc, 1, glm::value_ptr(glm::rotate(I, light_angle, glm::vec3(0.0, 1.0, 0.0)) * glm::vec4(light_direction, 0.0)));
		//std::cout << "Light angle: " << light_angle << std::endl;
	} else {
		glUniform4fv(u_light_direction_loc, 1, glm::value_ptr(light_direction));
	}

	glFinish(); //blocks until previous gl calls have completed
	GLint64 start;
	glGetInteger64v(GL_TIMESTAMP, &start);

	if (use_index) {
		draw_indexed_vbo();
	} else {
		draw_unindexed_vbo();
	}
	
	glFinish(); //blocks until previous gl calls have completed
	GLint64 finish;
	glGetInteger64v(GL_TIMESTAMP, &finish);

	//cout<< "Draw calls took " << (finish-start)/1000000.0 << "ms" << endl;
	points.push_back(GraphPoint((vertex_grid_side - 1) * (vertex_grid_side - 1) * 2, use_index, (finish-start)/1000000.0));
	
	glutSwapBuffers();
	
	glUseProgram(0);
	gl_error("At display");
}

void populate_indexed_vbo() {
	//Buffer vertices
	int nFloats = vertex_grid_side * vertex_grid_side * 3 * 2;
	int datasize = nFloats * sizeof(float);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_indexed_id);
	glBufferData(GL_ARRAY_BUFFER, datasize, nullptr, GL_STATIC_DRAW);

	float* data = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	int index = 0;
	
	for (unsigned int i = 0; i < vertex_grid_side; ++i) {
		for (unsigned int j = 0; j < vertex_grid_side; ++j) {
			glm::vec3 p1 = surface(i, j);
			data[index++] = p1.x;
			data[index++] = p1.y;
			data[index++] = p1.z;

			glm::vec3 n1 = normal(i, j);
			data[index++] = n1.x;
			data[index++] = n1.y;
			data[index++] = n1.z;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Buffer indices
	int nIndices = (vertex_grid_side - 1) * (vertex_grid_side - 1) * 6;
	int indexsize = nIndices * sizeof(unsigned short);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexsize, nullptr, GL_STATIC_DRAW);

	unsigned short* indices = (unsigned short*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	index = 0;
	for (unsigned int i = 0; i < (vertex_grid_side - 1); ++i) {
		for (unsigned int j = 0; j < (vertex_grid_side - 1); ++j) {
			indices[index++] = vertex_grid_side * j + i;
			indices[index++] = vertex_grid_side * (j + 1) + i;
			indices[index++] = vertex_grid_side * j + (i + 1);

			indices[index++] = vertex_grid_side * (j + 1) + i;
			indices[index++] = vertex_grid_side * j + (i + 1);
			indices[index++] = vertex_grid_side * (j + 1) + (i + 1);	
		}
	}
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void populate_unindexed_vbo() {
	int nFloats = (vertex_grid_side - 1) * (vertex_grid_side - 1) * 6 * 3 * 2;
	//int nFloats = vertex_grid_side * vertex_grid_side * 6 * 3 * 2;
	int datasize = nFloats * sizeof(float);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_unidexed_id);
	glBufferData(GL_ARRAY_BUFFER, datasize, nullptr, GL_STATIC_DRAW);

	float* data = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	int index = 0;

	for (unsigned int i = 0; i < (vertex_grid_side - 1); ++i) {
		for (unsigned int j = 0; j < (vertex_grid_side - 1); ++j) {
			//triangle 1
			glm::vec3 p1 = surface(i, j);
			data[index++] = p1.x;
			data[index++] = p1.y;
			data[index++] = p1.z;

			glm::vec3 n1 = normal(i, j);
			data[index++] = n1.x;
			data[index++] = n1.y;
			data[index++] = n1.z;

			glm::vec3 p2 = surface(i, j + 1);
			data[index++] = p2.x;
			data[index++] = p2.y;
			data[index++] = p2.z;

			glm::vec3 n2 = normal(i, j);
			data[index++] = n2.x;
			data[index++] = n2.y;
			data[index++] = n2.z;

			glm::vec3 p3 = surface(i + 1, j);
			data[index++] = p3.x;
			data[index++] = p3.y;
			data[index++] = p3.z;

			glm::vec3 n3 = normal(i, j);
			data[index++] = n3.x;
			data[index++] = n3.y;
			data[index++] = n3.z;
			
			//triangle 2
			data[index++] = p2.x;
			data[index++] = p2.y;
			data[index++] = p2.z;

			data[index++] = n2.x;
			data[index++] = n2.y;
			data[index++] = n2.z;


			data[index++] = p3.x;
			data[index++] = p3.y;
			data[index++] = p3.z;

			data[index++] = n3.x;
			data[index++] = n3.y;
			data[index++] = n3.z;

			glm::vec3 p4 = surface(i + 1, j + 1);
			data[index++] = p4.x;
			data[index++] = p4.y;
			data[index++] = p4.z;

			glm::vec3 n4 = normal(i, j);
			data[index++] = n4.x;
			data[index++] = n4.y;
			data[index++] = n4.z;
		}
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw_indexed_vbo() {
	glBindBuffer(GL_ARRAY_BUFFER, VBO_indexed_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
	int stride = (3 + 3) * sizeof(float);
	glEnableVertexAttribArray(a_position_location);
	glVertexAttribPointer(a_position_location, 3, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(a_normal_location);
	glVertexAttribPointer(a_normal_location, 3, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(3 * sizeof(float)));
	glDrawElements(GL_TRIANGLES, (vertex_grid_side - 1) * (vertex_grid_side - 1) * 6, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
	glDisableVertexAttribArray(a_position_location);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw_unindexed_vbo() {
	glBindBuffer(GL_ARRAY_BUFFER, VBO_unidexed_id);
	int stride = (3 + 3) * sizeof(float);
	glEnableVertexAttribArray(a_position_location);
	glVertexAttribPointer(a_position_location, 3, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(a_normal_location);
	glVertexAttribPointer(a_normal_location, 3, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(3 * sizeof(float)));
	glDrawArrays(GL_TRIANGLES, 0, (vertex_grid_side - 1) * (vertex_grid_side - 1) * 6);
	glDisableVertexAttribArray(a_position_location);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

glm::vec3 surface(int i, int j) {
	const float center = 0.5f * (vertex_grid_side - 1);
	const float xy_scale = 20.0f / (vertex_grid_side - 1);
	const float z_scale = 10.0f;

	float x = xy_scale * (i - center);
	float y = xy_scale * (j - center);

	float r = sqrt(x * x + y * y);
	float z = 1.0f;

	if(r != 0.0f)
	{
		z = sin(r) / r;
	}
	z = z * z_scale;

	return glm::vec3(x, y, z);
}

glm::vec3 normal(int i, int j) {
	glm::vec3 dFdu = surface(i+1, j) - surface(i-1, j);	
	glm::vec3 dFdv = surface(i, j+1) - surface(i, j-1);	
	glm::vec3 n = glm::cross(dFdu, dFdv);	
  
	return glm::normalize(n);
}

void add_vertex() {
	if (vertex_grid_side == 10) {
		vertex_grid_side = 20;
	} else if (vertex_grid_side == 20) {
		vertex_grid_side = 50;
	} else if (vertex_grid_side == 50) {
		vertex_grid_side = 100;
	} else if (vertex_grid_side == 100) {
		vertex_grid_side = 150;
	}
}

void remove_vertex() {
	if (vertex_grid_side == 150) {
		vertex_grid_side = 100;
	} else if (vertex_grid_side == 100) {
		vertex_grid_side = 50;
	} else if (vertex_grid_side == 50) {
		vertex_grid_side = 20;
	} else if (vertex_grid_side == 20) {
		vertex_grid_side = 10;
	}
}

int main (int argc, char* argv[]) {

	glutInit(&argc, argv);
	create_glut_window();
	create_glut_callbacks();
	create_menus();
	init_program();
	init_OpenGL();	
	glutMainLoop();
	
	return EXIT_SUCCESS;
}

void init_program() {
	seconds = 0.0;
	window_width = window_height = 512;
	world_radious = sqrtf(100.0);
	projection_type = true;
	reset_camera();

	vertex_grid_side = 150;
	use_index = true;
	use_normal_color = false;
	use_triangle_strip = false;
	use_gouraud = true;
	rotate_light = true;
	wireframe_mode = false;
	light_direction = glm::vec3(0.0, 0.0, 1.0);
}

void reset_camera() {
	base_rotation_angles = glm::vec2(0.0, -90.0);
	new_rotation_angles = glm::vec2(0.0, 0.0);
	scale_perspective = 60.0;
	scale_ortho = 1.0;
}

void create_glut_window() {
	glutInitDisplayMode (GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize (512, 512);
	win = glutCreateWindow ("Jorge Garcia, Project 3");
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
	glDeleteBuffers(1, &VBO_indexed_id);
	glDeleteBuffers(1, &VBO_unidexed_id);
	glDeleteBuffers(1, &index_buffer_id);
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
			use_index = (!use_index);
		break;

		case GLUT_KEY_F2:
			use_normal_color = (!use_normal_color);
			use_gouraud = !use_normal_color;
		break;

		case GLUT_KEY_F3:
			use_gouraud = (!use_gouraud);
			use_normal_color = !use_gouraud;
		break;

		case GLUT_KEY_F4:
			rotate_light = (!rotate_light);
		break;
	}

	glutPostRedisplay();
}

void keyboard(unsigned char key, int mouse_x, int mouse_y) {
	switch (key) {

		case '+':
			add_vertex();
			populate_indexed_vbo();
			populate_unindexed_vbo();
		break;

		case '-':
			remove_vertex();
			populate_indexed_vbo();
			populate_unindexed_vbo();
		break;

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

		case 's':
		case 'S':
			save_experiment();
		break;

		case 'w':
		case 'W':
			wireframe_mode = !wireframe_mode;
			if (wireframe_mode) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			} else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

   light_angle += 1.0f * delta_seconds;
   //light_angle = static_cast<int>(light_angle);
   
   gl_error("At idle"); //check for errors and print error strings
   glutPostRedisplay();
}

void mouse_active(int mouse_x, int mouse_y) {
	glm::vec2 mouse_current;
	if (mouse_dragging) {
		mouse_current.x = static_cast<float>(mouse_x);
		mouse_current.y = static_cast<float>(mouse_y);
	
		glm::vec2 deltas = mouse_start_drag - mouse_current;

		new_rotation_angles.x = deltas.x / glutGet(GLUT_WINDOW_WIDTH) * 180;
		new_rotation_angles.y = deltas.y / glutGet(GLUT_WINDOW_HEIGHT) * 180;
	}
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
		//std::cout << "Rotation Angles: (" << base_rotation_angles.x << ", " << base_rotation_angles.y << std::endl;
	}
	glutPostRedisplay();
}

void save_experiment() {
	std::ofstream output_file_indexed("data_indexed.txt");
	std::ofstream output_file_unindexed("data_unindexed.txt");
	output_file_indexed << "NumTriangles\tMiliseconds\tUseindex" << std::endl;
	output_file_unindexed << "NumTriangles\tMiliseconds\tUseindex" << std::endl;
	for (auto it = points.begin(); it != points.end(); ++it) {
		if (it->m_indexed) {
			output_file_indexed << it->m_triangles << "\t" << it->m_time << std::endl;
		} else {
			output_file_unindexed << it->m_triangles << "\t" << it->m_time << std::endl;
		}
	}
	output_file_indexed.close();
	output_file_unindexed.close();
}

void create_menus() {
	change_size_menu = glutCreateMenu(change_size);
	glutAddMenuEntry("10", 1);
	glutAddMenuEntry("20", 2);
	glutAddMenuEntry("50", 3);
	glutAddMenuEntry("100", 4);
	glutAddMenuEntry("150", 5);
	glutAddMenuEntry("Quit", 6);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void change_size (int id) {
	if (id == 1) {
		vertex_grid_side = 10;
	} else if (id == 2) {
		vertex_grid_side = 20;
	} else if (id == 3) {
		vertex_grid_side = 50;
	} else if (id == 4) {
		vertex_grid_side = 100;
	} else if (id == 5) {
		vertex_grid_side = 150;
	} else if (id == 6) {
		exit_glut();
	}
	populate_indexed_vbo();
	populate_unindexed_vbo();
	
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
