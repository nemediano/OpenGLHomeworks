#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GL/freeglut.h>

#define GLM_FORCE_PURE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/*Headers needed for imgui */
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glut.h"

/* Include heders and namespaces from my CGT Library */
#include "OGLHelpers.h"
#include "MathConstants.h"
#include "Texture.h"
#include "Mesh.h"
#include "Camera.h"
#include "Trackball.h"
#include "OGLProgram.h"
#include "Spotlight.h"
#include "Material.h"
using namespace ogl;
using namespace math;
using namespace image;
using namespace mesh;
using namespace camera;
using namespace ogl;
using namespace lighting;
/* CGT Library related*/
Camera cam;
Trackball ball;
Texture* textureMapPtr = nullptr;
Mesh* meshPtr = nullptr;
OGLProgram* programPtr = nullptr;
Material mat;
Spotlight light;
GLint window = 0;
// Location for shader variables
GLint u_PVM_location = -1;
GLint u_texture_location = -1;
GLint a_position_loc = -1;
GLint a_normal_loc = -1;
GLint a_texture_loc = -1;

//Lighting related variables
//Light
GLint u_LightCol_loc = -1;
GLint u_LightInt_loc = -1;
GLint u_LightApt_loc = -1;
GLint u_LightPos_loc = -1;
GLint u_LightDir_loc = -1;

//Material
GLint u_MatMetal_loc = -1;
GLint u_MatRough_loc = -1;
GLint u_F0_loc = -1;

void allocateLighting();
void passLightingState();

//Global variables for the program logic
bool rotate;
float seconds_elapsed;
float angle;
float scaleFactor;
glm::vec3 center;

void create_glut_window();
void init_program();
void init_OpenGL();
void create_glut_callbacks();
void exit_glut();

//Glut callback functions
void display();
void reshape(int new_window_width, int new_window_height);
void idle();
/*I am forced to implement all this callbacks, for the GUI to work propertly*/
void keyboard(unsigned char key, int mouse_x, int mouse_y);
void special(int key, int mouse_x, int mouse_y);
void mouse(int button, int state, int x, int y);
void mouseWheel(int button, int dir, int mouse_x, int mouse_y);
void mouseDrag(int mouse_x, int mouse_y);
void mousePasiveMotion(int mouse_x, int mouse_y);

//Imgui related function
void drawGUI();

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);

	create_glut_window();
	init_OpenGL();
	/*You need to call this once at the begining of your program for ImGui to work*/
	ImGui_ImplGLUT_Init();
	init_program();

	create_glut_callbacks();
	glutMainLoop();

	return EXIT_SUCCESS;
}

/*
This is the creation of the GUI.
Since is inmediate mode this need to be called at the end of display
*/

void drawGUI() {
	/* Always start with this call*/
	ImGui_ImplGLUT_NewFrame(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	/* Position of the menu, if no imgui.ini exist */
	ImGui::SetNextWindowSize(ImVec2(50, 50), ImGuiSetCond_FirstUseEver);

	/*Create a new menu for my app*/
	ImGui::Begin("My options");
	ImGui::Checkbox("Rotation", &rotate);
	if (ImGui::Button("Quit")) {
		exit_glut();
	}
	if (ImGui::Button("Reset camera")) {
		cam.setFovY(PI / 8.0f);
		ball.resetRotation();
	}
	ImGui::End();

	/* End with this when you want to render GUI */
	ImGui::Render();
}

void exit_glut() {
	delete textureMapPtr;
	delete meshPtr;
	delete programPtr;
	/* Shut down the gui */
	ImGui_ImplGLUT_Shutdown();
	/* Delete window (freeglut) */
	glutDestroyWindow(window);
	exit(EXIT_SUCCESS);
}

void create_glut_window() {
	//Set number of samples per pixel
	glutSetOption(GLUT_MULTISAMPLE, 8);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(800, 600);
	window = glutCreateWindow("Simple example: using framework");
}

void init_program() {
	using glm::vec3;
	/* Initialize global variables for program control */
	rotate = false;
	/* Then, create primitives (load them from mesh) */
	meshPtr = new Mesh("../models/Tiger.obj");
	if (meshPtr) {
		meshPtr->sendToGPU();
	}
	//Extract info form the mesh
	scaleFactor = meshPtr->scaleFactor();
	center = meshPtr->getBBCenter();

	textureMapPtr = new Texture("../models/bengal_tiger.jpg");
	seconds_elapsed = 0.0f;
	angle = 0.0f;
	//Set the default position of the camera
	cam.setLookAt(vec3(0.0f, 0.0f, 1.5f), vec3(0.0f));
	cam.setAspectRatio(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	cam.setFovY(PI / 4.0f);
	cam.setDepthView(0.1f, 3.0f);
	//Create trackball camera
	ball.setWindowSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	//Default position of the spotlight
	light.setPosition(vec3(0.0f, 0.0f, 2.0f));
	light.setTarget(vec3(0.0f));
	light.setAperture(10.0f * TO_RADIANS);
	//Default material
	mat.setF0(0.5f);
	mat.setMetalicity(0.5f);
	mat.setRoughness(0.2f);
	mat.setBaseColor(vec3(0.5f, 1.0f, 0.25f));
}

void init_OpenGL() {
	using std::cout;
	using std::cerr;
	using std::endl;
	/************************************************************************/
	/*                    Init OpenGL context   info                        */
	/************************************************************************/
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		cerr << "Error: " << glewGetErrorString(err) << endl;
	}
	cout << getOpenGLInfo() << endl;

	/************************************************************************/
	/*                   OpenGL program creation                            */
	/************************************************************************/
	//programPtr = new OGLProgram("shaders/simpleVertex.vert", "shaders/simpleFragment.frag");
	programPtr = new OGLProgram("shaders/disneyVertex.vert", "shaders/disneyFragment.frag");
	if (!programPtr || !programPtr->isOK()) {
		cerr << "Something wrong in shader" << endl;
	}


	/************************************************************************/
	/* Allocating variables for shaders                                     */
	/************************************************************************/

	u_PVM_location = programPtr->uniformLoc("PVM");
	u_texture_location = programPtr->uniformLoc("texture_image");
	allocateLighting();

	a_position_loc = programPtr->attribLoc("Position");
	a_normal_loc = programPtr->attribLoc("Normal");
	a_texture_loc = programPtr->attribLoc("TextCoord");

	//Activate antialliasing
	glEnable(GL_MULTISAMPLE);

	//Initialize some basic rendering state
	glEnable(GL_DEPTH_TEST);
	//Dark gray background color
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

}

void allocateLighting() {
	//Light
	u_LightCol_loc = programPtr->uniformLoc("lght.color");
	u_LightInt_loc = programPtr->uniformLoc("lght.intensity");
	u_LightApt_loc = programPtr->uniformLoc("lght.aperture");
	u_LightPos_loc = programPtr->uniformLoc("lght.position");
	u_LightDir_loc = programPtr->uniformLoc("lght.direction");
	//Material
	u_MatMetal_loc = programPtr->uniformLoc("mat.metalicity");
	u_MatRough_loc = programPtr->uniformLoc("mat.roughness");
	u_F0_loc = programPtr->uniformLoc("mat.F0");
}

void passLightingState() {
	//Light
	if (u_LightCol_loc != -1) {
		glUniform3fv(u_LightCol_loc, 1, glm::value_ptr(light.getColor()));
	}
	if (u_LightInt_loc != -1) {
		glUniform1f(u_LightInt_loc, light.getIntensity());
	}
	if (u_LightApt_loc != -1) {
		glUniform1f(u_LightApt_loc, light.getAperture());
	}
	if (u_LightPos_loc != -1) {
		glUniform3fv(u_LightPos_loc, 1, glm::value_ptr(light.getPosition()));
	}
	if (u_LightDir_loc != -1) {
		glUniform3fv(u_LightDir_loc, 1, glm::value_ptr(light.getDirection()));
	}
	
	//Material
	if (u_MatMetal_loc != -1) {
		glUniform1f(u_MatMetal_loc, mat.getMetalicity());
	}
	if (u_MatRough_loc != -1) {
		glUniform1f(u_MatRough_loc, mat.getRoughness());
	}
	if (u_F0_loc != -1) {
		glUniform3fv(u_F0_loc, 1, glm::value_ptr(mat.getF0()));
	}
}


void create_glut_callbacks() {
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutMouseWheelFunc(mouseWheel);
	glutMouseFunc(mouse);
	glutSpecialFunc(special);
	glutMotionFunc(mouseDrag);
	glutPassiveMotionFunc(mousePasiveMotion);
}

void reshape(int new_window_width, int new_window_height) {
	glViewport(0, 0, new_window_width, new_window_height);
	cam.setAspectRatio(new_window_width, new_window_height);
	ball.setWindowSize(new_window_width, new_window_height);
}

void display() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	programPtr->use();

	glm::mat4 I(1.0f);
	//Model
	glm::mat4 M = rotate ? glm::rotate(I, TAU / 10.0f * seconds_elapsed, glm::vec3(0.0f, 1.0f, 0.0f)) : I;
	M = glm::scale(M, glm::vec3(scaleFactor));
	M = glm::translate(M, -center);
	//View
	glm::mat4 V = cam.getViewMatrix() * ball.getRotation();
	//Projection
	glm::mat4 P = cam.getProjectionMatrix();

	/************************************************************************/
	/* Send uniform values to shader                                        */
	/************************************************************************/
	if (u_PVM_location != -1) {
		glUniformMatrix4fv(u_PVM_location, 1, GL_FALSE, glm::value_ptr(P * V * M));
	}
	//Set active texture and bind
	glActiveTexture(GL_TEXTURE0); //Active texture unit 0
	textureMapPtr->bind(); //The next binded texture will be refered with the active texture unit
	if (u_texture_location != -1) {
		glUniform1i(u_texture_location, 0); // we bound our texture to texture unit 0
	}
	//Send light and material
	passLightingState();

	/* Draw */
	meshPtr->drawTriangles(a_position_loc, a_normal_loc, a_texture_loc);

	//Unbind an clean
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	/* You need to call this to draw the GUI, After unbinding your program*/
	drawGUI();
	/* But, before flushing the drawinng commands*/

	glutSwapBuffers();
}

void idle() {
	static int last_time = 0;

	int time = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = time - last_time;
	last_time = time;
	float delta_seconds = 0.001f * elapsed;
	seconds_elapsed += delta_seconds;

	glutPostRedisplay();
}

void keyboard(unsigned char key, int mouse_x, int mouse_y) {
	/* See if ImGui handles it*/
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharacter(key);

	/* Now, the app */
	switch (key) {

	case 'R':
	case 'r':
		rotate = !rotate;
		break;

	case 27:
		exit_glut();
		break;

	case 'c':
	case 'C':
		cam.setFovY(PI / 8.0f);
		ball.resetRotation();
	break;

	default:
		break;
	}

	glutPostRedisplay();
}

void mouse(int button, int state, int mouse_x, int mouse_y) {
	/* See if ImGui handles it*/
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2(float(mouse_x), float(mouse_y));

	if (state == GLUT_DOWN && (button == GLUT_LEFT_BUTTON))
		io.MouseDown[0] = true;
	else
		io.MouseDown[0] = false;

	if (state == GLUT_DOWN && (button == GLUT_RIGHT_BUTTON))
		io.MouseDown[1] = true;
	else
		io.MouseDown[1] = false;

	/* Camera trackball*/
	if (button == GLUT_RIGHT_BUTTON) {
		if (state == GLUT_DOWN) {
			ball.startDrag(glm::vec2(mouse_x, mouse_y));
		} else {
			ball.endDrag(glm::vec2(mouse_x, mouse_y));
		}
	}


	glutPostRedisplay();
}

void special(int key, int mouse_x, int mouse_y) {
	/* See if ImGui handles it*/
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharacter(key);

	/* Now, the app*/

	glutPostRedisplay();
}


void mouseWheel(int button, int dir, int mouse_x, int mouse_y) {
	/* See if ImGui handles it*/
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2(float(mouse_x), float(mouse_y));
	if (dir > 0) {
		io.MouseWheel = 1.0;
	} else if (dir < 0) {
		io.MouseWheel = -1.0;
	}

	/* Camera zoom in-out*/
	const float DELTA_ANGLE = PI / 30.0f;
	if (button == 0) {
		cam.addFovY(dir > 0 ? DELTA_ANGLE : -DELTA_ANGLE);
	}

	glutPostRedisplay();
}

void mouseDrag(int mouse_x, int mouse_y) {
	/* See if ImGui handles it*/
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2(float(mouse_x), float(mouse_y));

	/*Trackball camera*/
	ball.drag(glm::ivec2(mouse_x, mouse_y));

	glutPostRedisplay();
}

void mousePasiveMotion(int mouse_x, int mouse_y) {
	/* See if ImGui handles it*/
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2(float(mouse_x), float(mouse_y));

	/*Now, the app*/

	glutPostRedisplay();
}