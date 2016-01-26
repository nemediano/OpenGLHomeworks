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

//GLM using directives
using glm::vec3;
using glm::vec2;
using glm::ivec2;
using glm::mat4;

#include "convex_hull.h"
#include "opengl/HelperFunctions.h"
#include "opengl/OpenGLProgram.h"
#include "Globals.h"
#include "interface.h"

using namespace std;
using namespace options;

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
	glutKeyboardFunc(keyboard);
	/*
	glutIdleFunc(idle);
	glutSpecialFunc(special_keyboard);
	glutMotionFunc(mouse_active);
	glutMouseWheelFunc(mouse_wheel);
	*/
	glutMouseFunc(mouse);
	glutReshapeFunc(reshape);
}


void init_program() {
	nPoints = 0;
	
	world_high = vec3(1.0f, 1.0f, 1.0f);
	world_low = vec3(-1.0f, -1.0f, -1.0f);
	
	allocate_buffers();
}


void display() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 I(1.0f);
	
	//Model
	mat4 M = I;
	
	//View
	vec3 position = vec3(0.0, 0.0, 1.0f);
	vec3 center = vec3(0.0f, 0.0f, 0.0f);
	vec3 camera_up = vec3(0.0f, 1.0f, 0.0f);
	mat4 V = glm::lookAt(position, center, camera_up);

	//Projection
	GLfloat xLeft = world_low.x;
	GLfloat xRight = world_high.x;
	GLfloat yBottom = world_low.y;
	GLfloat yTop = world_high.y;
	GLfloat zNear = world_low.z;
	GLfloat zFar = world_high.z;
	mat4 P = glm::ortho(xLeft, xRight, yBottom, yTop, zNear, zFar);
	//There is something fishy here!!
	mat4 PVM = glm::transpose(P * V * M);

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

	vec3 colors[] = { 
		                   vec3(1.0f, 0.0f, 0.0f),
						   vec3(0.0f, 1.0f, 0.0f),
						   vec3(0.0f, 0.0f, 1.0f), 
	                     };
	
	Vertex tmpVertex;
	float delta_angle = TAU / SIDES;
	float angle = TAU / 4;
	vec2 p;
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



void render_points(const glm::mat4& PVM) {
	
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

void render_lines(const mat4& PVM) {
	
	program_contours_ptr->use_program();
	u_PVM_location = program_contours_ptr->get_uniform_location("PVM");
	u_Color_location = program_contours_ptr->get_uniform_location("Color");
	a_position_loc = program_contours_ptr->get_attrib_location("Position");
	vec3 color = vec3(1.0f, 1.0f, 0.0f);
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

void render_triangles(const mat4& PVM) {
	
	program_polygons_ptr->use_program();
	u_PVM_location = program_polygons_ptr->get_uniform_location("PVM");
	u_Color_location = program_polygons_ptr->get_uniform_location("Color");
	a_position_loc = program_polygons_ptr->get_attrib_location("Position");
	vec3 color = vec3(1.0f, 1.0f, 1.0f);
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