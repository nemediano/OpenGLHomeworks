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
#include <glm/gtx/euler_angles.hpp>

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
#include "MatCookTorrance.h"
#include "Geometries.h"
#include "ScreenGrabber.h"

using namespace math;
using namespace mesh;
using namespace camera;
using namespace ogl;
using namespace lighting;
using namespace image;
/* CGT Library related*/
Camera cam;
Trackball ball;
ScreenGrabber grabber;
Mesh* meshPtr = nullptr;
Mesh* cubePtr = nullptr;
Mesh* spherePtr = nullptr;
Mesh* currentMeshPtr = nullptr;
OGLProgram* cookTorranceWorldPtr = nullptr;
OGLProgram* cookTorranceViewPtr = nullptr;
MatCookTorrance mat;
GLint window = 0;
// Location for shader variables
struct Locations {
	GLint u_PVM = -1;
	GLint u_VM = -1;
	GLint u_P = -1;
	GLint u_V = -1;
	GLint u_M = -1;
	GLint u_NormMat = -1;
	GLint u_gamma = -1;
	GLint u_option = -1;

	GLint u_La = -1;
	GLint u_Ls = -1;
	GLint u_Ld = -1;

	GLint u_LightPos = -1;
	GLint u_CameraPos = -1;

	GLint u_Ka = -1;
	GLint u_Ks = -1;
	GLint u_Kd = -1;
	GLint u_eta = -1;
	GLint u_m = -1;

	GLint a_position = -1;
	GLint a_normal = -1;
	GLint a_texture = -1;
};

Locations cookTorranceViewLoc;
Locations cookTorranceWorldLoc;

//Global variables for the program logic
struct Light {
	glm::vec3 eulerAngles;
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
int currentMesh;
int currentMode;

std::vector<MatCookTorrance> materials;

void passLightingState();
void reload_shaders();
void change_mesh();
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

	ImGui::Text("Mesh");
	ImGui::RadioButton("Custom", &currentMesh, 0); ImGui::SameLine();
	ImGui::RadioButton("Sphere", &currentMesh, 1); ImGui::SameLine();
	ImGui::RadioButton("Cube", &currentMesh, 2);

	ImGui::Text("Space for calculations");
	ImGui::RadioButton("World", &currentShader, 0); ImGui::SameLine();
	ImGui::RadioButton("View", &currentShader, 1);

	ImGui::Text("Reflections");
	ImGui::RadioButton("Full", &currentMode, 0); ImGui::SameLine();
	ImGui::RadioButton("F", &currentMode, 1); ImGui::SameLine();
	ImGui::RadioButton("G", &currentMode, 2); ImGui::SameLine();
	ImGui::RadioButton("D", &currentMode, 3);

	ImGui::Text("Light position");
	ImGui::SliderFloat("Distance", &light.distance, 0.0f, 5.0f);
	ImGui::SliderAngle("Angle X", &light.eulerAngles.x, -180.0f, 180.0f);
	ImGui::SliderAngle("Angle Y", &light.eulerAngles.y, -180.0f, 180.0f);

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

	if (ImGui::CollapsingHeader("Material editor")) {
		//Edit Material
		vec3 Ka = mat.getKa();
		vec3 Kd = mat.getKd();
		vec3 Ks = mat.getKs();
		float eta = mat.getRefractionIndex();
		float m = mat.getMicrofacetSlope();
		ImGui::ColorEdit3("Ambient", glm::value_ptr(Ka));
		ImGui::ColorEdit3("Difusse", glm::value_ptr(Kd));
		ImGui::ColorEdit3("Specular", glm::value_ptr(Ks));
		ImGui::Text("Index of refraction");
		ImGui::SliderFloat("Eta", &eta, 1.0f, 5.0f, "%.2f");
		ImGui::Text("Microfacet orientation");
		ImGui::SliderFloat("m", &m, 0.0f, PI, "%.3f");
		mat.setKa(Ka);
		mat.setKd(Kd);
		mat.setKs(Ks);
		mat.setRefractionIndex(eta);
		mat.setMicrofacetSlope(m);

		const char* items[] = {
			"Emerald", "Jade", "Obsidian", "Pearl", "Ruby", "Turquoise",
			"Brass", "Bronze", "Chrome", "Copper", "Gold", "Silver",
			"Black plastic", "Cyan plastic", "Green plastic", "Red plastic", "White plastic", "Yellow plastic",
			"Black rubber", "Cyan rubber", "Green rubber", "Red rubber", "White rubber", "Yellow rubber",
		};
		static int selectedMaterial = -1;
		ImGui::Combo("Predefined", &selectedMaterial, items, static_cast<int>(materials.size()));
		if (ImGui::Button("Load") && selectedMaterial != -1) {
			mat = materials[selectedMaterial];
			selectedMaterial = -1;
		}
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
		ImGui::SameLine();
		if (ImGui::Button("Reset camera")) {
			cam.setFovY(PI / 4.0f);
			ball.resetRotation();
		}
		ImGui::SameLine();
		if (ImGui::Button("Screenshot")) {
			grabber.grab();
		}
	}
	ImGui::End();

	/* End with this when you want to render GUI */
	ImGui::Render();
}

void exit_glut() {
	delete meshPtr;
	delete cubePtr;
	delete spherePtr;
	delete cookTorranceViewPtr;
	delete cookTorranceWorldPtr;
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
	glutInitWindowSize(1200, 800);
	window = glutCreateWindow("Cook-Torrance shading");
}

void init_program() {
	using glm::vec3;
	using namespace lighting;
	/* Initialize global variables for program control */
	rotation = false;
	/* Then, create primitives (load them from mesh) */
	meshPtr = new Mesh("../models/teapot.obj");
	spherePtr = new Mesh(Geometries::icosphere(3));
	cubePtr = new Mesh(Geometries::cube());

	if (meshPtr) {
		meshPtr->sendToGPU();
	}

	if (spherePtr) {
		spherePtr->sendToGPU();
	}

	if (cubePtr) {
		cubePtr->sendToGPU();
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
	//Also setup the image grabber
	grabber.resize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	//Default position of the spotlight
	light.eulerAngles = glm::vec3(-TAU / 8.0f, TAU / 8.0f, 0.0f);
	light.distance = glm::sqrt(3.0f);
	light.properties.setLa(vec3(1.0f));
	light.properties.setLs(vec3(1.0f));
	light.properties.setLd(vec3(1.0f));
	//Default material
	mat.setKa(vec3(0.122f, 0.216f, 0.145f));
	mat.setKd(vec3(0.4f, 0.742f, 0.7f));
	mat.setKs(vec3(0.48f, 0.188f, 0.447f));
	mat.setMicrofacetSlope(0.3f);
	mat.setRefractionIndex(3.2f);

	backgroundColor = vec3(0.15f);

	currentShader = 0;
	currentMesh = 0;
	currentMode = 0;
	gammaCorrection = false;
	gamma = 1.0f;
	/*Load predefined materials*/
	materials.push_back(EMERALD);
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

	cookTorranceViewPtr = new OGLProgram("shaders/cookTorranceView.vert", "shaders/cookTorranceView.frag");
	cookTorranceWorldPtr = new OGLProgram("shaders/cookTorranceWorld.vert", "shaders/cookTorranceWorld.frag");

	if (!cookTorranceViewPtr || !cookTorranceViewPtr->isOK()) {
		cerr << "Something wrong in Phong view shader" << endl;
		backgroundColor = glm::vec3(1.0f, 0.0f, 1.0f);
	}

	if (!cookTorranceWorldPtr || !cookTorranceWorldPtr->isOK()) {
		cerr << "Something wrong in Phong world shader" << endl;
		backgroundColor = glm::vec3(1.0f, 0.0f, 1.0f);
	}

	if (cookTorranceViewPtr->isOK() && cookTorranceWorldPtr->isOK()) {
		cout << "Shaders compiled correctlly!" << endl;
		backgroundColor = 0.15f * glm::vec3(1.0f);
	}

	/************************************************************************/
	/* Allocating variables for shaders                                     */
	/************************************************************************/
	//Uniforms
	cookTorranceViewLoc.u_PVM = cookTorranceViewPtr->uniformLoc("PVM");
	cookTorranceViewLoc.u_VM = cookTorranceViewPtr->uniformLoc("VM");
	cookTorranceViewLoc.u_NormMat = cookTorranceViewPtr->uniformLoc("NormalMat");
	cookTorranceViewLoc.u_gamma = cookTorranceViewPtr->uniformLoc("gamma");
	cookTorranceViewLoc.u_option = cookTorranceViewPtr->uniformLoc("option");
	//Light
	cookTorranceViewLoc.u_LightPos = cookTorranceViewPtr->uniformLoc("light.position");
	cookTorranceViewLoc.u_La = cookTorranceViewPtr->uniformLoc("light.La");
	cookTorranceViewLoc.u_Ld = cookTorranceViewPtr->uniformLoc("light.Ld");
	cookTorranceViewLoc.u_Ls = cookTorranceViewPtr->uniformLoc("light.Ls");
	//Material
	cookTorranceViewLoc.u_eta = cookTorranceViewPtr->uniformLoc("mat.eta");
	cookTorranceViewLoc.u_m = cookTorranceViewPtr->uniformLoc("mat.m");
	cookTorranceViewLoc.u_Ka = cookTorranceViewPtr->uniformLoc("mat.Ka");
	cookTorranceViewLoc.u_Kd = cookTorranceViewPtr->uniformLoc("mat.Kd");
	cookTorranceViewLoc.u_Ks = cookTorranceViewPtr->uniformLoc("mat.Ks");

	//Atrributes
	cookTorranceViewLoc.a_position = cookTorranceViewPtr->attribLoc("Position");
	cookTorranceViewLoc.a_normal = cookTorranceViewPtr->attribLoc("Normal");
	cookTorranceViewLoc.a_texture = cookTorranceViewPtr->attribLoc("TextCoord");

	/************************************************************/

	//Uniforms
	cookTorranceWorldLoc.u_PVM = cookTorranceWorldPtr->uniformLoc("PVM");
	cookTorranceWorldLoc.u_M = cookTorranceWorldPtr->uniformLoc("M");
	cookTorranceWorldLoc.u_NormMat = cookTorranceWorldPtr->uniformLoc("NormalMat");
	cookTorranceWorldLoc.u_gamma = cookTorranceWorldPtr->uniformLoc("gamma");
	cookTorranceWorldLoc.u_option = cookTorranceWorldPtr->uniformLoc("option");
	cookTorranceWorldLoc.u_CameraPos = cookTorranceWorldPtr->uniformLoc("cameraPosition");
	//Light
	cookTorranceWorldLoc.u_LightPos = cookTorranceWorldPtr->uniformLoc("light.position");
	cookTorranceWorldLoc.u_La = cookTorranceWorldPtr->uniformLoc("light.La");
	cookTorranceWorldLoc.u_Ld = cookTorranceWorldPtr->uniformLoc("light.Ld");
	cookTorranceWorldLoc.u_Ls = cookTorranceWorldPtr->uniformLoc("light.Ls");
	//Material
	cookTorranceWorldLoc.u_eta = cookTorranceWorldPtr->uniformLoc("mat.eta");
	cookTorranceWorldLoc.u_m = cookTorranceWorldPtr->uniformLoc("mat.m");
	cookTorranceWorldLoc.u_Ka = cookTorranceWorldPtr->uniformLoc("mat.Ka");
	cookTorranceWorldLoc.u_Kd = cookTorranceWorldPtr->uniformLoc("mat.Kd");
	cookTorranceWorldLoc.u_Ks = cookTorranceWorldPtr->uniformLoc("mat.Ks");
	//Atrributes
	cookTorranceWorldLoc.a_position = cookTorranceWorldPtr->attribLoc("Position");
	cookTorranceWorldLoc.a_normal = cookTorranceWorldPtr->attribLoc("Normal");
	cookTorranceWorldLoc.a_texture = cookTorranceWorldPtr->attribLoc("TextCoord");

}

void passLightingState() {
	using glm::vec3;

	if (currentShader == 0) {
		switch (currentMode) {
			//Full model
			case 0:
				glUniform3fv(cookTorranceWorldLoc.u_Ls, 1, glm::value_ptr(light.properties.getLs()));
				glUniform3fv(cookTorranceWorldLoc.u_Ka, 1, glm::value_ptr(mat.getKa()));
				glUniform3fv(cookTorranceWorldLoc.u_Ks, 1, glm::value_ptr(mat.getKs()));
				glUniform3fv(cookTorranceWorldLoc.u_Kd, 1, glm::value_ptr(mat.getKd()));
			break;
			//Just one component of the model
			case 1:
			case 2:
			case 3:
				glUniform3fv(cookTorranceWorldLoc.u_Ls, 1, glm::value_ptr(vec3(1.0f)));
				glUniform3fv(cookTorranceWorldLoc.u_Ka, 1, glm::value_ptr(vec3(0.0f)));
				glUniform3fv(cookTorranceWorldLoc.u_Ks, 1, glm::value_ptr(vec3(1.0f)));
				glUniform3fv(cookTorranceWorldLoc.u_Kd, 1, glm::value_ptr(vec3(0.0f)));
			break;
		}
		glUniform1f(cookTorranceWorldLoc.u_eta, mat.getRefractionIndex());
		glUniform1f(cookTorranceWorldLoc.u_m, mat.getMicrofacetSlope());

		glUniform3fv(cookTorranceWorldLoc.u_La, 1, glm::value_ptr(light.properties.getLa()));
		glUniform3fv(cookTorranceWorldLoc.u_Ld, 1, glm::value_ptr(light.properties.getLd()));

	} else {
		switch (currentMode) {
			//Full model
			case 0:
				glUniform3fv(cookTorranceWorldLoc.u_Ls, 1, glm::value_ptr(light.properties.getLs()));
				glUniform3fv(cookTorranceViewLoc.u_Ka, 1, glm::value_ptr(mat.getKa()));
				glUniform3fv(cookTorranceViewLoc.u_Ks, 1, glm::value_ptr(mat.getKs()));
				glUniform3fv(cookTorranceViewLoc.u_Kd, 1, glm::value_ptr(mat.getKd()));
			break;
			//Just one component of the model
			case 1:
			case 2:
			case 3:
				glUniform3fv(cookTorranceWorldLoc.u_Ls, 1, glm::value_ptr(vec3(1.0f)));
				glUniform3fv(cookTorranceViewLoc.u_Ka, 1, glm::value_ptr(vec3(0.0f)));
				glUniform3fv(cookTorranceViewLoc.u_Ks, 1, glm::value_ptr(vec3(1.0f)));
				glUniform3fv(cookTorranceViewLoc.u_Kd, 1, glm::value_ptr(vec3(0.0f)));
			break;
		}
		glUniform1f(cookTorranceViewLoc.u_eta, mat.getRefractionIndex());
		glUniform1f(cookTorranceViewLoc.u_m, mat.getMicrofacetSlope());

		glUniform3fv(cookTorranceViewLoc.u_La, 1, glm::value_ptr(light.properties.getLa()));
		glUniform3fv(cookTorranceViewLoc.u_Ld, 1, glm::value_ptr(light.properties.getLd()));
	}
}

void change_mesh() {
	switch (currentMesh) {
	case 0:
		currentMeshPtr = meshPtr;
		break;

	case 1:
		currentMeshPtr = spherePtr;
		break;

	case 2:
		currentMeshPtr = cubePtr;
		break;
	}

	scaleFactor = currentMeshPtr->scaleFactor();
	meshCenter = currentMeshPtr->getBBCenter();
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
	grabber.resize(new_window_width, new_window_height);
}

void display() {
	using glm::vec3;
	using glm::vec4;
	using glm::vec2;
	using glm::mat4;

	//Select mesh to render
	change_mesh();

	mat4 I(1.0f);
	//Model
	mat4 M = rotation ? glm::rotate(I, TAU / 10.0f * seconds_elapsed, vec3(0.0f, 1.0f, 0.0f)) : I;
	M = glm::scale(M, vec3(scaleFactor));
	M = glm::translate(M, -meshCenter);
	//View
	mat4 V = cam.getViewMatrix() * ball.getRotation();
	//Projection
	mat4 P = cam.getProjectionMatrix();

	//Calculate Light position
	vec4 light_initial_pos = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	mat4 T = glm::translate(I, vec3(0.0f, 0.0f, light.distance));
	vec3 light_position = vec3(glm::eulerAngleXYZ(light.eulerAngles.x, light.eulerAngles.y, light.eulerAngles.z) * T * light_initial_pos);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (currentShader == 0) {
		cookTorranceWorldPtr->use();
		/************************************************************************/
		/* Send uniform values to shader                                        */
		/************************************************************************/
		glUniformMatrix4fv(cookTorranceWorldLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M));
		glUniformMatrix4fv(cookTorranceWorldLoc.u_M, 1, GL_FALSE, glm::value_ptr(M));
		glUniformMatrix4fv(cookTorranceWorldLoc.u_NormMat, 1, GL_FALSE, glm::value_ptr(glm::inverse(glm::transpose(M))));
		glUniform1f(cookTorranceWorldLoc.u_gamma, gamma);
		glUniform3fv(cookTorranceWorldLoc.u_LightPos, 1, glm::value_ptr(light_position));
		vec4 camera_pos = vec4(cam.getPosition(), 1.0f);
		vec3 camPosInWorld = vec3(glm::inverse(V) * camera_pos);
		glUniform3fv(cookTorranceWorldLoc.u_CameraPos, 1, glm::value_ptr(vec3(camPosInWorld)));

		//Send light and material
		passLightingState();
		/* Draw */
		currentMeshPtr->drawTriangles(cookTorranceWorldLoc.a_position, cookTorranceWorldLoc.a_normal, cookTorranceWorldLoc.a_texture);
	}
	else {
		cookTorranceViewPtr->use();

		/************************************************************************/
		/* Send uniform values to shader                                        */
		/************************************************************************/

		glUniformMatrix4fv(cookTorranceViewLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M));
		glUniformMatrix4fv(cookTorranceViewLoc.u_VM, 1, GL_FALSE, glm::value_ptr(V * M));
		glUniformMatrix4fv(cookTorranceViewLoc.u_NormMat, 1, GL_FALSE, glm::value_ptr(glm::inverse(glm::transpose(V * M))));
		glUniform1f(cookTorranceViewLoc.u_gamma, gamma);
		vec3 posInView = vec3(V * vec4(light_position, 1.0f));
		glUniform3fv(cookTorranceViewLoc.u_LightPos, 1, glm::value_ptr(posInView));
		//Send light and material
		passLightingState();

		/* Draw */
		currentMeshPtr->drawTriangles(cookTorranceViewLoc.a_position, cookTorranceViewLoc.a_normal, cookTorranceViewLoc.a_texture);
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
	case '1':
		currentMesh = 0;
		break;

	case '2':
		currentMesh = 1;
		break;

	case '3':
		currentMesh = 2;
		break;

	case 'W':
	case 'w':
		currentShader = 0;
		break;

	case 'V':
	case 'v':
		currentShader = 1;
		break;

	case 'R':
	case 'r':
		rotation = !rotation;
		break;

	case ' ':
		grabber.grab();
		break;

	case 27:
		exit_glut();
		break;

	case 'c':
	case 'C':
		cam.setFovY(PI / 4.0f);
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
	switch (key) {
	case GLUT_KEY_F1:
		currentMode = 0;
		break;

	case GLUT_KEY_F2:
		currentMode = 1;
		break;

	case GLUT_KEY_F3:
		currentMode = 2;
		break;

	case GLUT_KEY_F4:
		currentMode = 3;
		break;
	}
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