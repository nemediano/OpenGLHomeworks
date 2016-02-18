#include "Globals.h"
#include <glm/gtx/norm.hpp>

namespace options {

	//Scene
	GLfloat world_radious = 2.0f;
	float delta_seconds = 0.0f;
	float elapsed_time = 0.0f;
	float angle = 0.0f;
	bool rotate_fish = false;
	int filter_option = 0;
	int selected_id = options::INSTANCE_NUMBER;
	const int FILTERS_NUMBER = 4;
	const int INSTANCE_NUMBER = 6;

	//Camera handling
	GLfloat field_of_view_y = PI / 3.0f;
	glm::vec3 camera_position = glm::vec3(0.0f, 0.0f, world_radious);
	glm::vec3 camera_center = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec2 camera_pan = glm::vec2(0.0f, 0.0f);
	glm::quat camera_base_rotation = glm::quat(1.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	glm::quat camera_new_rotation = glm::quat(1.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	
	//For the mouse dragging 
	bool mouse_dragging = false;
	glm::vec2 mouse_start_drag = glm::vec2(0.0f, 0.0f);

	//Glut window pointer
	int window = 0;

	//Variables for GPU side program 1
	GLint u_PVM_location = -1;
	GLint u_NormalMatrix_location = -1;
	GLint u_VM_location = -1;
	GLint u_texture_map_location = -1;
	GLint u_selected_location = -1;
	GLint u_time_location = -1;

	GLint a_position_loc = -1;
	GLint a_normal_loc = -1;
	GLint a_texture_coordinate_loc = -1;

	//Variables for GPU side program 2
	GLint u_texture_map_location_2 = -1;
	GLint u_filter_option_location = -1;
	GLint a_position_location_2 = -1;

	//Manage the Frame buffer object for render pass 1
	GLuint fbo_id = -1;
	GLuint depth_buffer_id = -1;
	GLuint fbo_render_texture = -1;
	GLuint fbo_pick_texture = -1;

	//Manage the Vertex Buffer Object
	GLuint vbo = 0;
	GLuint indexBuffer = 0;

	//OpenGL main program
	opengl::OpenGLProgram* program_pass1_ptr = nullptr;
	opengl::OpenGLProgram* program_pass2_ptr = nullptr;

	//Light source
	scene::Light light;
	//Shader location for light
	GLint u_LightPosition_location = -1;
	GLint u_La_location = -1;
	GLint u_Ld_location = -1;
	GLint u_Ls_location = -1;
	//Shader location for the camera
	GLint u_view = -1;

}