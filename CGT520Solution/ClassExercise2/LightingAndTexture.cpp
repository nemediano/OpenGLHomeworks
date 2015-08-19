#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>

//This is a bug on GLM in 32 bits I hope they solve it soon
#define GLM_FORCE_PURE
#define GLM_MESSAGES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include "mesh.h"
#include "texture.h"

#include <cstdlib>
#include <cmath>
#include <iostream>

using namespace std;
static int win = 0;

#include "OpenGLProgram.h"
using namespace shaders;

OpenGLProgram* gl_program_ptr = nullptr;

//shader uniform locations
int VM_loc = -1;
int P_loc = -1;
int Time_loc = -1;
int Light_loc = -1;

//shader attrib locations
int pos_loc = -1;
int normal_loc = -1;
int tex_coord_loc = -1;
int texture_option_loc = -1;

bool per_fragment = true;
bool use_texture = false;

//vertex array object (holds bindings and offsets for VBO)
GLuint VAO = -1;

//opengl vertex buffer objects
GLuint VertexBuffer = 0;
GLuint NormalBuffer = 0;
GLuint TexCoordBuffer = 0;
GLuint IndexBuffer = 0;

//opengl texture object
GLuint TextureID = 0;

float angle = 0.0f;

void display();
void load_shader();
void init_OpenGL();
void idle();
void exit_glut();
void keyboard(unsigned char key, int x, int y);
void idle();
void create_glut_window();
void create_glut_callbacks();

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 I = glm::mat4(1.0);
	glm::mat4 M = glm::scale(I, glm::vec3(2.0f, 2.0f, 2.0f));
	glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 P = glm::perspective(90.0f, 1.0f, 0.1f, 100.0f);

	gl_program_ptr->use_program();

   if(VM_loc != -1) {
	   glUniformMatrix4fv(VM_loc, 1, GL_FALSE,  glm::value_ptr(V * M));
   }

   if(P_loc != -1) {
      glUniformMatrix4fv(P_loc, 1, GL_FALSE,  glm::value_ptr(P));
   }

   glUniform3f(Light_loc, 0.0f, cos(angle), sin(angle));
   glUniform1i(texture_option_loc, use_texture ? 1 : 0);


   //We assigned 0 to the "texture" shader uniform variable, so use tex unit 0.
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureID);

   glBindVertexArray(VAO);
   //render indexed primitives
	glDrawElements(GL_TRIANGLES, 3 * NumFaces, GL_UNSIGNED_INT, 0);

	glutSwapBuffers();
	gl_error("At display");
}

void load_shader() {
	if (gl_program_ptr != nullptr) {
		delete gl_program_ptr;
	}

   //create and load shaders
	if (per_fragment) {
		gl_program_ptr = new OpenGLProgram("phongvshader.glsl", "phongfshader.glsl");
	} else {
		gl_program_ptr = new OpenGLProgram("phongGouraudvshader.glsl", "phongGouraudfshader.glsl");
	}
	
	gl_program_ptr->use_program();

	pos_loc = gl_program_ptr->get_attrib_location("pos_attrib");
	normal_loc = gl_program_ptr->get_attrib_location("normal_attrib");
	tex_coord_loc = gl_program_ptr->get_attrib_location("tex_coord_attrib");

	VM_loc = gl_program_ptr->get_uniform_location("VM");
	P_loc = gl_program_ptr->get_uniform_location("P");
	Time_loc = gl_program_ptr->get_uniform_location("time");
	Light_loc = gl_program_ptr->get_uniform_location("light");

   //set texture unit for uniform sampler variable
	int tex_loc = gl_program_ptr->get_uniform_location("texture_map");
	texture_option_loc = gl_program_ptr->get_uniform_location("texture_option");
   if(tex_loc != -1) {
      glUniform1i(tex_loc, 0); // we will bind our texture to texture unit 0
   }
   gl_error("At shader loading");
}


void init_OpenGL() {
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
	cout << "Using freeglut version: " <<  ver / 10000 << "." << (ver / 100) % 100 << "." << ver % 100 << endl;
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	
	load_shader();

   //create vertex array object for saving VBO settings
   glGenVertexArrays(1, &VAO);
   glBindVertexArray(VAO);

   //create vertex buffers for vertex coords, normals, and tex coords
   glGenBuffers(1, &VertexBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
   glBufferData(GL_ARRAY_BUFFER, 3 * 4 * NumVertices, Vertices, GL_STATIC_DRAW);

   glGenBuffers(1, &NormalBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, NormalBuffer);
   glBufferData(GL_ARRAY_BUFFER, 3 * 4 * NumVertices, Normals, GL_STATIC_DRAW);

   glGenBuffers(1, &TexCoordBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, TexCoordBuffer);
   glBufferData(GL_ARRAY_BUFFER, 2 * 4 * NumVertices, TexCoords, GL_STATIC_DRAW);

   //create index buffer for mesh indices
   glGenBuffers(1, &IndexBuffer);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * 4 * NumFaces, Faces, GL_STATIC_DRAW);

   if(pos_loc >= 0) {
      glEnableVertexAttribArray(pos_loc);
	   glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	   glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 0, 0);
   }

   if(normal_loc >= 0) {
      glEnableVertexAttribArray(normal_loc);
	   glBindBuffer(GL_ARRAY_BUFFER, NormalBuffer);
	   glVertexAttribPointer(normal_loc, 3, GL_FLOAT, false, 0, 0);
   }

   if(tex_coord_loc >= 0) {
      glEnableVertexAttribArray(tex_coord_loc);
	   glBindBuffer(GL_ARRAY_BUFFER, TexCoordBuffer);
	   glVertexAttribPointer(tex_coord_loc, 2, GL_FLOAT, false, 0, 0);
   }
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);

   gl_error("At creating the OpenGL environment");

   //create and load texture
   glGenTextures(1, &TextureID);
   glBindTexture(GL_TEXTURE_2D, TextureID);
   //Width, Height, Image are declared in texture.h
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Image);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void idle() {
	static int last_time = 0;
	static float seconds = 0.0f;

	int time = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = time - last_time;
	last_time = time;
	float delta_seconds = 0.001f * elapsed;	
	seconds += delta_seconds;
	angle = 1.5f * seconds;
	//set shader time uniform variable
	gl_program_ptr->use_program();

	if(Time_loc != -1) {
		glUniform1f(Time_loc, seconds);
	}

	glutPostRedisplay();
}

void exit_glut() {
	glutDestroyWindow(win);
	exit(EXIT_SUCCESS);
}

void keyboard(unsigned char key, int x, int y) {
   if(key == 27) { //press ESC to exit
	   exit_glut();
   }

   if(key == 'r' || key == 'R') {
	   load_shader();
   }

   if(key == 'l' || key == 'L') {
	   per_fragment = !per_fragment;
	   load_shader();
   }

   if(key == 't' || key == 'T') {
	   use_texture = !use_texture;
   }
}

void create_glut_window() {
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition (5, 5);
	glutInitWindowSize (640, 640);
	win = glutCreateWindow ("CGT 520 Light and Texture Demo");
}

void create_glut_callbacks() {
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
}

int main (int argc, char* argv[]) {
	glutInit(&argc, argv); 
	create_glut_window();
	create_glut_callbacks();
	init_OpenGL();

	glutMainLoop();

	exit_glut();

	return EXIT_SUCCESS;		
}

