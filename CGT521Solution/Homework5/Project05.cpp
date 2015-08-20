#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>

//This is a bug on GLM in 32 bits I hope they solve it soon
#define GLM_FORCE_PURE
#define GLM_MESSAGES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include <iostream>
#include <cstdlib>
#include "OpenGLProgram.h"
#include "fbo/FBOHandler.h"

#include "mesh.h"
#include "texture.h"

using namespace std;
static int win = 0;

bool texture_map_flag;
bool texture_animation;
bool rotate_animation;
bool light_mode_flag;
bool fish_animation;

bool mouse_dragging;
glm::vec2 mouse_start_drag;
glm::vec2 base_rotation_angles;
glm::vec2 new_rotation_angles;
glm::vec2 base_pan;
glm::vec2 new_pan;
enum CAM_MODE { PAN, ROTATE, UNUSED };
CAM_MODE mode;
float fovy;

float seconds;
int window_width;
int window_height;

//OpenGL program object
shaders::OpenGLProgram* open_gl_program_ptr = nullptr;
//Frame buffer handler object 
fbo::FBOHandler* fbo_handler_ptr = nullptr;

int VM_loc = -1;
int P_loc = -1;
int Time_loc = -1;
int texture_map_flag_loc = -1;
int light_mode_flag_loc = -1;
int fish_animation_loc = -1;
int rotate_animation_loc = -1;
int view_vector_loc = -1;
int light_position_in_vs_loc = -1;
int light_direction_in_vs_loc = -1;
int tex_loc = -1;

//OpenGL vertex buffer objects
GLuint VertexBuffer = 0;
GLuint NormalBuffer = 0;
GLuint TexCoordBuffer = 0;
GLuint IndexBuffer = 0;

//OpenGL texture object
GLuint TextureID = 0;

//Callback function for special keys
void special_keys(int key, int mouse_x, int mouse_y);
void keyboard (unsigned char key, int mouse_x, int mouse_y);
void mouse_active(int mouse_x, int mouse_y);
void mouse(int button, int state, int mouse_x, int mouse_y);
void mouse_wheel(int wheel, int direction, int mouse_x, int mouse_y);

const float  PI = 3.141593f;
const float TAU = 6.283185f;

void mouse_active(int mouse_x, int mouse_y) {
	glm::vec2 mouse_current;

	mouse_current.x = static_cast<float>(mouse_x);
	mouse_current.y = static_cast<float>(mouse_y);

	glm::vec2 deltas = mouse_start_drag - mouse_current;

	if (mode == ROTATE) {
		new_rotation_angles.x = deltas.x / glutGet(GLUT_WINDOW_WIDTH) * TAU / 2.0f;
		new_rotation_angles.y = deltas.y / glutGet(GLUT_WINDOW_HEIGHT) * TAU / 2.0f;
	}
	else {
		new_pan.x = deltas.x / glutGet(GLUT_WINDOW_WIDTH);
		new_pan.y = -deltas.y / glutGet(GLUT_WINDOW_HEIGHT);
	}

	glutPostRedisplay();
}

void mouse_wheel(int wheel, int direction, int mouse_x, int mouse_y) {
	const float TICKS = PI / 20;
	if (direction > 0.0f && fovy < (PI - TICKS)) {
		fovy += TICKS;
	} else if (direction < 0 && fovy > TICKS) {
		fovy -= TICKS;
	}

	glutPostRedisplay();
}

void mouse(int button, int state, int mouse_x, int mouse_y) {
	if (state == GLUT_DOWN && (button == GLUT_LEFT_BUTTON || button == GLUT_MIDDLE_BUTTON)) {
		mouse_dragging = true;
		mouse_start_drag.x = static_cast<float>(mouse_x);
		mouse_start_drag.y = static_cast<float>(mouse_y);
		if (button == GLUT_LEFT_BUTTON) {
			mode = ROTATE;
		} else {
			mode = PAN;
		}
	} else if (state == GLUT_UP && (button == GLUT_LEFT_BUTTON || button == GLUT_MIDDLE_BUTTON)) {
		mouse_dragging = false;
		if (button == GLUT_LEFT_BUTTON) {
			base_rotation_angles += new_rotation_angles;
			new_rotation_angles = glm::vec2(0.0f, 0.0f);
		} else {
			base_pan += new_pan;
			new_pan = glm::vec2(0.0f, 0.0f);
		}
		mode = UNUSED;
	}
	glutPostRedisplay();
}

void display() {
   if (texture_animation) {
      fbo_handler_ptr->render(texture_animation, seconds); 
   }
   //Bind the texture that you just render
   glBindTexture(GL_TEXTURE_2D, fbo_handler_ptr->get_texture_id());
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   glEnable(GL_DEPTH_TEST);
   glViewport(0, 0, window_width, window_height);

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glm::mat4 I(1.0f);
   //glm::mat4 M = RotateX(base_rotation_angles.y + new_rotation_angles.y) * RotateY(base_rotation_angles.x + new_rotation_angles.x) * Scale(4.0f, 4.0f, 4.0f);
   glm::mat4 M = glm::scale(I, glm::vec3(4.0f, 4.0f, 4.0f));
   M = glm::rotate(M, base_rotation_angles.x + new_rotation_angles.x, glm::vec3(0.0f, 1.0f, 0.0f));
   M = glm::rotate(M, base_rotation_angles.y + new_rotation_angles.y, glm::vec3(1.0f, 0.0f, 0.0f));
   glm::vec3 light_position = glm::vec3(0.0f, 0.707f, 10.0f);
   glm::vec3 eye = glm::vec3(0.0f, 0.0f, 1.0f);
   glm::vec3 at = glm::vec3(0.0f, 0.0f, 0.0f);
   glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
   glm::mat4 V = glm::lookAt(eye, at, up);
   glm::vec3 view = at - eye;
   glm::mat4 P = glm::perspective(fovy, 1.0f, 0.1f, 100.0f);
   glm::vec3 light_direction = at - light_position;
   //reactivate modern OpenGL
   open_gl_program_ptr->use_program();
   
   //Pass program state variables to shader
   if(texture_map_flag_loc != -1) {
	   glUniform1i(texture_map_flag_loc, static_cast<int>(texture_map_flag));
   }
    if(fish_animation_loc != -1) {
		glUniform1i(fish_animation_loc, static_cast<int>(fish_animation));
   }
   if(light_mode_flag_loc != -1) {
	   glUniform1i(light_mode_flag_loc, static_cast<int>(light_mode_flag));
   }
   if(rotate_animation_loc != -1) {
	   glUniform1i(rotate_animation_loc, static_cast<int>(rotate_animation));
   }
   //Geometric variables for the shader
   if (light_position_in_vs_loc != -1) {
	   glUniform4fv(light_position_in_vs_loc, 1, glm::value_ptr(V * glm::vec4(light_position, 1.0f)));
   }
   if (light_direction_in_vs_loc != -1) {
	   glUniform4fv(light_direction_in_vs_loc, 1, glm::value_ptr(V * glm::vec4(light_direction, 1.0f)));
   }
   if (view_vector_loc != -1) {
	   glUniform4fv(view_vector_loc, 1, glm::value_ptr(glm::vec4(view, 1.0f)));
   }
   if(VM_loc != -1) {
	   glUniformMatrix4fv(VM_loc, 1, GL_FALSE, glm::value_ptr(V * M));
   }
   if(P_loc != -1) {
      glUniformMatrix4fv(P_loc, 1, GL_FALSE, glm::value_ptr(P));
   }

   int pos_loc = open_gl_program_ptr->get_attrib_location("pos_attrib");
   int normal_loc = open_gl_program_ptr->get_attrib_location("normal_attrib");
   int tex_coord_loc = open_gl_program_ptr->get_attrib_location("tex_coord_attrib");
	
	glEnableVertexAttribArray(pos_loc);
	glEnableVertexAttribArray(normal_loc);
	glEnableVertexAttribArray(tex_coord_loc);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, NormalBuffer);
	glVertexAttribPointer(normal_loc, 3, GL_FLOAT, false, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, TexCoordBuffer);
	glVertexAttribPointer(tex_coord_loc, 2, GL_FLOAT, false, 0, 0);

	//render indexed primitives
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
	glDrawElements(GL_TRIANGLES, 3 * NumFaces, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(pos_loc);
	glDisableVertexAttribArray(normal_loc);
	glDisableVertexAttribArray(tex_coord_loc);

	glutSwapBuffers();
	glUseProgram(0);
}

void InitOpenGL() {
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
	cout << "Using freeglut version: " << ver / 10000 << "." << (ver / 100) % 100 << "." << ver % 100 << endl;
	
   //create vertex buffers for vertex coords, normals, and tex coords
   glGenBuffers(1, &VertexBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
   glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * NumVertices, Vertices, GL_STATIC_DRAW);

   glGenBuffers(1, &NormalBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, NormalBuffer);
   glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * NumVertices, Normals, GL_STATIC_DRAW);

   glGenBuffers(1, &TexCoordBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, TexCoordBuffer);
   glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float) * NumVertices, TexCoords, GL_STATIC_DRAW);

   //create index buffer for mesh indices
   glGenBuffers(1, &IndexBuffer);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(unsigned int) * NumFaces, Faces, GL_STATIC_DRAW);

   //Frame buffer object handler
   fbo_handler_ptr = new fbo::FBOHandler(256, 256);
   fbo_handler_ptr->render(texture_animation, 0.0);

   //create and load shaders
   open_gl_program_ptr = new shaders::OpenGLProgram("shaders/PhongVertex.vert", "shaders/PhongFragment.frag");
   //open_gl_program_ptr = new shaders::ShaderHandler("shaders/CookTorranceVertex.vert", "shaders/CookTorranceFragment.frag");

   //Because we are gonna set some variables to shaders
   open_gl_program_ptr->use_program();
   VM_loc = open_gl_program_ptr->get_uniform_location("VM");
   P_loc = open_gl_program_ptr->get_uniform_location("P");
   Time_loc = open_gl_program_ptr->get_uniform_location("time");
   view_vector_loc = open_gl_program_ptr->get_uniform_location("view");
   light_position_in_vs_loc = open_gl_program_ptr->get_uniform_location("light_position_in_vs");
   light_direction_in_vs_loc = open_gl_program_ptr->get_uniform_location("light_direction_in_vs");

   //Make a location in the shader for program state variables
   fish_animation_loc = open_gl_program_ptr->get_uniform_location("fish_animation");
   texture_map_flag_loc = open_gl_program_ptr->get_uniform_location("texture_map_flag");
   light_mode_flag_loc = open_gl_program_ptr->get_uniform_location("light_mode");
   rotate_animation_loc = open_gl_program_ptr->get_uniform_location("rotate_animation");
   
   //set texture unit for uniform sampler variable
   tex_loc = open_gl_program_ptr->get_uniform_location("texture_map");
   if(tex_loc != -1) {
      glUniform1i(tex_loc, 0);
   }
   glUseProgram(0);
   //state for the main window
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   glEnable(GL_DEPTH_TEST);
}

void idle() {
	static int last_time = 0;

	int time = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = time - last_time;
	last_time = time;
	float delta_seconds = 0.001f * elapsed;	
	seconds += delta_seconds;

	//set shader time uniform variable
	open_gl_program_ptr->use_program();

   if(Time_loc != -1) {
	   glUniform1f(Time_loc, seconds);
   }

	glutPostRedisplay();
}
	
void CreateGlutWindow() {
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition (5, 5);
	glutInitWindowSize (640, 640);
	win = glutCreateWindow ("Jorge Garcia, Project 5");
}

void CreateGlutCallbacks() {
	glutDisplayFunc(display);
	glutSpecialFunc(special_keys);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutMouseWheelFunc(mouse_wheel);
	glutMouseFunc(mouse);
	glutMotionFunc(mouse_active);
}

void ExitGlut() {
	if (fbo_handler_ptr) {
		delete fbo_handler_ptr;
		fbo_handler_ptr = nullptr;
	}
	if (open_gl_program_ptr) {
		delete open_gl_program_ptr;
		open_gl_program_ptr = nullptr;
	}
	glutDestroyWindow(win);
	exit(EXIT_SUCCESS);
}

void init_program() {
	texture_map_flag = true;
	rotate_animation = false;
	texture_animation = true;
	seconds = 0.0;
	fovy = TAU / 15.0f;
	base_rotation_angles = glm::vec2(0.0f, 0.0f);
	new_rotation_angles = glm::vec2(0.0f, 0.0f);
	base_pan = glm::vec2(0.0f, 0.0f);
	new_pan = glm::vec2(0.0f, 0.0f);
	mode = UNUSED;
	window_width = window_height = 640;
}

int main (int argc, char* argv[]) {
	glutInit(&argc, argv); 
	CreateGlutWindow();
	CreateGlutCallbacks();
	InitOpenGL();
	init_program();
	glutMainLoop();

	ExitGlut();
	return EXIT_SUCCESS;		
}

void special_keys(int key, int mouse_x, int mouse_y) {
	switch (key) {
		case GLUT_KEY_F3:
			fish_animation = (!fish_animation);
		break;

		case GLUT_KEY_F4:
			texture_map_flag = (!texture_map_flag);
		break;

		case GLUT_KEY_F5:
			light_mode_flag = (!light_mode_flag);
		break;

		case GLUT_KEY_F2:
			//rotate_animation = (!rotate_animation);
            rotate_animation = true;
			texture_animation = false;
		break;

		case GLUT_KEY_F1:
			//texture_animation = (!texture_animation);
			rotate_animation = false;
			texture_animation = true;
		break;
	}
	glutPostRedisplay();
}

void keyboard (unsigned char key, int mouse_x, int mouse_y) {
	switch (key) {

		case 'R':
		case 'r':
			base_rotation_angles = glm::vec2(0.0f, 0.0f);
			base_pan = glm::vec2(0.0f, 0.0f);
			fovy = TAU / 15.0f;
		break;

	    case 27:
			exit(EXIT_SUCCESS);
		break;

		default:
		break;
	}

	glutPostRedisplay();
}
