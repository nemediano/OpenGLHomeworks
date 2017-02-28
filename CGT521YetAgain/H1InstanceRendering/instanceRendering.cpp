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
#include <glm/gtc/random.hpp>

/* Include heders and namespaces from my CGT Library */
#include "OGLHelpers.h"
#include "MathConstants.h"
#include "OGLProgram.h"
#include "Geometries.h"
#include "Camera.h"
#include "Trackball.h"
using namespace ogl;
using namespace math;
using namespace mesh;
using namespace camera;
using namespace std;
/*Camera related function*/
Camera cam;
Trackball ball;

/*Headers needed for imgui */
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glut.h"

/* CGT Library related*/
OGLProgram* programInstanciatedPtr = nullptr;
OGLProgram* programNormalPtr = nullptr;
Mesh* meshPtr = nullptr;
GLint window = 0;

struct InstancedLocation {
	GLint u_PV_location = -1;
	GLint u_NormMat_location = -1;
	GLint u_Time_location = -1;
	GLint a_position_loc = -1;
	GLint a_normal_loc = -1;
	GLint a_texture_loc = -1;
	//Instance attribute
	GLint a_color_loc = -1;
	GLint a_transformation_loc = -1;
};

struct UnInstancedLocation {
	GLint u_PVM_location = -1;
	GLint u_NormMat_location = -1;
	GLint u_color_loc = -1;
	GLint u_Time_location = -1;
	GLint a_position_loc = -1;
	GLint a_normal_loc = -1;
	GLint a_texture_loc = -1;
};

InstancedLocation instLoc;
UnInstancedLocation unInstLoc;

// Location for shader variables

//VBO for instanced attribute
GLuint color_buffer_id = 0;
GLuint transformation_buffer_id = 0;
//Global variables for the program logic
float seconds_elapsed;
glm::vec3 meshCenter;
float scaleFactor;
bool instanciated;
const unsigned int MAX_INSTANCES = 3000;
unsigned int instace_number = 100;

vector<glm::vec3> colors;
vector<glm::mat4> transformations;

void create_glut_window();
void init_program();
void init_OpenGL();
void create_glut_callbacks();
void exit_glut();
void reload_shaders();
void drawInstanciated();
void drawUnInstanciated();

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

//Time query managers
GLuint timer_query;
GLuint nanoseconds;

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


void exit_glut() {
	delete meshPtr;
	delete programInstanciatedPtr;
	/* Delete window (freeglut) */
	glutDestroyWindow(window);
	exit(EXIT_SUCCESS);
}

void create_glut_window() {
	//Set number of samples per pixel
	glutSetOption(GLUT_MULTISAMPLE, 8);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
#if _DEBUG
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
#endif
	glutInitWindowSize(800, 600);
	window = glutCreateWindow("Jorge Garcia, Homework 1");
	
}

void drawGUI() {
	/* Always start with this call*/
	ImGui_ImplGLUT_NewFrame(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	/* Position of the menu, if no imgui.ini exist */
	ImGui::SetNextWindowSize(ImVec2(50, 50), ImGuiSetCond_FirstUseEver);

	/*Create a new menu for my app*/
	ImGui::Begin("Render options");
	ImGui::Checkbox("Draw instanciated", &instanciated);
	int tmp = int(instace_number) / 100;
	ImGui::Text("Instances (hundreds)");
	ImGui::InputInt("", &tmp);
	instace_number = glm::clamp(unsigned int(tmp * 100), 0U, MAX_INSTANCES);
	ImGui::Text("Time in seconds %f", double(nanoseconds) / 1000000000.0);
	ImGui::Text("Time in nanoseconds %u", nanoseconds);
	if (ImGui::Button("Quit")) {
		exit_glut();
	}
	ImGui::End();

	/* End with this when you want to render GUI */
	ImGui::Render();
}

void init_program() {
	using glm::vec3;
	using glm::mat4;
	/* Initialize global variables for program control */
	seconds_elapsed = 0.0f;
	/* Load a mesh from file and send it to GPU */
	meshPtr = new Mesh("../models/cow.obj");
	if (meshPtr) {
		meshCenter = meshPtr->getBBCenter();
		scaleFactor = meshPtr->scaleFactor();
		meshPtr->sendToGPU();
	}
	//Set the default position of the camera
	cam.setLookAt(vec3(0.0f, 0.0f, 3.0f), vec3(0.0f));
	cam.setAspectRatio(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	cam.setFovY(PI / 4.0f);
	cam.setDepthView(0.1f, 9.0f);
	//Create trackball camera
	ball.setWindowSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	//Instanciated mode
	instanciated = true;
	/*Create a buffers for the instanced atributes */
	glGenBuffers(1, &color_buffer_id);
	glGenBuffers(1, &transformation_buffer_id);
	/* Generate a bunch of random colors*/
	vec3 color;
	for (int i = 0; i < MAX_INSTANCES; ++i) {
		color = glm::linearRand(vec3(0.2f), vec3(1.0f));
		colors.push_back(color);
	}
	/* Generate the transformation matrices */
	//Model
	mat4 I = mat4(1.0f);
	glm::mat4 M;
	M = glm::scale(I, 0.5f * vec3(1.0f));
	M = glm::scale(M, scaleFactor * vec3(1.0f));
	M = glm::translate(M, -meshCenter);
	for (int i = 0; i < MAX_INSTANCES; ++i) {
		mat4 T = glm::scale(I, glm::linearRand(0.5f, 1.0f) * vec3(1.0f));
		T = glm::translate(T, glm::ballRand(3.0f));
		transformations.push_back(T * M);
	}

	/* Send the colors and transformation to GPU*/
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(vec3), colors.data(), GL_STATIC_DRAW);
	//This set this attribute as an instanced attribute
	glVertexAttribDivisor(instLoc.a_color_loc, 1);

	glBindBuffer(GL_ARRAY_BUFFER, transformation_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, transformations.size() * sizeof(mat4), transformations.data(), GL_STATIC_DRAW);
	
	//Making instanciated
	for (int i = 0; i < 4; ++i) {
		glVertexAttribDivisor(instLoc.a_transformation_loc + i, 1);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	instanciated = true;
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
	ogl::getErrorLog();
	

	reload_shaders();

	//Activate antialliasing
	glEnable(GL_MULTISAMPLE);

	//Initialize some basic rendering state
	glEnable(GL_DEPTH_TEST);
	//Dark black background color
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

	//For the time query
	glGenQueries(1, &timer_query);
}

void reload_shaders() {
	using std::cerr;
	using std::endl;
	/************************************************************************/
	/*                   OpenGL program creation                            */
	/************************************************************************/
	auto tmpProgram = new OGLProgram("shaders/hm01Instanciated.vert", "shaders/hm01Instanciated.frag");

	if (!tmpProgram || !tmpProgram->isOK()) {
		cerr << "Something wrong in shader" << endl;
		delete tmpProgram;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	}
	else {
		programInstanciatedPtr = tmpProgram;
		glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	}

	tmpProgram = new OGLProgram("shaders/hm01Normal.vert", "shaders/hm01Instanciated.frag");

	if (!tmpProgram || !tmpProgram->isOK()) {
		cerr << "Something wrong in shader" << endl;
		delete tmpProgram;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	}
	else {
		programNormalPtr = tmpProgram;
		glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	}

	/************************************************************************/
	/* Allocating variables for shaders                                     */
	/************************************************************************/
	/* Instanciated first */
	instLoc.u_PV_location = programInstanciatedPtr->uniformLoc("PV");
	instLoc.u_Time_location = programInstanciatedPtr->uniformLoc("time");
	instLoc.u_NormMat_location = programInstanciatedPtr->uniformLoc("NormMat");
	instLoc.a_position_loc = programInstanciatedPtr->attribLoc("Position");
	instLoc.a_normal_loc = programInstanciatedPtr->attribLoc("Normal");
	instLoc.a_texture_loc = programInstanciatedPtr->attribLoc("TextCoord");
	instLoc.a_color_loc = programInstanciatedPtr->attribLoc("Color");
	instLoc.a_transformation_loc = programInstanciatedPtr->attribLoc("M");
	/* UnInstanciated second */
	unInstLoc.u_PVM_location = programNormalPtr->uniformLoc("PVM");
	unInstLoc.u_NormMat_location = programNormalPtr->uniformLoc("NormMat");
	unInstLoc.u_color_loc = programNormalPtr->uniformLoc("Color");
	unInstLoc.a_position_loc = programNormalPtr->attribLoc("Position");
	unInstLoc.a_normal_loc = programNormalPtr->attribLoc("Normal");
	unInstLoc.a_texture_loc = programNormalPtr->attribLoc("TextCoord");
}


void create_glut_callbacks() {
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMouseFunc(mouse);
	glutMouseWheelFunc(mouseWheel);
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
	using glm::mat4;
	using namespace std;
	glBeginQuery(GL_TIME_ELAPSED, timer_query);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	if (instanciated) {
		drawInstanciated();
	} else {
		drawUnInstanciated();
	}
	
	//Unbind an clean
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	/* You need to call this to draw the GUI, After unbinding your program*/
	drawGUI();
	/* But, before flushing the drawinng commands*/

	glutSwapBuffers();
	glFinish();
	glEndQuery(GL_TIME_ELAPSED);
	glGetQueryObjectuiv(timer_query, GL_QUERY_RESULT, &nanoseconds);
}

void drawInstanciated() {
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;
	
	programInstanciatedPtr->use();

	//Model
	mat4 M;
	mat4 I = mat4(1.0f);
	M = glm::scale(I, 0.5f * vec3(1.0f));
	M = glm::scale(M, scaleFactor * vec3(1.0f));
	M = glm::translate(M, -meshCenter);
	//View
	mat4 V = cam.getViewMatrix() * ball.getRotation();
	//Projection
	mat4 P = cam.getProjectionMatrix();

	/************************************************************************/
	/* Send uniform values to shader                                        */
	/************************************************************************/

	if (instLoc.u_PV_location != -1) {
		glUniformMatrix4fv(instLoc.u_PV_location, 1, GL_FALSE, glm::value_ptr(P * V));
	}
	if (instLoc.u_NormMat_location != -1) {
		glUniformMatrix4fv(instLoc.u_NormMat_location, 1, GL_FALSE, glm::value_ptr(glm::inverse(glm::transpose(V * M))));
	}
	if (instLoc.u_Time_location != -1) {
		glUniform1f(instLoc.u_Time_location, seconds_elapsed);
	}
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer_id);
	if (instLoc.a_color_loc != -1) {
		glEnableVertexAttribArray(instLoc.a_color_loc);
		//Colors in this buffer are thigly packes so the zero at the end
		glVertexAttribPointer(instLoc.a_color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);
	}
	/* Matrix atribute location actually uses four location one per column
	This code is taken form the OpenGL programming guide pg 130*/
	glBindBuffer(GL_ARRAY_BUFFER, transformation_buffer_id);
	if (instLoc.a_transformation_loc != -1) {
		for (int i = 0; i < 4; i++) {
			// Set up the vertex attribute
			if (instLoc.a_transformation_loc + i != -1) {
				// Enable it
				glEnableVertexAttribArray(instLoc.a_transformation_loc + i);
				glVertexAttribPointer(instLoc.a_transformation_loc + i, // Location
					4, GL_FLOAT, GL_FALSE, // vec4
					sizeof(mat4), // Stride
					(void *)(sizeof(vec4) * i)); // Start offset
			}
		}
	}

	/* Draw */
	meshPtr->drawTriangles(instLoc.a_position_loc, instLoc.a_normal_loc, instLoc.a_texture_loc, instace_number);

	if (instLoc.a_transformation_loc != -1) {
		for (int i = 0; i < 4; i++) {
			if (instLoc.a_transformation_loc + i != -1) {
				glDisableVertexAttribArray(instLoc.a_transformation_loc + i);
			}
		}
	}

	if (instLoc.a_color_loc != -1) {
		glDisableVertexAttribArray(instLoc.a_color_loc);
	}
}

void drawUnInstanciated() {
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

	programNormalPtr->use();

	//Model
	mat4 M;
	//View
	mat4 V = cam.getViewMatrix() * ball.getRotation();
	//Projection
	mat4 P = cam.getProjectionMatrix();
	vec3 color;
	if (unInstLoc.u_Time_location != -1) {
		glUniform1f(unInstLoc.u_Time_location, seconds_elapsed);
	}

	/************************************************************************/
	/* Send uniform values to shader                                        */
	/************************************************************************/
	for (unsigned int i = 0; i < instace_number; ++i) {
		M = transformations[i];
		
		if (unInstLoc.u_PVM_location != -1) {
			glUniformMatrix4fv(unInstLoc.u_PVM_location, 1, GL_FALSE, glm::value_ptr(P * V * M));
		}
		
		if (unInstLoc.u_NormMat_location != -1) {
			glUniformMatrix4fv(unInstLoc.u_NormMat_location, 1, GL_FALSE, glm::value_ptr(glm::inverse(glm::transpose(V * M))));
		}
		color = colors[i];
		if (unInstLoc.u_color_loc != -1) {
			glUniform3fv(unInstLoc.u_color_loc, 1, glm::value_ptr(color));
		}

		/* Draw */
		meshPtr->drawTriangles(unInstLoc.a_position_loc, unInstLoc.a_normal_loc, unInstLoc.a_texture_loc);
	}
	
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
	switch (key) {

		case 'R':
		case 'r':
			reload_shaders();
		break;

		case 27:
			exit_glut();
		break;
	
		case 'i':
		case 'I':
			instanciated = !instanciated;
		break;

		default:
		break;
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
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			ball.startDrag(glm::vec2(mouse_x, mouse_y));
		}
		else {
			ball.endDrag(glm::vec2(mouse_x, mouse_y));
		}
	}

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
