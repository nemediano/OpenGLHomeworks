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

#include "NonPhotoRendering.h"



scene::Mesh* mesh_ptr = nullptr;
scene::Mesh* quad_ptr = nullptr;
image::Texture* texture_map_ptr = nullptr;
//Select filter using shader subroutine
std::vector<GLuint> filters;
GLuint *fragment_options_array = nullptr;
GLsizei fragment_filters_counter = 0;
//Array to hold the three enums for render into FBO
GLenum fbo_buffers[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};

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
	delete options::program_pass1_ptr;
	delete options::program_pass2_ptr;
	delete mesh_ptr;
	delete quad_ptr;
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

	options::u_P_location = options::program_pass1_ptr->get_uniform_location("P");
	options::u_V_location = options::program_pass1_ptr->get_uniform_location("V");
	options::u_M_location = options::program_pass1_ptr->get_uniform_location("M");
	options::u_time_location = options::program_pass1_ptr->get_uniform_location("time");
	options::u_texture_option_location = options::program_pass1_ptr->get_uniform_location("texture_option");

	options::a_position_loc = options::program_pass1_ptr->get_attrib_location("Position");
	options::a_normal_loc = options::program_pass1_ptr->get_attrib_location("Normal");
	options::a_texture_coordinate_loc = options::program_pass1_ptr->get_attrib_location("TextureCoordinate");

	//Light options for the fragment shader
	options::u_LightPosition_location = options::program_pass1_ptr->get_uniform_location("lightPosition");
	options::u_La_location = options::program_pass1_ptr->get_uniform_location("La");
	options::u_Ld_location = options::program_pass1_ptr->get_uniform_location("Ld");
	options::u_Ls_location = options::program_pass1_ptr->get_uniform_location("Ls");

	//Material properties for the fragment shader
	options::u_Ka_location = options::program_pass1_ptr->get_uniform_location("Ka");
	options::u_Kd_location = options::program_pass1_ptr->get_uniform_location("Kd");
	options::u_Ks_location = options::program_pass1_ptr->get_uniform_location("Ks");
	options::u_shininess_location = options::program_pass1_ptr->get_uniform_location("shininess");

	//Texture map 
	options::u_texture_map_location = options::program_pass1_ptr->get_uniform_location("texture_map");
	
	/************************************************************************/
	/* Uniforms and attributes for second OpenGL program                    */
	/************************************************************************/
	options::u_color_map_location_2 = options::program_pass2_ptr->get_uniform_location("color_map");
	options::u_color_shading_map_location_2 = options::program_pass2_ptr->get_uniform_location("color_shade_map");
	options::u_normal_map_location_2 = options::program_pass2_ptr->get_uniform_location("normal_map");
	options::a_position_location_2 = options::program_pass2_ptr->get_attrib_location("pos_attrib");
	
	options::u_filter_option_location = options::program_pass2_ptr->get_subroutine_uniform_location(GL_FRAGMENT_SHADER, "selectedFilter");
	filters.push_back(options::program_pass2_ptr->get_subroutine_index_location(GL_FRAGMENT_SHADER, "no_filter"));
	filters.push_back(options::program_pass2_ptr->get_subroutine_index_location(GL_FRAGMENT_SHADER, "average_3x3"));
	filters.push_back(options::program_pass2_ptr->get_subroutine_index_location(GL_FRAGMENT_SHADER, "average_9x9"));
	filters.push_back(options::program_pass2_ptr->get_subroutine_index_location(GL_FRAGMENT_SHADER, "edge_detection"));
	filters.push_back(options::program_pass2_ptr->get_subroutine_index_location(GL_FRAGMENT_SHADER, "sobel_filter"));
	filters.push_back(options::program_pass2_ptr->get_subroutine_index_location(GL_FRAGMENT_SHADER, "sobel_threshold"));
	filters.push_back(options::program_pass2_ptr->get_subroutine_index_location(GL_FRAGMENT_SHADER, "sobel_threshold_normals"));
	
	glGetProgramStageiv(options::program_pass2_ptr->get_program_id(), GL_FRAGMENT_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORMS, &fragment_filters_counter);
	fragment_options_array = new GLuint[fragment_filters_counter];

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
	options::window = glutCreateWindow("Non-Photorealistic rendering");
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
	glm::vec3 Kd = glm::vec3(0.714f, 0.4284f, 0.18144);
	glm::vec3 Ks = glm::vec3(0.393548f, 0.271906f, 0.166721f);
	float shine = 10.0f;

	options::default_material = scene::Material(Ka, Kd, Ks, shine);
	//options::default_material = scene::BRONZE;

	
	/************************************************************************/
	/* For draw pass 1                                                     */
	/************************************************************************/
	options::mesh_file = "Amago0.obj";
	options::texture_file = "AmagoT.bmp";
	reload_mesh_and_texture();
	
	/************************************************************************/
	/* For draw pass 2                                                      */
	/************************************************************************/
	quad_ptr = new scene::Mesh();
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	Vertex v0, v1, v2, v3;
	v0.position = glm::vec3(-1.0, -1.0, 0.0f);
	v1.position = glm::vec3( 1.0, -1.0, 0.0f);
	v2.position = glm::vec3( 1.0,  1.0, 0.0f);
	v3.position = glm::vec3(-1.0,  1.0, 0.0f);
	vertices.push_back(v0);
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);
	quad_ptr->set_vertices(vertices);
	quad_ptr->set_index(indices);
	quad_ptr->send_data_to_gpu();

	/************************************************************************/
	/* Crete and setup Frame buffer object  to store render pass 1          */
	/************************************************************************/
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);
	//Create a texture to render the mesh with texture during pass 1
	glGenTextures(1, &options::fbo_color_texture);
	glBindTexture(GL_TEXTURE_2D, options::fbo_color_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//Create a texture to render the mesh with texture and Phong shading during pass 1
	glGenTextures(1, &options::fbo_shading_texture);
	glBindTexture(GL_TEXTURE_2D, options::fbo_shading_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//Create a texture to store the normals during pass 1
	glGenTextures(1, &options::fbo_normals_texture);
	glBindTexture(GL_TEXTURE_2D, options::fbo_normals_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	//Create render buffer for depth.
	glGenRenderbuffers(1, &options::depth_buffer_id);
	glBindRenderbuffer(GL_RENDERBUFFER, options::depth_buffer_id);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	//Create the Frame Buffer object
	glGenFramebuffers(1, &options::fbo_id);
	glBindFramebuffer(GL_FRAMEBUFFER, options::fbo_id);
	//Attach texture to render mesh with texture into it
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, options::fbo_color_texture, 0);
	//Attach texture to render mesh with texture and Phong shading into it
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, options::fbo_shading_texture, 0);
	//Attach texture to write the normals into it
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, options::fbo_normals_texture, 0);
	//Attach depth buffer to FBO
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, options::depth_buffer_id);
	opengl::check_framebuffer_status();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void display() {

	options::program_pass1_ptr->use_program();
	//We are gonna render to this FBO
	glBindFramebuffer(GL_FRAMEBUFFER, options::fbo_id); 
	//We need to say that this pass will render two buffers, one with the Phong shading
	//and one with the normals
	glDrawBuffers(3, fbo_buffers);
	//Make the viewport match the texture in the FBO
	int texture_width;
	int texture_height;
	glBindTexture(GL_TEXTURE_2D, options::fbo_color_texture);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texture_width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texture_height);
	glViewport(0, 0, texture_width, texture_height);
	//Clear the FBO
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw_pass_1();
	//opengl::gl_error("After first pass");
	//Do not render the next pass to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//Render to back buffer
	glDrawBuffer(GL_BACK);
	
	options::program_pass2_ptr->use_program();
	//Make the viewport match the whole window
	int window_width = glutGet(GLUT_WINDOW_WIDTH);
	int window_height = glutGet(GLUT_WINDOW_HEIGHT);
	glViewport(0, 0, window_width, window_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//opengl::gl_error("Before second pass");
	draw_pass_2();
	//opengl::gl_error("After second pass");

	//Cleaning
	glUseProgram(0);
	glutSwapBuffers();
	//opengl::gl_error("At the end of display");
}

void draw_pass_1() {
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

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
	
	//Pass light source to shader
	pass_light();

	mesh_ptr->draw_triangles(options::a_position_loc, options::a_normal_loc, options::a_texture_coordinate_loc, 1);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void draw_pass_2() {
	//Pass updated textures to fragment shader
	//The one with just mesh and texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, options::fbo_color_texture);
	if (options::u_color_map_location_2 != -1) {
		// we bound texture with color to texture unit 0
		glUniform1i(options::u_color_map_location_2, 0);
	}
	//The one with mesh and texture using Phong shading
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, options::fbo_shading_texture);
	if (options::u_color_shading_map_location_2 != -1) {
		// we bound texture with Phong shading to texture unit 1
		glUniform1i(options::u_color_shading_map_location_2, 1);
	}
	//The one with the normals
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, options::fbo_normals_texture);
	if (options::u_normal_map_location_2 != -1) {
		// we bound texture with Phong shading to texture unit 2
		glUniform1i(options::u_normal_map_location_2, 2);
	}
	//pass the option of the filter we want to use
	fragment_options_array[options::u_filter_option_location] = filters[options::filter_option];
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, fragment_filters_counter, fragment_options_array);
	//Draw using this texture
	quad_ptr->draw_triangles(options::a_position_location_2, -1, -1);
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

	//Material properties
	if (options::u_Ka_location != -1) {
		glUniform3fv(options::u_Ka_location, 1, glm::value_ptr(options::default_material.getKa()));
	}
	if (options::u_Kd_location != -1) {
		glUniform3fv(options::u_Kd_location, 1, glm::value_ptr(options::default_material.getKd()));
	}
	if (options::u_Ks_location != -1) {
		glUniform3fv(options::u_Ks_location, 1, glm::value_ptr(options::default_material.getKs()));
	}
	if (options::u_shininess_location != -1) {
		glUniform1f(options::u_shininess_location, options::default_material.getShininnes());
	}
}

void reload_shaders() {
	if (options::program_pass1_ptr) {
		delete options::program_pass1_ptr;
	}
	options::program_pass1_ptr = new opengl::OpenGLProgram("shaders/vertexShader.glsl", "shaders/fragmentShader.glsl");
	
	if (options::program_pass2_ptr) {
		delete options::program_pass2_ptr;
	}
	options::program_pass2_ptr = new opengl::OpenGLProgram("shaders/simpleVertexShader.glsl", "shaders/simpleFragmentShader.glsl");
	
	
	glClearColor(options::background_color.r, options::background_color.g, options::background_color.b, options::background_color.a);

	if (!options::program_pass1_ptr->is_ok()) {
		cerr << "Error at first GL program creation" << endl;
		opengl::gl_error();
		//exit(EXIT_FAILURE);
		glClearColor(options::background_shader_error_color.r, options::background_shader_error_color.g,
				     options::background_shader_error_color.b, options::background_shader_error_color.a);
	}

	if (!options::program_pass2_ptr->is_ok()) {
		cerr << "Error at second GL program creation" << endl;
		opengl::gl_error();
		//exit(EXIT_FAILURE);
		glClearColor(options::background_shader_error_color.r, options::background_shader_error_color.g,
			options::background_shader_error_color.b, options::background_shader_error_color.a);
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