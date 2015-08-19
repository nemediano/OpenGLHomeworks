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
#include "OpenGLProgram.h"


using namespace std;
using namespace shaders;

OpenGLProgram* gl_program_ptr = nullptr;

static int win = 0;

//shader uniform locations
int M_loc = -1;
int PV_loc = -1;
int Time_loc = -1;
int Color_loc = -1;

//shader attrib locations
int pos_loc = -1;
int tex_coord_loc = -1;

bool pauseRotation = false;
float rev_per_min = 0.0f;
int mode = 0;


//The function for drawing each object gets passed the modeling transformation of its parent. Each object should compute its 
//own transformation relative to its parent, and pass that matrix to any children. In this hierarchy the sun is at the root of
//the tree. The sun's children are the planets. The moons are children of the planet they orbit around.

// planet : Mercury
// color : magenta
// radius : 0.5
// orbital rate : 4*rev_per_min
// orbital radius : 5.0
void drawMercury(glm::mat4 M) {
	glm::mat4 I(1.0f);
	glm::mat4 Model = M * glm::rotate(I, 4.0f * rev_per_min, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::translate(I, glm::vec3(5.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(M_loc, 1, GL_FALSE, glm::value_ptr(Model));
	glUniform4f(Color_loc, 1.0f, 0.0f, 1.0f, 1.0f);
	glutSolidSphere(0.5, 10, 10);
}

// planet : Venus
// color : green
// radius : 1.0
// orbital rate : 3*rev_per_min
// orbital radius : 10.0
void drawVenus(glm::mat4 M) {
   glm::mat4 I(1.0f);
   glm::mat4 Model = M * glm::rotate(I, 3.0f * rev_per_min, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::translate(I, glm::vec3(10.0f, 0.0f, 0.0f));
   glUniformMatrix4fv(M_loc, 1, GL_FALSE, glm::value_ptr(Model));
   glUniform4f(Color_loc, 0.0f, 1.0f, 0.0f, 1.0f);
   glutSolidSphere(1.0, 10, 10);
}

// moon : Earth's moon
// color : white
// radius : 0.25
// orbital rate : 2*rev_per_min
// orbital radius : 2.0
void drawMoon(glm::mat4 M) {
   glm::mat4 I(1.0f);
   glm::mat4 Model = M * glm::rotate(I, 2.0f * rev_per_min, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::translate(I, glm::vec3(2.0f, 0.0f, 0.0f));
   glUniformMatrix4fv(M_loc, 1, GL_FALSE, glm::value_ptr(Model));
   glUniform4f(Color_loc, 1.0f, 1.0f, 1.0f, 1.0f);
   glutSolidSphere(0.25, 10, 10);
}

// planet : Earth
// color : blue
// radius : 1.0
// orbital rate : 1*rev_per_min
// orbital radius : 15.0
void drawEarth(glm::mat4 M) {
   glm::mat4 I(1.0f);
   glm::mat4 Model = M * glm::rotate(I, 1.0f * rev_per_min, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::translate(I, glm::vec3(15.0f, 0.0f, 0.0f));
   glUniformMatrix4fv(M_loc, 1, GL_FALSE, glm::value_ptr(Model));
   glUniform4f(Color_loc, 0.0f, 0.0f, 1.0f, 1.0f);
   glutSolidSphere(1.0, 10, 10);
   drawMoon(Model);
}

// moon : Mars' moon #1
// color : white
// radius : 0.25
// orbital rate : 3*rev_per_min
// orbital radius : 1.5
void drawMarsMoon1(glm::mat4 M) {
   glm::mat4 I(1.0f);
   glm::mat4 Model = M * glm::rotate(I, 3.0f * rev_per_min, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::translate(I, glm::vec3(1.5, 0.0, 0.0));
   glUniformMatrix4fv(M_loc, 1, GL_FALSE, glm::value_ptr(Model));
   glUniform4f(Color_loc, 1.0f, 1.0f, 1.0f, 1.0f);
   glutSolidSphere(0.25, 10, 10);
}

// moon : Mars' moon #2
// color : white
// radius : 0.25
// orbital rate : 4*rev_per_min
// orbital radius : 2.0
void drawMarsMoon2(glm::mat4 M) {
	glm::mat4 I(1.0f);
	glm::mat4 Model = M * glm::rotate(I, 4.0f * rev_per_min, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::translate(I, glm::vec3(2.0, 0.0, 0.0));
	glUniformMatrix4fv(M_loc, 1, GL_FALSE, glm::value_ptr(Model));
	glUniform4f(Color_loc, 1.0f, 1.0f, 1.0f, 1.0f);
	glutSolidSphere(0.25, 10, 10);
}

// planet : Mars
// color : red
// radius : 1.0
// orbital rate : 0.5*rev_per_min
// orbital radius : 20.0
void drawMars(glm::mat4 M) {
	glm::mat4 I(1.0f);
	glm::mat4 Model = M * glm::rotate(I, 0.5f * rev_per_min, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::translate(I, glm::vec3(20.0, 0.0, 0.0));
	glUniformMatrix4fv(M_loc, 1, GL_FALSE, glm::value_ptr(Model));
	glUniform4f(Color_loc, 1.0f, 0.0f, 0.0f, 1.0f);
	glutSolidSphere(1.0, 10, 10);
	drawMarsMoon1(Model);
	drawMarsMoon2(Model);
}

// Sun
// color : yellow
// radius : 2.0
// fixed at origin
void drawSun(glm::mat4 M) {
   glm::mat4 Model = M;
   glUniformMatrix4fv(M_loc, 1, GL_FALSE, glm::value_ptr(Model));
   glUniform4f(Color_loc, 1.0f, 1.0f, 0.0f, 1.0f);
   glutSolidSphere(2.0, 10, 10);
   drawMars(Model);
   drawEarth(Model);
   drawVenus(Model);
   drawMercury(Model);
}

void drawSolarSystem() {
	glm::mat4 I(1.0f);
	glm::mat4 M = glm::translate(I, glm::vec3(0.0f, 0.0f, 0.0f));
	drawSun(M);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 I(1.0);

	glm::mat4 M = glm::scale(I, glm::vec3(1.0f, 1.0f, 1.0f));
	glm::mat4 V = glm::lookAt(glm::vec3(20.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 P = glm::perspective(90.0f, 1.0f, 0.1f, 100.0f);

	gl_program_ptr->use_program();

	if(M_loc != -1) {
		glUniformMatrix4fv(M_loc, 1, GL_FALSE, glm::value_ptr(M));
	}

	if(PV_loc != -1) {
		glUniformMatrix4fv(PV_loc, 1, GL_FALSE, glm::value_ptr(P * V));
	}

	if(Color_loc != -1) {
		glUniform4f(Color_loc, 0.5f, 0.5f, 0.0f, 1.0f);
	}

	drawSolarSystem();
  
	glutSwapBuffers();

	gl_error("At the end of display");
}

void load_Shader() {
	if (gl_program_ptr != nullptr) {
		delete gl_program_ptr;
	}

	//create and load shaders
	gl_program_ptr = new OpenGLProgram("vshader.glsl", "fshader.glsl");
	if (!gl_program_ptr->is_ok()) {
		cerr << "Error at shader creation" << endl;
		exit(EXIT_FAILURE);
	}

	pos_loc = gl_program_ptr->get_attrib_location("pos_attrib");
	tex_coord_loc = gl_program_ptr->get_attrib_location("tex_coord_attrib");

	M_loc = gl_program_ptr->get_uniform_location("M");
	PV_loc = gl_program_ptr->get_uniform_location("PV");
	Time_loc = gl_program_ptr->get_uniform_location("time");
	Color_loc = gl_program_ptr->get_uniform_location("uColor");

	gl_error("At setting up the shaders");
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
	cout << "Using freeglut version: " << ver / 10000 << "." << (ver / 100) % 100 << "." << ver % 100 << endl;
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	
	load_Shader();

	gl_error("At init OpenGL");
}

void idle() {
	static int last_time = 0;
	static float seconds = 0.0f;

	int time = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = time-last_time;
	last_time = time;
	float delta_seconds = 0.001f*elapsed;
	
	if(pauseRotation == false) {
		seconds += delta_seconds;
		rev_per_min = 6.0f*seconds;
	}

	//set shader time uniform variable
	gl_program_ptr->use_program();

	if(Time_loc != -1) {
		glUniform1f(Time_loc, seconds);
	}

	glutPostRedisplay();
}

void exit_glut() {
	glutDestroyWindow(win);
	exit(0);
}

void keyboard(unsigned char key, int x, int y) {
	
	if (key == 27) {//press ESC to exit
		exit_glut();
	}  

	if(key == 'r' || key == 'R') {
		load_Shader();
	}
}



void special(int key, int x, int y) {

   if(key == GLUT_KEY_F1) {
	   pauseRotation = !pauseRotation;
   }

}

void create_glut_window() {
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(5, 5);
	glutInitWindowSize(640, 640);
	win = glutCreateWindow("CGT 520 Solar System Hierarchy");
}

void create_glut_callbacks() {
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
}

int main (int argc, char* argv[]) {
	glutInit(&argc, argv); 
	create_glut_window();
	create_glut_callbacks();
	init_OpenGL();

	glutMainLoop();

	exit_glut();
	return 0;		
}

