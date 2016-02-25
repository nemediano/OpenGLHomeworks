#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <set>
#include <string>

using namespace std;

#define GLM_FORCE_PURE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "opengl/HelperFunctions.h"
#include "opengl/OpenGLProgram.h"
#include "glut/Globals.h"
#include "glut/Callbacks.h"
#include "scene/Light.h"
#include "scene/Material.h"
#include "scene/Mesh.h"
#include "image/Texture.h"

#include "LightingDemo.h"

scene::Mesh* mesh_ptr = nullptr;
image::Texture* texture_map_ptr = nullptr;
//Select lighting model using shader subroutine
std::vector<GLuint> available_models;
GLuint *fragment_options_array = nullptr;
GLsizei fragment_options_counter = 0;

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
	delete mesh_ptr;
	delete texture_map_ptr;
	delete fragment_options_array;

	glutDestroyWindow(options::window);
	exit(EXIT_SUCCESS);
}

void init_OpenGL() {
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
	}
	opengl::get_OpenGL_info();
	reload_shaders();
	opengl::get_error_log();

	/************************************************************************/
	/* Uniforms and attributes for first OpenGL program                     */
	/************************************************************************/

	options::u_P_location = options::program_ptr->get_uniform_location("P");
	options::u_V_location = options::program_ptr->get_uniform_location("V");
	options::u_M_location = options::program_ptr->get_uniform_location("M");
	options::u_time_location = options::program_ptr->get_uniform_location("time");
	options::u_texture_option_location = options::program_ptr->get_uniform_location("texture_option");

	options::a_position_loc = options::program_ptr->get_attrib_location("Position");
	options::a_normal_loc = options::program_ptr->get_attrib_location("Normal");
	options::a_texture_coordinate_loc = options::program_ptr->get_attrib_location("TextureCoordinate");

	//Light options for the fragment shader
	options::u_LightPosition_location = options::program_ptr->get_uniform_location("light.position");
	options::u_La_location = options::program_ptr->get_uniform_location("light.La");
	options::u_Ld_location = options::program_ptr->get_uniform_location("light.Ld");
	options::u_Ls_location = options::program_ptr->get_uniform_location("light.Ls");

	//Material properties for the fragment shader
	options::u_Ka_location = options::program_ptr->get_uniform_location("current_material.Ka");
	options::u_Kd_location = options::program_ptr->get_uniform_location("current_material.Kd");
	options::u_Ks_location = options::program_ptr->get_uniform_location("current_material.Ks");
	options::u_shininess_location = options::program_ptr->get_uniform_location("current_material.shine");

	//Location for the different lighting models
	options::u_lighting_model_option_location = options::program_ptr->get_subroutine_uniform_location(GL_FRAGMENT_SHADER, "selectedModel");
	available_models.push_back(options::program_ptr->get_subroutine_index_location(GL_FRAGMENT_SHADER, "phong_shading"));
	available_models.push_back(options::program_ptr->get_subroutine_index_location(GL_FRAGMENT_SHADER, "cook_torrance"));
	

	glGetProgramStageiv(options::program_ptr->get_program_id(), GL_FRAGMENT_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORMS, &fragment_options_counter);
	fragment_options_array = new GLuint[fragment_options_counter];

	//Texture map 
	options::u_texture_map_location = options::program_ptr->get_uniform_location("texture_map");
	
	//Activate antialliasing
	glEnable(GL_MULTISAMPLE);

	//initialize some basic rendering state
	glEnable(GL_DEPTH_TEST);

	opengl::gl_error("At scene creation");
}

void create_glut_window() {
	//Set number of samples per pixel
	glutSetOption(GLUT_MULTISAMPLE, 8);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(800, 800);
	options::window = glutCreateWindow("Lighting demo");
}

void create_glut_callbacks() {
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutMouseWheelFunc(mouse_wheel);
	glutSpecialFunc(special_keyboard);
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
	const float light_angle = TAU / 8.0f;
	options::light.setPosition(options::world_radious * glm::vec3(0.0f, glm::sin(light_angle), 1.0f - cos(light_angle)));
	options::light.setDirection(glm::vec3(0.0f));
	options::light.setAperture(TAU / 8.0f);
	
	
	//Setup default material
	//Material
	glm::vec3 Ka = glm::vec3(0.2125f, 0.1275f, 0.054f);
	glm::vec3 Kd = glm::vec3(0.714f, 0.4284f, 0.18144f);
	glm::vec3 Ks = glm::vec3(0.393548f, 0.271906f, 0.166721f);
	float shine = 10.0f;
	options::materials.push_back(scene::Material(Ka, Kd, Ks, shine));
	options::materials.push_back(scene::EMERALD);
	options::materials.push_back(scene::JADE);
	options::materials.push_back(scene::OBSIDIAN);
	options::materials.push_back(scene::PEARL);
	options::materials.push_back(scene::RUBY);
	options::materials.push_back(scene::TURQUOISE);
	options::materials.push_back(scene::BRASS);
	options::materials.push_back(scene::BRONZE);
	options::materials.push_back(scene::CHROME);
	options::materials.push_back(scene::COPPER);
	options::materials.push_back(scene::GOLD);
	options::materials.push_back(scene::SILVER);
	options::materials.push_back(scene::BLACK_PLASTIC);
	options::materials.push_back(scene::CYAN_PLASTIC);
	options::materials.push_back(scene::GREEN_PLASTIC);
	options::materials.push_back(scene::RED_PLASTIC);
	options::materials.push_back(scene::WHITE_PLASTIC);
	options::materials.push_back(scene::YELLOW_PLASTIC);
	options::materials.push_back(scene::BLACK_RUBBER);
	options::materials.push_back(scene::CYAN_RUBBER);
	options::materials.push_back(scene::GREEN_RUBBER);
	options::materials.push_back(scene::RED_RUBBER);
	options::materials.push_back(scene::WHITE_RUBBER);
	options::materials.push_back(scene::YELLOW_RUBBER);
	options::current_material_index = 0;

	/************************************************************************/
	/* Initial mesh value                                                   */
	/************************************************************************/
	options::mesh_file = "Amago0.obj";
	options::texture_file = "AmagoT.bmp";
	reload_mesh_and_texture();
}


void display() {
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

	/************************************************************************/
	/* Clear buffers an initialization                                      */
	/************************************************************************/
	options::program_ptr->use_program();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/************************************************************************/
	/* Calculations per frame about camera                                  */
	/************************************************************************/
	mat4 I(1.0f);
	//Model
	mat4 M = glm::scale(I, mesh_ptr->get_scale_factor() * glm::vec3(1.0f, 1.0f, 1.0f));
	if (options::rotate_fish) {
		options::angle += TAU * options::delta_seconds / 4.0f;
	}
	M = glm::rotate(M, options::angle, glm::vec3(0.0f, 1.0f, 0.0f));
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

	/************************************************************************/
	/* Pass variables to shader                                             */
	/************************************************************************/

	if (options::u_P_location != -1) {
		glUniformMatrix4fv(options::u_P_location, 1, GL_FALSE, glm::value_ptr(P));
	}
	if (options::u_V_location != -1) {
		glUniformMatrix4fv(options::u_V_location, 1, GL_FALSE, glm::value_ptr(V));
	}
	if (options::u_M_location != -1) {
		glUniformMatrix4fv(options::u_M_location, 1, GL_FALSE, glm::value_ptr(M));
	}
	if (options::u_time_location != -1) {
		glUniform1f(options::u_time_location, options::elapsed_time);
	}
	if (options::u_texture_option_location != -1) {
		glUniform1i(options::u_texture_option_location, options::has_texture ? 1 : 0);
	}

	if (texture_map_ptr) {
		glActiveTexture(GL_TEXTURE0);
		texture_map_ptr->bind();
		if (options::u_texture_map_location != -1) {
			glUniform1i(options::u_texture_map_location, 0); // we bound our texture to texture unit 0
		}
	}
	else {
		pass_material();
	}
	//Pass light source to shader
	pass_light();
	//pass the option of the shading model we want to use
	fragment_options_array[options::u_lighting_model_option_location] = available_models[options::lighting_model_option];
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, fragment_options_counter, fragment_options_array);
	/************************************************************************/
	/*  Send drawing command to gpu                                         */
	/************************************************************************/
	mesh_ptr->draw_triangles(options::a_position_loc, options::a_normal_loc, options::a_texture_coordinate_loc, 1);
	/************************************************************************/
	/* Cleaning the state                                                   */
	/************************************************************************/
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);

	glutSwapBuffers();
	//opengl::gl_error("At the end of display");
}

void pass_light() {
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

}

void reload_shaders() {
	if (options::program_ptr) {
		delete options::program_ptr;
	}
	options::program_ptr = new opengl::OpenGLProgram("shaders/vertexShader.glsl", "shaders/fragmentShader.glsl");
	
	
	glClearColor(options::background_color.r, options::background_color.g, options::background_color.b, options::background_color.a);

	if (!options::program_ptr->is_ok()) {
		cerr << "Error at GL program creation" << endl;
		opengl::gl_error();
		//exit(EXIT_FAILURE);
		glClearColor(options::background_shader_error_color.r, options::background_shader_error_color.g,
				     options::background_shader_error_color.b, options::background_shader_error_color.a);
	}

}

void pass_material() {
	//Material properties
	if (options::u_Ka_location != -1) {
		glUniform3fv(options::u_Ka_location, 1, glm::value_ptr(options::materials[options::current_material_index].getKa()));
	}
	if (options::u_Kd_location != -1) {
		glUniform3fv(options::u_Kd_location, 1, glm::value_ptr(options::materials[options::current_material_index].getKd()));
	}
	if (options::u_Ks_location != -1) {
		glUniform3fv(options::u_Ks_location, 1, glm::value_ptr(options::materials[options::current_material_index].getKs()));
	}
	if (options::u_shininess_location != -1) {
		glUniform1f(options::u_shininess_location, options::materials[options::current_material_index].getShininnes());
	}
}

void reload_mesh_and_texture() {
	//Load mesh from file
	if (mesh_ptr) {
		delete mesh_ptr;
	}
	mesh_ptr = new scene::Mesh(options::mesh_file);
	mesh_ptr->send_data_to_gpu();
	options::has_texture = mesh_ptr->has_texture();

	//Load texture map from file
	if (texture_map_ptr) {
		delete texture_map_ptr;
	}
	texture_map_ptr = options::texture_file.empty() ? nullptr : new image::Texture(options::texture_file);
}