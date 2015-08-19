#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>

//This is a bug on GLM in 32 bits I hope they solve it soon
#define GLM_FORCE_PURE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include <iostream>
#include <cstdlib>
#include "OpenGLProgram.h"
#include "mesh.h"
#include "texture.h"

using namespace std;
using namespace shaders;

static int win = 0;
bool fish_animation;
bool texture_map_flag;
bool texture_animation;

//OpenGL shader object
OpenGLProgram* pProgram = nullptr;

int VM_loc = -1;
int P_loc = -1;
int Time_loc = -1;
int fish_animation_loc = -1;
int texture_map_flag_loc = -1;
int texture_animation_loc = -1;

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

void display() {
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glm::mat4 I(1.0f);
   glm::mat4 M = glm::scale(I, glm::vec3(6.0f, 6.0f, 6.0f));
   glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
   glm::mat4 P = glm::perspective(45.0f, 1.0f, 0.1f, 100.0f);

   pProgram->use_program();
   
   //Pass program state variables to shader
   if(fish_animation_loc != -1) {
	   glUniform1i(fish_animation_loc, static_cast<int>(fish_animation));
   }
   if(texture_animation_loc != -1) {
	   glUniform1i(texture_animation_loc, static_cast<int>(texture_animation));
   }
   if(texture_map_flag_loc != -1) {
	   glUniform1i(texture_map_flag_loc, static_cast<int>(texture_map_flag));
   }


   if(VM_loc != -1) {
	   glUniformMatrix4fv(VM_loc, 1, GL_FALSE, glm::value_ptr(V * M));
   }

   if(P_loc != -1) {
      glUniformMatrix4fv(P_loc, 1, GL_FALSE, glm::value_ptr(P));
   }

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureID);

	int pos_loc = pProgram->get_attrib_location("pos_attrib");
	int normal_loc = pProgram->get_attrib_location("normal_attrib");
	int tex_coord_loc = pProgram->get_attrib_location("tex_coord_attrib");
	
	glEnableVertexAttribArray(pos_loc);
	glEnableVertexAttribArray(normal_loc);
	glEnableVertexAttribArray(tex_coord_loc);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, NormalBuffer);
	glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, TexCoordBuffer);
	glVertexAttribPointer(tex_coord_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);

	//render indexed primitives
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
	glDrawElements(GL_TRIANGLES, 3 * NumFaces, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(pos_loc);
	glDisableVertexAttribArray(normal_loc);
	glDisableVertexAttribArray(tex_coord_loc);

	glutSwapBuffers();
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

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	
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

	//create and load texture
	glGenTextures(1, &TextureID);
	glBindTexture(GL_TEXTURE_2D, TextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//create and load shaders
	pProgram = new OpenGLProgram("vertexShader.glsl", "fragmentShader.glsl");

	if (!pProgram->is_ok()) {
		cerr << "Error at GL program creation" << endl;
		gl_error();
		exit(EXIT_FAILURE);
	}

	pProgram->use_program();

	VM_loc = pProgram->get_uniform_location("VM");
	P_loc = pProgram->get_uniform_location("P");
	Time_loc = pProgram->get_uniform_location("time");

	//Make a location in the shader for program state variables
	fish_animation_loc = pProgram->get_uniform_location("fish_animation");
	texture_map_flag_loc = pProgram->get_uniform_location("texture_map_flag");
	texture_animation_loc = pProgram->get_uniform_location("texture_animation");

	//set texture unit for uniform sampler variable
	int tex_loc = pProgram->get_uniform_location("texture_map");
	if(tex_loc != -1) {
		glUniform1i(tex_loc, 0);
	}
	glUseProgram(0);
}

void idle() {
	static int last_time = 0;
	static float seconds = 0.0f;

	int time = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = time - last_time;
	last_time = time;
	float delta_seconds = 0.001f * elapsed;	
	seconds += delta_seconds;

	//set shader time uniform variable
	pProgram->use_program();

   if(Time_loc != -1) {
	   glUniform1f(Time_loc, seconds);
   }

	glutPostRedisplay();
}
	
void CreateGlutWindow() {
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition (5, 5);
	glutInitWindowSize (640, 640);
	win = glutCreateWindow ("Jorge Garcia, Project 3");
}

void CreateGlutCallbacks() {
	glutDisplayFunc(display);
	glutSpecialFunc(special_keys);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
}

void ExitGlut() {
	glutDestroyWindow(win);
	exit(EXIT_SUCCESS);
}

void init_program() {
	fish_animation = false;
	texture_map_flag = false;
	texture_animation = false;
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
		case GLUT_KEY_F1:
			fish_animation = (!fish_animation);
		break;

		case GLUT_KEY_F2:
			texture_map_flag = (!texture_map_flag);
		break;

		case GLUT_KEY_F3:
			texture_animation = (!texture_animation);
			if (texture_animation) {
				texture_map_flag = true;
			}
		break;
	}
	glutPostRedisplay();
}

void keyboard (unsigned char key, int mouse_x, int mouse_y) {
	switch (key) {
	    case 27:
			exit(EXIT_SUCCESS);
		break;

		default:
		break;
	}

	glutPostRedisplay();
}