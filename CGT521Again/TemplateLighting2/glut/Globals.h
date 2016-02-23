#ifndef GLOBALS_H
#define GLOBALS_H
#include <vector>
#include <GL/glew.h>
#define GLM_FORCE_PURE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "../opengl/OpenGLProgram.h"
#include "../scene/Light.h"
#include "../scene/Material.h"

namespace options {

	//Scene
	extern GLfloat world_radious;
	extern float delta_seconds;
	extern float elapsed_time;
	extern float angle;
	extern bool rotate_fish;
	extern bool has_texture;
	extern string texture_file;
	extern string mesh_file;
	extern glm::vec4 background_color;
	extern glm::vec4 background_shader_error_color;

	//Camera handling
	extern GLfloat field_of_view_y;
	extern glm::vec3 camera_position;
	extern glm::vec3 camera_center;
	extern glm::vec3 camera_up;
	extern glm::vec2 camera_pan;
	extern glm::quat camera_base_rotation;
	extern glm::quat camera_new_rotation;
	extern GLfloat field_of_view;

	//For the mouse dragging 
	extern bool mouse_dragging;
	extern glm::vec2 mouse_start_drag;

	//Glut window pointer
	extern int window;

	//Variables for GPU side
	extern GLint u_P_location;
	extern GLint u_V_location;
	extern GLint u_M_location;
	extern GLint u_texture_map_location;
	extern GLint u_time_location;
	extern GLint u_texture_option_location;
	extern GLint u_lighting_model_option_location;

	extern GLint a_position_loc;
	extern GLint a_normal_loc;
	extern GLint a_texture_coordinate_loc;

	//OpenGL main program
	extern opengl::OpenGLProgram* program_ptr;

	//Light source
	extern scene::Light light;
	//Shader location for light
	extern GLint u_LightPosition_location;
	extern GLint u_La_location;
	extern GLint u_Ld_location;
	extern GLint u_Ls_location;
	//Material
	extern int current_material_index;
	extern GLint u_Ka_location;
	extern GLint u_Kd_location;
	extern GLint u_Ks_location;
	extern GLint u_shininess_location;
	extern std::vector<scene::Material> materials;
	//Lighting model
	extern int lighting_model_option;
	extern const int MODELS_NUMBER;
}

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 text_coordinates;
};

struct Triangle {
	glm::vec3 p_0;
	glm::vec3 p_1;
	glm::vec3 p_2;
};

//Two math constants (New glm uses radians as default)
const float TAU = 6.28318f;
const float PI = 3.14159f;


#endif