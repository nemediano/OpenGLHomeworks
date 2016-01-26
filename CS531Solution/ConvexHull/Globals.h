#ifndef GLOBALS_H
#define GLOBALS_H

#include <vector>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include "opengl/OpenGLProgram.h"


struct Vertex {
	glm::vec2 position;
	glm::vec3 color;
};

namespace options {
	//OpenGl program handlers
	extern opengl::OpenGLProgram* program_points_ptr;
	extern opengl::OpenGLProgram* program_contours_ptr;
	extern opengl::OpenGLProgram* program_polygons_ptr;

	//Glut window pointer
	extern int window;
	extern glm::vec3 world_low;
	extern glm::vec3 world_high;

	//Variables for GPU side
	extern GLint u_PVM_location;
	extern GLint u_Color_location;
	extern GLint a_position_loc;
	extern GLint a_color_loc;

	//Manage the Vertex Buffer Objects
	extern GLuint vbo;
	extern GLuint indexBufferPoints;
	extern GLuint indexBufferContours;
	extern GLuint indexBufferPolygons;

	//Two math constants (New glm uses radians as default)
	extern const float TAU;
	extern const float PI;

	extern int nPoints;

	//Program logic
	extern vector<Vertex> vertices;
	extern vector<unsigned short> indices_points;
	extern vector<unsigned short> indices_contours;
	extern vector<unsigned short> indices_polygons;
}

#endif