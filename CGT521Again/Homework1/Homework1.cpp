#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cstdlib>
#include <iostream>

#define GLM_FORCE_PURE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "opengl/HelperFunctions.h"
#include "opengl/OpenGLProgram.h"
#include "Globals.h"
#include "Callbacks.h"
#include "LoadMesh.h"
#include "Homework1.h"

using namespace std;


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
	delete options::program_ptr;

	
	glutDestroyWindow(options::window);
	exit(EXIT_SUCCESS);
}

void init_OpenGL() {
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
	}
	opengl::get_OpenGL_info();

	options::program_ptr = new opengl::OpenGLProgram("shaders/vertexShader.glsl", "shaders/fragmentShader.glsl");

	if (!options::program_ptr->is_ok()) {
		cerr << "Error at GL program creation" << endl;
		opengl::gl_error();
		exit(EXIT_FAILURE);
	}

	opengl::get_error_log();
	
	options::u_PVM_location = options::program_ptr->get_uniform_location("PVM");
	options::u_NormalMatrix_location = options::program_ptr->get_uniform_location("NormalMatrix");
	options::u_VM_location = options::program_ptr->get_uniform_location("VM");

	//Light options for the fragment shader
	options::u_LightPosition_location = options::program_ptr->get_uniform_location("lightPosition");
	options::u_La_location = options::program_ptr->get_uniform_location("La");
	options::u_Ld_location = options::program_ptr->get_uniform_location("Ld");
	options::u_Ls_location = options::program_ptr->get_uniform_location("Ls");
	options::u_view = options::program_ptr->get_uniform_location("view");

	//Material properties for the fragment shader
	options::u_Ka_location = options::program_ptr->get_uniform_location("Ka");
	options::u_Kd_location = options::program_ptr->get_uniform_location("Kd");
	options::u_Ks_location = options::program_ptr->get_uniform_location("Ks");
	options::u_shininess_location = options::program_ptr->get_uniform_location("shininess");

	//Activate antialliasing
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	//initialize some basic rendering state
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

	opengl::gl_error("At scene creation");
}

void create_glut_window() {
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	options::window = glutCreateWindow("Jorge Garcia - Homework 1");
}

void create_glut_callbacks() {
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutMouseWheelFunc(mouse_wheel);
	/*
	glutSpecialFunc(special_keyboard);
	*/
	glutMouseFunc(mouse);
	glutMotionFunc(mouse_active);
	glutReshapeFunc(reshape);
}



void init_program() {
	reset_camera();
	//Create light source
	options::light.setType(scene::PUNTUAL);
	//Remember shader calculations are in view space, this light is always a little above the
	//camera. (These coordinates are relative to the camera center).
	//Angle that makes the camera with the center
	const float light_angle = TAU / 16.0f;
	options::light.setPosition(options::world_radious * glm::vec3(0.0f, glm::sin(light_angle), 1.0f - cos(light_angle)));
	options::light.setDirection(glm::vec3(0.0f));
	options::light.setAperture(TAU / 8.0f);

	//Create material
	options::material = scene::Material(glm::vec3(1.0f, 1.0f, 0.0f), 32.0f);

	//Load the mesh
	//We need to have the program active at Mesh loading
	options::program_ptr->use_program();
		options::mesh_data = assets::LoadMesh(options::mesh_name);
	glUseProgram(0);
	
}


void display() {

	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	options::program_ptr->use_program();
	mat4 I(1.0f);

	//Model
	mat4 M = glm::scale(I, glm::vec3(options::mesh_data.mScaleFactor));

	//View
	/* Camera rotation must be accumulated: base rotation then new rotation */
	mat4 camRot = glm::mat4_cast(options::camera_new_rotation * options::camera_base_rotation);
	vec3 position = options::camera_position;
	vec3 center = options::camera_center;
	mat4 V = glm::lookAt(position, center, options::camera_up) * camRot;

	//Projection
	GLfloat aspect = (float)glutGet(GLUT_WINDOW_WIDTH) / (float)glutGet(GLUT_WINDOW_HEIGHT);
	GLfloat fovy = options::field_of_view_y;
	GLfloat zNear = 0.1f;
	GLfloat zFar = 100.0f;
	mat4 P = glm::perspective(fovy, aspect, zNear, zFar);

	vec4 color = vec4(1.0f, 1.0f, 0.0f, 1.0f);

	if (options::u_PVM_location != -1) {
		glUniformMatrix4fv(options::u_PVM_location, 1, GL_FALSE, glm::value_ptr(P * V * M));
	}
	if (options::u_VM_location != -1) {
		glUniformMatrix4fv(options::u_VM_location, 1, GL_FALSE, glm::value_ptr(V * M));
	}
	if (options::u_NormalMatrix_location != -1) {
		glUniformMatrix4fv(options::u_NormalMatrix_location, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(V * M))));
	}

	//Pass light source to shader
	pass_light_and_material();

	/* Bind */
	glBindVertexArray(options::mesh_data.mVao);
	glVertexAttribDivisor(options::mesh_data.mVao, 1);

	/* Draw */
	int num_instances = 9;
	glDrawElementsInstanced(GL_TRIANGLES, options::mesh_data.mNumIndices, GL_UNSIGNED_INT, 0, num_instances);

	/* Unbind and clean */
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);

	glutSwapBuffers();
	opengl::gl_error("At the end of display");
}


void pass_light_and_material() {
	//Light properties
	if (options::u_LightPosition_location != -1) {
		glUniform3fv(options::u_LightPosition_location, 1, glm::value_ptr(options::light.getPosition()));
	}
	if (options::u_La_location != -1) {
		glUniform3fv(options::u_La_location, 1, glm::value_ptr(options::light.getLa()));
	}
	if (options::u_Ld_location != -1) {
		glUniform3fv(options::u_Ld_location, 1, glm::value_ptr(options::light.getLd()));
	}
	if (options::u_Ls_location != -1) {
		glUniform3fv(options::u_Ls_location, 1, glm::value_ptr(options::light.getLs()));
	}

	//Material properties
	if (options::u_Ka_location != -1) {
		glUniform3fv(options::u_Ka_location, 1, glm::value_ptr(options::material.getKa()));
	}
	if (options::u_Kd_location != -1) {
		glUniform3fv(options::u_Kd_location, 1, glm::value_ptr(options::material.getKd()));
	}
	if (options::u_Ks_location != -1) {
		glUniform3fv(options::u_Ks_location, 1, glm::value_ptr(options::material.getKs()));
	}
	if (options::u_shininess_location != -1) {
		glUniform1f(options::u_shininess_location, options::material.getShininnes());
	}
}