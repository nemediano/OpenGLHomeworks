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

using namespace std;
using namespace shaders;

static int win = 0;

//opengl program object
OpenGLProgram* program_ptr = nullptr;

//shader uniform locations
int VM_loc = -1;
int P_loc = -1;
int Time_loc = -1;
int Color_loc = -1;

//shader attrib locations
int pos_loc = -1;
int tex_coord_loc = -1;

//vertex array object (holds bindings and offsets for VBO)
GLuint VAO = -1;

//opengl vertex buffer objects
GLuint VertexBuffer = 0;
GLuint TexCoordBuffer = 0;


void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 I(1.0f);
	glm::mat4 M = glm::scale(I, glm::vec3(1.0f, 1.0f, 1.0f));
	glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 P = glm::ortho(-1.0f, +1.0f, -1.0f, +1.0f, +1.0f, -1.0f);

	for(int i=0; i<2; i++)
	{
		program_ptr->use_program();

		if(VM_loc != -1)
		{
			glUniformMatrix4fv(VM_loc, 1, GL_FALSE, glm::value_ptr(V * M));
		}

		if(P_loc != -1)
		{
			glUniformMatrix4fv(P_loc, 1, GL_FALSE, glm::value_ptr(P));
		}

		if(Color_loc != -1)
		{
			glUniform4f(Color_loc, 1.0f, 0.5f, 0.5f, 1.0f);
		}
	}

	glBindVertexArray(VAO);
   //draw ground plane
	program_ptr->use_program(); //untextured
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glutSwapBuffers();
	gl_error("At display function");
}

void load_shader()
{
	if(program_ptr != nullptr)
	{
		delete program_ptr;
	}

    //create and load shaders
	program_ptr = new OpenGLProgram("vshader.glsl", "fshader.glsl");
	if (!program_ptr->is_ok()) {
		cerr << "At shader compilation" << endl;
		exit(EXIT_FAILURE);
	}

	program_ptr->use_program();

    pos_loc = program_ptr->get_attrib_location("pos_attrib");
	tex_coord_loc = program_ptr->get_attrib_location("tex_coord_attrib");

    VM_loc = program_ptr->get_uniform_location("VM");
	P_loc = program_ptr->get_uniform_location("P");
	Time_loc = program_ptr->get_uniform_location("time");
	Color_loc = program_ptr->get_uniform_location("uColor");

    gl_error("At shader loading");
  
}

void init_OpenGL()
{
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
	
	load_shader();

	//create vertex array object for saving VBO settings
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	float vertices[] = {1.0f, 1.0f, 0.0f, 1.0f, -1.0f, 0.0f, -1.0f, 1.0f, 0.0f, -1.0f, -1.0f, 0.0f};
	float texcoords[] = {1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};

	//create vertex buffers for vertex coords, normals, and tex coords
	glGenBuffers(1, &VertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &TexCoordBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, TexCoordBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);

	if(pos_loc >= 0)
	{
		glEnableVertexAttribArray(pos_loc);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
		glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 0, 0);
	}

	if(tex_coord_loc >= 0)
	{
		glEnableVertexAttribArray(tex_coord_loc);
		glBindBuffer(GL_ARRAY_BUFFER, TexCoordBuffer);
		glVertexAttribPointer(tex_coord_loc, 2, GL_FLOAT, false, 0, 0);
	}

	gl_error("At end of the OpenGL init");
}

void idle()
{
	static int last_time = 0;
	static float seconds = 0.0f;

	int time = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = time-last_time;
	last_time = time;
	float delta_seconds = 0.001f*elapsed;
	seconds += delta_seconds;
	
	//set shader time uniform variable
	program_ptr->use_program();

	if(Time_loc != -1)
	{
		glUniform1f(Time_loc, seconds);
	}
  
	glutPostRedisplay();
}

void exit_glut()
{
	glutDestroyWindow(win);
	exit(EXIT_SUCCESS);
}

void keyboard(unsigned char key, int x, int y)
{
   if(key == 27) //press ESC to exit
   {
	   exit_glut();
   }  
   if(key == 'r' || key == 'R')
   {
	   load_shader();
   }
}

void CreateGlutWindow()
{
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition (5, 5);
	glutInitWindowSize (640, 640);
	win = glutCreateWindow ("Noise");
}

void CreateGlutCallbacks()
{
	glutDisplayFunc(display);
	glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
}

int main (int argc, char **argv)
{
	glutInit(&argc, argv); 
	CreateGlutWindow();
	CreateGlutCallbacks();
	init_OpenGL();

	glutMainLoop();

	exit_glut();
	return EXIT_SUCCESS;
}

