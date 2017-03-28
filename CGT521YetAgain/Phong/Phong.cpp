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
#include "LightPhong.h"
#include "MatPhong.h"
#include "Geometries.h"
using namespace math;
using namespace mesh;
using namespace camera;
using namespace ogl;
using namespace lighting;
/* CGT Library related*/
Camera cam;
Trackball ball;
Mesh* meshPtr = nullptr;
OGLProgram* phongWorldPtr = nullptr;
OGLProgram* phongViewPtr = nullptr;
MatPhong mat;
GLint window = 0;
// Location for shader variables
struct Locations {
	GLint u_PVM = -1;
	GLint u_PV = -1;
	GLint u_P = -1;
	GLint u_V = -1;
	GLint u_M = -1;

	GLint u_La = -1;
	GLint u_Ls = -1;
	GLint u_Ld = -1;

	GLint u_LightPos = -1;

	GLint u_Ka = -1;
	GLint u_Ks = -1;
	GLint u_Kd = -1;
	GLint u_alpha = -1;

	GLint a_position = -1;
	GLint a_normal = -1;
	GLint a_texture = -1;
};

Locations phongViewLoc;
Locations phongWorldLoc;

//Global variables for the program logic
struct Light {
	glm::vec2 angles;
	float distance;
	LightPhong properties;
};

Light light;

float seconds_elapsed;

float scaleFactor;
glm::vec3 meshCenter;

bool gammaCorrection;
float gamma;
bool rotation;
glm::vec3 backgroundColor;
int currentShader;

void passLightingState();
void reload_shaders();
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
	using glm::vec3;

	/* Always start with this call*/
	ImGui_ImplGLUT_NewFrame(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	/* Position of the menu, if no imgui.ini exist */
	ImGui::SetNextWindowSize(ImVec2(50, 50), ImGuiSetCond_FirstUseEver);

	/*Create a new menu for my app*/
	ImGui::Begin("Options");

	ImGui::Text("Space for calculations");
	ImGui::RadioButton("World", &currentShader, 0); ImGui::SameLine();
	ImGui::RadioButton("View", &currentShader, 1);
	
	if (ImGui::CollapsingHeader("Material editor")) {
		//Edit Material
		vec3 Ka = mat.getKa();
		vec3 Kd = mat.getKd();
		vec3 Ks = mat.getKs();
		float alpha = mat.getAlpha();
		ImGui::ColorEdit3("Ambient", glm::value_ptr(Ka));
		ImGui::ColorEdit3("Difusse", glm::value_ptr(Kd));
		ImGui::ColorEdit3("Specular", glm::value_ptr(Ks));
		ImGui::SliderFloat("Alpha", &alpha, 0.0f, 1.0f, "%.2f", 2.0f);
		mat.setKa(Ka);
		mat.setKd(Kd);
		mat.setKs(Ks);
		mat.setAlpha(alpha);
	}

	if (ImGui::CollapsingHeader("Light physical properties")) {
		vec3 La = light.properties.getLa();
		vec3 Ld = light.properties.getLd();
		vec3 Ls = light.properties.getLs();
		ImGui::ColorEdit3("Ambient", glm::value_ptr(La));
		ImGui::ColorEdit3("Difusse", glm::value_ptr(Ld));
		ImGui::ColorEdit3("Specular", glm::value_ptr(Ls));
		light.properties.setLa(La);
		light.properties.setLd(Ld);
		light.properties.setLs(Ls);
	}
	
	
	if (ImGui::CollapsingHeader("General options")) {
		ImGui::Checkbox("Rotation", &rotation);
		ImGui::Checkbox("Gamma correction", &gammaCorrection);
		if (gammaCorrection) {
			ImGui::SliderFloat("Gamma", &gamma, 1.0f, 2.5f);
		}
		if (ImGui::Button("Quit")) {
			exit_glut();
		}
		if (ImGui::Button("Reset camera")) {
			cam.setFovY(PI / 8.0f);
			ball.resetRotation();
		}
	}
	ImGui::End();

	/* End with this when you want to render GUI */
	ImGui::Render();
}

void exit_glut() {
	delete meshPtr;
	delete phongViewPtr;
	delete phongWorldPtr;
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
	window = glutCreateWindow("Phong shading");
}

void init_program() {
	using glm::vec3;
	/* Initialize global variables for program control */
	rotation = false;
	/* Then, create primitives (load them from mesh) */
	meshPtr = new Mesh("../models/teapot.obj");
	//meshPtr = new Mesh(Geometries::icosphere(3));

	if (meshPtr) {
		meshPtr->sendToGPU();
	}
	//Extract info form the mesh
	scaleFactor = meshPtr->scaleFactor();
	meshCenter = meshPtr->getBBCenter();

	seconds_elapsed = 0.0f;
	
	//Set the default position of the camera
	cam.setLookAt(vec3(0.0f, 0.0f, 1.5f), vec3(0.0f));
	cam.setAspectRatio(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	cam.setFovY(PI / 4.0f);
	cam.setDepthView(0.1f, 3.0f);
	//Create trackball camera
	ball.setWindowSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	//Default position of the spotlight
	light.angles = glm::vec2(0.0f);
	light.distance = 2.0f;
	light.properties.setLa(vec3(1.0f));
	light.properties.setLs(vec3(1.0f));
	light.properties.setLd(vec3(1.0f));
	//Default material
	vec3 yellow = vec3(1.0f, 1.0f, 0.0f);
	mat.setKa(0.25f * yellow);
	mat.setKd(yellow);
	mat.setKs(vec3(0.5f));

	backgroundColor = vec3(0.15f);
	
	currentShader = 0;
	gammaCorrection = false;
	gamma = 1.0f;
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
	//Load shaders
	reload_shaders();
	
	//Activate antialliasing
	glEnable(GL_MULTISAMPLE);

	//Initialize some basic rendering state
	glEnable(GL_DEPTH_TEST);
	
	glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
}

void reload_shaders() {
	using std::cout;
	using std::cerr;
	using std::endl;

	phongViewPtr = new OGLProgram("shaders/phongView.vert", "shaders/phongView.frag");
	phongWorldPtr = new OGLProgram("shaders/phongWorld.vert", "shaders/phongWorld.frag");

	if (!phongViewPtr || !phongViewPtr->isOK()) {
		cerr << "Something wrong in Phong view shader" << endl;
		backgroundColor = vec3(1.0f, 0.0f, 1.0f);
	}

	if (!phongWorldPtr || !phongWorldPtr->isOK()) {
		cerr << "Something wrong in Phong world shader" << endl;
		backgroundColor = vec3(1.0f, 0.0f, 1.0f);
	}

	if (phongViewPtr->isOK() && phongWorldPtr->isOK()) {
		cout << "Shaders compiled correctlly!" << endl;
		backgroundColor = 0.15f * vec3(1.0f);
	}

	/************************************************************************/
	/* Allocating variables for shaders                                     */
	/************************************************************************/
	//Uniforms
	phongViewLoc.u_PVM = phongViewPtr->uniformLoc("PVM");
	phongViewLoc.u_M = phongViewPtr->uniformLoc("M");
	//Atrributes
	phongViewLoc.a_position = phongViewPtr->attribLoc("Position");
	phongViewLoc.a_normal = phongViewPtr->attribLoc("Normal");
	
	
}

void passLightingState() {
	if (currentShader == 0) {
		glUniform3fv(u_LightCol_loc, 1, glm::value_ptr(dirLight.getColor()));
		glUniform1f(u_LightInt_loc, dirLight.getIntensity());
		glUniformMatrix4fv(u_LightM_loc, 1, GL_FALSE, glm::value_ptr(light.getM()));
	} else {
		glUniform3fv(u_LightCol_loc, 1, glm::value_ptr(dirLight.getColor()));
		glUniform1f(u_LightInt_loc, dirLight.getIntensity());
		glUniformMatrix4fv(u_LightM_loc, 1, GL_FALSE, glm::value_ptr(light.getM()));
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
	using glm::vec3;
	using glm::vec4;
	using glm::vec2;
	using glm::mat4;

	mat4 I(1.0f);
	//Model
	mat4 M = rotation ? glm::rotate(I, TAU / 10.0f * seconds_elapsed, vec3(0.0f, 1.0f, 0.0f)) : I;
	//M = glm::rotate(M, TAU / 4.0f, vec3(1.0, 0.0f, 0.0f));
	M = glm::scale(M, vec3(scaleFactor));
	M = glm::translate(M, -meshCenter);

	//View
	mat4 V = cam.getViewMatrix() * ball.getRotation();
	//Projection
	mat4 P = cam.getProjectionMatrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (currentShader == 0) {
		phongViewPtr->use();

		/************************************************************************/
		/* Send uniform values to shader                                        */
		/************************************************************************/

		glUniformMatrix4fv(u_PVM_location, 1, GL_FALSE, glm::value_ptr(P * V * M));	
		glUniform3fv(u_CamPos_loc, 1, glm::value_ptr(glm::vec3(cam.getPosition())));
		

		//Send light and material
		passLightingState();

		/* Draw */
		meshPtr->drawTriangles(phongViewLoc.a_position, phongViewLoc.a_normal, phongViewLoc.a_texture);
	} else {
		phongWorldPtr->use();

		/************************************************************************/
		/* Send uniform values to shader                                        */
		/************************************************************************/

		glUniformMatrix4fv(u_PVM_location, 1, GL_FALSE, glm::value_ptr(P * V * M));
		glUniform3fv(u_CamPos_loc, 1, glm::value_ptr(glm::vec3(cam.getPosition())));
	

		//Send light and material
		passLightingState();

		/* Draw */
		meshPtr->drawTriangles(phongWorldLoc.a_position, phongWorldLoc.a_normal, phongWorldLoc.a_texture);
	}
	
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
		rotation = !rotation;
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
		}
		else {
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
	}
	else if (dir < 0) {
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