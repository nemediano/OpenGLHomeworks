#include "Globals.h"


namespace options {

	opengl::OpenGLProgram* program_points_ptr = nullptr;
	opengl::OpenGLProgram* program_contours_ptr = nullptr;
	opengl::OpenGLProgram* program_polygons_ptr = nullptr;

	//Glut window pointer
	int window = 0;
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

	int nPoints = 0;

	//Program logic
	vector<Vertex> vertices;
	vector<unsigned short> indices_points;
	vector<unsigned short> indices_contours;
	vector<unsigned short> indices_polygons;
}