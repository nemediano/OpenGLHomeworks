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

opengl::OpenGLProgram* program_points_ptr = nullptr;
opengl::OpenGLProgram* program_contours_ptr = nullptr;
opengl::OpenGLProgram* program_polygons_ptr = nullptr;

using namespace std;
//Glut window pointer
int window = 0;
glm::ivec2 window_size = glm::ivec2();
glm::vec3 world_low = glm::vec3();
glm::vec3 world_high = glm::vec3();

//Variables for GPU side
GLint u_PVM_location = -1;
GLint u_Color_location = -1;
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

int nPoints = 3;


//Program management
void exit_glut();
void init_OpenGL();
void init_program();
void create_glut_window();
void create_glut_callbacks();
void allocate_buffers();
void update_gpu_data();

//Scene creation
void create_primitives();
void render_points(const glm::mat4& PVM);
void render_lines(const glm::mat4& PVM);
void render_triangles(const glm::mat4& PVM);

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
//void mouse_active(int mouse_x, int mouse_y);
void mouse(int button, int state, int mouse_x, int mouse_y);
//void mouse_wheel(int wheel, int direction, int mouse_x, int mouse_y);
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

	delete program_points_ptr;
	delete program_contours_ptr;
	delete program_polygons_ptr;

	glutDestroyWindow(window);
	exit(EXIT_SUCCESS);
}

void init_OpenGL() {
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
	}
	opengl::get_OpenGL_info();

	program_points_ptr = new opengl::OpenGLProgram("shaders/vertexShaderPoints.glsl", "shaders/fragmentShaderPoints.glsl");
	program_contours_ptr = new opengl::OpenGLProgram("shaders/vertexShaderContours.glsl", "shaders/fragmentShaderContours.glsl");
	program_polygons_ptr = new opengl::OpenGLProgram("shaders/vertexShaderPolygons.glsl", "shaders/fragmentShaderPolygons.glsl");
	
	if (!(program_points_ptr->is_ok() && program_contours_ptr->is_ok() && program_polygons_ptr->is_ok())) {
		cerr << "Error at GL program creation" << endl;
		opengl::gl_error();
		exit(EXIT_FAILURE);
	}

	opengl::get_error_log();
	
	
	//Activate antialliasing
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

	//initialize some basic rendering state
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.9f, 0.9f, 0.9f, 1.0f);

	//ability to make the point a little bigger in the shader
	glEnable(GL_PROGRAM_POINT_SIZE);
	//The alternative is to disable the previous and use
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
	//glutMouseWheelFunc(mouse_wheel);
	/*
	glutSpecialFunc(special_keyboard);
	*/
	glutMouseFunc(mouse);
	//glutMotionFunc(mouse_active);
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
	nPoints = 0;
	window_size = glm::ivec2(512, 512);
	world_high = glm::vec3(1.0f, 1.0f, 1.0f);
	world_low = glm::vec3(-1.0f, -1.0f, -1.0f);
	
	allocate_buffers();
	//create_primitives();
	//update_gpu_data();
}


void display() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
	//There is something fishy here!!
	glm::mat4 PVM = glm::transpose(P * V * M);

	if (!indices_polygons.empty()) {
		render_triangles(PVM);
	}
	if (!indices_contours.empty()) {
		render_lines(PVM);
	}
	if (!indices_points.empty()) {
		render_points(PVM);
	}
	

	glutSwapBuffers();
	opengl::gl_error("At the end of display");
}


void create_primitives() {

	const GLuint SIDES = 9;
	//GLuint SIDES = nPoints;
	indices_points.clear();
	for (auto i = 0; i < SIDES; ++i) {
		indices_points.push_back(i);
	}
	indices_contours.clear();
	for (auto i = 0; i < SIDES; ++i) {
		indices_contours.push_back(i);
	}

	//Might not work as expected!!!
	indices_polygons.clear();
	for (auto i = 0; i < SIDES; ++i) {
		indices_polygons.push_back(i);
	}

	glm::vec3 colors[] = { 
		                   glm::vec3(1.0f, 0.0f, 0.0f),
						   glm::vec3(0.0f, 1.0f, 0.0f),
						   glm::vec3(0.0f, 0.0f, 1.0f), 
	                     };
	
	Vertex tmpVertex;
	float delta_angle = TAU / SIDES;
	float angle = TAU / 4;
	glm::vec2 p;
	float radius = 0.5f;
	for (auto i = 0; i < SIDES; ++i) {
		p.x = radius * cos(angle);
		p.y = radius * sin(angle);
		angle += delta_angle;
		tmpVertex.position = p;
		tmpVertex.color = colors[i % 3];
		vertices.push_back(tmpVertex);
	}
}

void keyboard(unsigned char key, int mouse_x, int mouse_y) {
	if (key == 27) {//press ESC to exit
		exit_glut();
	} else if (key == 'c' || key == 'C') {
		nPoints -= 3;
		create_primitives();
	} else if (key == 'v' || key == 'V') {
		nPoints += 3;
		create_primitives();
	}

	glutPostRedisplay();
}

void mouse(int button, int state, int mouse_x, int mouse_y) {
	glm::vec2 mouse_in_world;
	glm::vec2 mouse_in_window = glm::vec2(mouse_x, window_size.y - mouse_y);
	glm::vec3 world_size = world_high - world_low;
	mouse_in_world = glm::vec2(world_size.x / window_size.x, world_size.y / window_size.y) * mouse_in_window - 0.5f * glm::vec2(world_size.x, world_size.y);
	
	glm::vec3 red = glm::vec3(1.0f, 0.0f, 0.0f);
	if (button == GLUT_LEFT && state == GLUT_DOWN) {
		Vertex tmp;
		tmp.position = mouse_in_world;
		tmp.color = red;
		vertices.push_back(tmp);
		indices_points.push_back(vertices.size() - 1);
		update_gpu_data();
	}
	//cout << "(" << mouse_in_world.x << ", " << mouse_in_world.y << ")" << endl;
	glutPostRedisplay();
}

void render_points(const glm::mat4& PVM) {
	/* --------------------Points-------------------- */
	program_points_ptr->use_program();
	u_PVM_location = program_points_ptr->get_uniform_location("PVM");
	a_position_loc = program_points_ptr->get_attrib_location("Position");
	a_color_loc = program_points_ptr->get_attrib_location("Color");
	if (u_PVM_location != -1) {
		glUniformMatrix4fv(u_PVM_location, 1, GL_FALSE, glm::value_ptr(PVM));
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
	glDrawElements(GL_POINTS, indices_points.size(), GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
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
}

void render_lines(const glm::mat4& PVM) {
	/* -------------------- Contours -------------------- */
	program_contours_ptr->use_program();
	u_PVM_location = program_contours_ptr->get_uniform_location("PVM");
	u_Color_location = program_contours_ptr->get_uniform_location("Color");
	a_position_loc = program_contours_ptr->get_attrib_location("Position");
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 0.0f);
	if (u_PVM_location != -1) {
		glUniformMatrix4fv(u_PVM_location, 1, GL_FALSE, glm::value_ptr(PVM));
	}
	if (u_Color_location != -1) {
		glUniform3fv(u_Color_location, 1, glm::value_ptr(color));
	}
	/* Bind */
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	if (a_position_loc != -1) {
		glEnableVertexAttribArray(a_position_loc);
		glVertexAttribPointer(a_position_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSET_OF(Vertex, position));
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferContours);
	/* Draw */
	glDrawElements(GL_LINE_LOOP, indices_contours.size(), GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
	/* Unbind and clean */
	if (a_position_loc != -1) {
		glDisableVertexAttribArray(a_position_loc);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

void render_triangles(const glm::mat4& PVM) {
	/* -------------------- Polygons -------------------- */
	program_polygons_ptr->use_program();
	u_PVM_location = program_polygons_ptr->get_uniform_location("PVM");
	u_Color_location = program_polygons_ptr->get_uniform_location("Color");
	a_position_loc = program_polygons_ptr->get_attrib_location("Position");
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	if (u_PVM_location != -1) {
		glUniformMatrix4fv(u_PVM_location, 1, GL_FALSE, glm::value_ptr(PVM));
	}
	if (u_Color_location != -1) {
		glUniform3fv(u_Color_location, 1, glm::value_ptr(color));
	}
	/* Bind */
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	if (a_position_loc != -1) {
		glEnableVertexAttribArray(a_position_loc);
		glVertexAttribPointer(a_position_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSET_OF(Vertex, position));
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferPolygons);
	/* Draw */
	glDrawElements(GL_TRIANGLES, indices_polygons.size(), GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
	/* Unbind and clean */
	if (a_position_loc != -1) {
		glDisableVertexAttribArray(a_position_loc);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

void allocate_buffers() {
	GLuint tmp[4] = { 0 };
	//Create the buffers
	glCreateBuffers(4, tmp);
	vbo = tmp[0];
	indexBufferPoints = tmp[1];
	indexBufferContours = tmp[2];
	indexBufferPolygons = tmp[3];

	const unsigned short MAX_POINT = 20;

	//First create the vertices
	glNamedBufferStorage(vbo, MAX_POINT * sizeof(Vertex), nullptr, GL_DYNAMIC_STORAGE_BIT);
	
	//Indices for points
	glNamedBufferStorage(indexBufferPoints, MAX_POINT * sizeof(unsigned short), nullptr, GL_DYNAMIC_STORAGE_BIT);
	//Indices for contours
	glNamedBufferStorage(indexBufferContours, MAX_POINT * sizeof(unsigned short), nullptr, GL_DYNAMIC_STORAGE_BIT);
	//Indices for polygons
	glNamedBufferStorage(indexBufferPolygons, MAX_POINT * sizeof(unsigned short), nullptr, GL_DYNAMIC_STORAGE_BIT);

	
	//Clean
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	opengl::gl_error("At the end of allocate buffers");
}

void update_gpu_data() {
		
	//First send the vertices
	//glBindBuffer(GL_ARRAY_BUFFER, vbo);
	//glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	glNamedBufferSubData(vbo, 0, vertices.size() * sizeof(Vertex), vertices.data());
	//Indices for points
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferPoints);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_points.size() * sizeof(unsigned short), indices_points.data(), GL_STATIC_DRAW);
	glNamedBufferSubData(indexBufferPoints, 0, indices_points.size() * sizeof(unsigned short), indices_points.data());
	//Indices for contours
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferContours);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_contours.size() * sizeof(unsigned short), indices_contours.data(), GL_STATIC_DRAW);
	glNamedBufferSubData(indexBufferContours, 0, indices_contours.size() * sizeof(unsigned short), indices_contours.data());
	//Indices for polygons
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferPolygons);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_polygons.size() * sizeof(unsigned short), indices_polygons.data(), GL_STATIC_DRAW);
	glNamedBufferSubData(indexBufferPolygons, 0, indices_polygons.size() * sizeof(unsigned short), indices_polygons.data());
	
	//Clean
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	opengl::gl_error("At the end of update gpu data");
}