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
#include "Texture.h"
using namespace image;
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
OGLProgram* programGeomPassPtr = nullptr;
OGLProgram* programFilterPtr = nullptr;
OGLProgram* programNoFilterPtr = nullptr;
OGLProgram* programAvg3FilterPtr = nullptr;
OGLProgram* programAvg5FilterPtr = nullptr;
OGLProgram* programEdgeFilterPtr = nullptr;

Mesh* meshPtr = nullptr;
Mesh* quadMeshPtr = nullptr;
Texture* textureMapPtr = nullptr;
GLint window = 0;

struct GeomLocations {
	GLint u_P = -1;
	GLint u_V = -1;
	GLint u_Time = -1;
	GLint u_selected = -1;
	GLint u_Texture = -1;
	GLint a_position = -1;
	GLint a_normal = -1;
	GLint a_texture = -1;
	//Instance attribute
	GLint a_color = -1;
	GLint a_transformation = -1;
};

GeomLocations loc;

struct FilterLocations {
	GLint u_PVM = -1;
	GLint u_colTex = -1;
	GLint a_position = -1;
	GLint a_normal = -1;
	GLint a_texture = -1;
};

FilterLocations filoc;

struct FBOproperties {
	GLuint id = 0;
	int width = 0;
	int height = 0;
	GLuint colorTex = 0;
	GLuint pickTex = 0;
	GLuint depthTex = 0;
};

FBOproperties fbo;

//VBO for instanced attribute
GLuint color_buffer_id = 0;
GLuint transformation_buffer_id = 0;
//Global variables for the program logic
float seconds_elapsed;
int filter_option;
unsigned int instance_selected;
glm::vec3 meshCenter;
float scaleFactor;
const unsigned int MAX_INSTANCES = 1024;
unsigned int instace_number = 254;

vector<glm::vec3> colors;
vector<glm::mat4> transformations;

void create_glut_window();
void init_program();
void init_OpenGL();
void create_glut_callbacks();
void exit_glut();
void changeFilter();
void reload_shaders();
void geomPass();
void setupGeomPass();
void filterPass();
void setupFilterPass();
void drawUnInstanciated();

//FBO manages related functions
void createFBO(int width, int height);
void deleteFBO();

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


void exit_glut() {
	delete meshPtr;
	delete programGeomPassPtr;
	delete quadMeshPtr;
	delete programFilterPtr;
	/* Delete window (freeglut) */
	glutDestroyWindow(window);
	exit(EXIT_SUCCESS);
}

void create_glut_window() {
	//Set number of samples per pixel
	glutSetOption(GLUT_MULTISAMPLE, 8);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(800, 600);
	window = glutCreateWindow("Jorge Garcia, Homework 2");
}

void init_program() {
	using glm::vec3;
	using glm::mat4;
	
	/* Initialize global variables for program control */
	seconds_elapsed = 0.0f;
	filter_option = 0;
	instance_selected = 0;
	//Set the default position of the camera
	cam.setLookAt(vec3(0.0f, 0.0f, 3.0f), vec3(0.0f));
	cam.setAspectRatio(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	cam.setFovY(PI / 4.0f);
	cam.setDepthView(0.1f, 9.0f);
	//Create trackball camera
	ball.setWindowSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	
	setupGeomPass();
	setupFilterPass();
	//Moved to reshape I believe
	//createFBO();
	changeFilter();
}

void setupGeomPass() {
	using glm::vec3;
	using glm::mat4;

	/* Load a mesh from file and send it to GPU */
	meshPtr = new Mesh("../models/Amago.obj");
	if (meshPtr) {
		meshCenter = meshPtr->getBBCenter();
		scaleFactor = meshPtr->scaleFactor();
		meshPtr->sendToGPU();
	}
	textureMapPtr = new Texture("../models/AmagoTexture.png");

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
	M = glm::scale(I, scaleFactor * vec3(1.0f));
	M = glm::translate(M, -meshCenter);
	for (int i = 0; i < MAX_INSTANCES; ++i) {
		mat4 T = glm::scale(I, glm::linearRand(0.25f, 0.5f) * vec3(1.0f));
		T = glm::translate(T, glm::ballRand(3.0f));
		transformations.push_back(T * M);
	}
	/*Create a buffers for the instanced atributes */
	glGenBuffers(1, &color_buffer_id);
	glGenBuffers(1, &transformation_buffer_id);

	/* Send the colors to GPU*/
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(vec3), colors.data(), GL_STATIC_DRAW);
	//This set this attribute as an instanced attribute
	glVertexAttribDivisor(loc.a_color, 1);
	/* Send the attributes to GPU*/
	glBindBuffer(GL_ARRAY_BUFFER, transformation_buffer_id);
	glBufferData(GL_ARRAY_BUFFER, transformations.size() * sizeof(mat4), transformations.data(), GL_STATIC_DRAW);
	//Making instanciated
	for (int i = 0; i < 4; ++i) {
		glVertexAttribDivisor(loc.a_transformation + i, 1);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void setupFilterPass() {
	quadMeshPtr = new Mesh(Geometries::plane());
	if (quadMeshPtr) {
		quadMeshPtr->sendToGPU();
	}

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
}

void reload_shaders() {
	using std::cerr;
	using std::endl;
	/************************************************************************/
	/*                   OpenGL program creation                            */
	/************************************************************************/
	auto tmpProgram = new OGLProgram("shaders/hm02GeomPass.vert", "shaders/hm02GeomPass.frag");

	if (!tmpProgram || !tmpProgram->isOK()) {
		cerr << "Something wrong in shader" << endl;
		delete tmpProgram;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	} else {
		programGeomPassPtr = tmpProgram;
		glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	}

	tmpProgram = new OGLProgram("shaders/hm02FilterPass.vert", "shaders/hm02FilterPass.frag");

	if (!tmpProgram || !tmpProgram->isOK()) {
		cerr << "Something wrong in shader" << endl;
		delete tmpProgram;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	} else {
		programFilterPtr = programNoFilterPtr = tmpProgram;
		glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	}

	tmpProgram = new OGLProgram("shaders/hm02FilterPass.vert", "shaders/hm02Avg3Filter.frag");

	if (!tmpProgram || !tmpProgram->isOK()) {
		cerr << "Something wrong in shader" << endl;
		delete tmpProgram;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	} else {
		programAvg3FilterPtr = tmpProgram;
		glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	}

	tmpProgram = new OGLProgram("shaders/hm02FilterPass.vert", "shaders/hm02Avg5Filter.frag");

	if (!tmpProgram || !tmpProgram->isOK()) {
		cerr << "Something wrong in shader" << endl;
		delete tmpProgram;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	} else {
		programAvg5FilterPtr = tmpProgram;
		glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	}

	tmpProgram = new OGLProgram("shaders/hm02FilterPass.vert", "shaders/hm02EdgeFilter.frag");

	if (!tmpProgram || !tmpProgram->isOK()) {
		cerr << "Something wrong in shader" << endl;
		delete tmpProgram;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	} else {
		programEdgeFilterPtr = tmpProgram;
		glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	}
	
	/* Geometry pass first */
	loc.u_P = programGeomPassPtr->uniformLoc("P");
	loc.u_Time = programGeomPassPtr->uniformLoc("time");
	loc.u_V = programGeomPassPtr->uniformLoc("V");
	loc.u_selected = programGeomPassPtr->uniformLoc("selected");
	loc.u_Texture = programGeomPassPtr->uniformLoc("textureMap");
	loc.a_position = programGeomPassPtr->attribLoc("Position");
	loc.a_normal = programGeomPassPtr->attribLoc("Normal");
	loc.a_texture = programGeomPassPtr->attribLoc("TextCoord");
	loc.a_color = programGeomPassPtr->attribLoc("Color");
	loc.a_transformation = programGeomPassPtr->attribLoc("M");

	/* Filter pass*/
	filoc.u_PVM = programFilterPtr->uniformLoc("PVM");
	filoc.u_colTex = programFilterPtr->uniformLoc("colorTexture");
	filoc.a_position = programFilterPtr->attribLoc("Position");
	filoc.a_normal = programFilterPtr->attribLoc("Normal");
	filoc.a_texture = programFilterPtr->attribLoc("TextCoord");
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
	createFBO(new_window_width, new_window_height);
}

void display() {
	
	geomPass();
	filterPass();

	//Unbind an clean
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	/* You need to call this to draw the GUI, After unbinding your program*/
	drawGUI();
	/* But, before flushing the drawinng commands*/

	glutSwapBuffers();
}

void geomPass() {
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

	//We are gonna render to this FBO
	glBindFramebuffer(GL_FRAMEBUFFER, fbo.id);
	//We need to say that this pass will render two buffers, one with the Phong shading
	//and one with the instances id
	GLenum fbo_textures_to_render[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, fbo_textures_to_render);
	//Make the viewport dimensions match the texture in the FBO
	glViewport(0, 0, fbo.width, fbo.height);
	//Activate antialliasing
	glEnable(GL_MULTISAMPLE);
	//Initialize some basic rendering state
	glEnable(GL_DEPTH_TEST);
	//Dark black background color
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	programGeomPassPtr->use();

	//View
	mat4 V = cam.getViewMatrix() * ball.getRotation();
	//Projection
	mat4 P = cam.getProjectionMatrix();

	/************************************************************************/
	/* Send uniform values to shader                                        */
	/************************************************************************/
	if (loc.u_P != -1) {
		glUniformMatrix4fv(loc.u_P, 1, GL_FALSE, glm::value_ptr(P));
	}
	if (loc.u_V != -1) {
		glUniformMatrix4fv(loc.u_V, 1, GL_FALSE, glm::value_ptr(V));
	}
	if (loc.u_Time != -1) {
		glUniform1f(loc.u_Time, seconds_elapsed);
	}
	if (loc.u_selected != -1) {
		glUniform1i(loc.u_selected, instance_selected);
	}
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer_id);
	if (loc.a_color != -1) {
		glEnableVertexAttribArray(loc.a_color);
		//Colors in this buffer are thigly packes so the zero at the end
		glVertexAttribPointer(loc.a_color, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);
	}
	/* Matrix atribute location actually uses four location one per column
	This code is taken form the OpenGL programming guide pg 130*/
	glBindBuffer(GL_ARRAY_BUFFER, transformation_buffer_id);
	if (loc.a_transformation != -1) {
		for (int i = 0; i < 4; i++) {
			// Set up the vertex attribute
			if (loc.a_transformation + i != -1) {
				// Enable it
				glEnableVertexAttribArray(loc.a_transformation + i);
				glVertexAttribPointer(loc.a_transformation + i, // Location
					4, GL_FLOAT, GL_FALSE, // vec4
					sizeof(mat4), // Stride
					(void *)(sizeof(vec4) * i)); // Start offset
			}
		}
	}

	//Set active texture and bind
	glActiveTexture(GL_TEXTURE0); //Active texture unit 0
	textureMapPtr->bind(); //The next binded texture will be refered with the active texture unit
	if (loc.u_Texture != -1) {
		glUniform1i(loc.u_Texture, 0); // we bound our texture to texture unit 0
	}

	/* Draw */
	meshPtr->drawTriangles(loc.a_position, loc.a_normal, loc.a_texture, instace_number);

	if (loc.a_transformation != -1) {
		for (int i = 0; i < 4; i++) {
			if (loc.a_transformation + i != -1) {
				glDisableVertexAttribArray(loc.a_transformation + i);
			}
		}
	}

	if (loc.a_color != -1) {
		glDisableVertexAttribArray(loc.a_color);
	}

	glUseProgram(0);

	glDisable(GL_MULTISAMPLE);
	//Initialize some basic rendering state
	glDisable(GL_DEPTH_TEST);
}

void filterPass() {
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

	//Do not render this pass to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//Render to back buffer
	glDrawBuffer(GL_BACK);

	//Dark black background color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, fbo.width, fbo.height);
	glClear(GL_COLOR_BUFFER_BIT);
	programFilterPtr->use();

	mat4 I(1.0f);
	//Model
	mat4 M = glm::scale(I, vec3(2.0f));
	//View
	mat4 V = glm::lookAt(vec3(0.0f, 0.0f, 1.0f), vec3(0.0f), vec3(0.0, 1.0, 0.0));
	//Projection
	/*In ortho as in orthoRH, the far and near coordinate are inverted.
	Either pass it inverted or use orthoLH*/
	mat4 P = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f);

	/************************************************************************/
	/* Send uniform values to shader                                        */
	/************************************************************************/
	if (filoc.u_PVM != -1) {
		glUniformMatrix4fv(filoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M));
	}

	//Set active texture and bind
	glActiveTexture(GL_TEXTURE0); //Active texture unit 0
	// NEED to bind the corresponding texture form FBO
	glBindTexture(GL_TEXTURE_2D, fbo.colorTex); //The next binded texture will be refered with the active texture unit
	if (filoc.u_colTex != -1) {
		glUniform1i(filoc.u_colTex, 0); // we bound our texture to texture unit 0
	}

	/* Draw */
	quadMeshPtr->drawTriangles(loc.a_position, loc.a_normal, loc.a_texture);

	
	glUseProgram(0);
	
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
	
	switch (key) {
		case 'R':
		case 'r':
			reload_shaders();
		break;

		case 27:
			exit_glut();
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

	switch (key) {
		case GLUT_KEY_RIGHT:
			filter_option = ++filter_option % 4;
			changeFilter();
		break;

		case GLUT_KEY_LEFT:
			filter_option = (--filter_option + 4) % 4;
			changeFilter();
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
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			ball.startDrag(glm::vec2(mouse_x, mouse_y));
		} else {
			ball.endDrag(glm::vec2(mouse_x, mouse_y));
		}
	} else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		GLubyte pixel[4] = {0};
		//Temporaly bind the fbo for read form pick texture
		glBindFramebuffer(GL_FRAMEBUFFER, fbo.id);
		//Read from pick texture (attachment 1)
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(mouse_x, fbo.height - mouse_y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
		//Finish reading unbind it
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//Use pixel to get the id (background is clear to (38, 38, 38))
		//If the blue component is zero, then we are not in background
		instance_selected = pixel[1] != 0 ? 0 : pixel[0];
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

void createFBO(int width, int height) {
	deleteFBO();
	fbo.width = width;
	fbo.height = height;
	//Create a texture to render pass 1 into
	glGenTextures(1, &fbo.colorTex);
	glBindTexture(GL_TEXTURE_2D, fbo.colorTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, fbo.width, fbo.height, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//Create a texture to store the picking
	glGenTextures(1, &fbo.pickTex);
	glBindTexture(GL_TEXTURE_2D, fbo.pickTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fbo.width, fbo.height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	//Create render buffer for depth.
	glGenRenderbuffers(1, &fbo.depthTex);
	glBindRenderbuffer(GL_RENDERBUFFER, fbo.depthTex);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fbo.width, fbo.height);
	//Create the Frame Buffer object
	glGenFramebuffers(1, &fbo.id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo.id);
	//Attach texture to render into it
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo.colorTex, 0);
	//Attach texture to store the picking ids
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fbo.pickTex, 0);
	//Attach depth buffer to FBO
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo.depthTex);
	if (!ogl::framebufferStatus()) {
		cerr << "Could not create FBO" << endl;
	}
	//Bind the default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void deleteFBO() {
	if (fbo.width != 0 && fbo.height != 0) {
		//Bind the default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//Now, that you are sure it is not binded: delete it
		glDeleteFramebuffers(1, &fbo.id);
		//Delete depth texture
		glDeleteRenderbuffers(1, &fbo.depthTex);
		//Bind default texture
		glBindTexture(GL_TEXTURE_2D, 0);
		//Now, that you are sure they are not binded: delete them
		GLuint textures[2] = {fbo.colorTex, fbo.pickTex};
		glDeleteTextures(2, textures);
	}
}

void changeFilter() {
	switch (filter_option) {
		case 0:
			programFilterPtr = programNoFilterPtr;
		break;

		case 1:
			programFilterPtr = programAvg3FilterPtr;
		break;

		case 2:
			programFilterPtr = programAvg5FilterPtr;
		break;

		case 3:
			programFilterPtr = programEdgeFilterPtr;
		break;
	}
}

void drawGUI() {
	/* Always start with this call*/
	ImGui_ImplGLUT_NewFrame(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	/* Position of the menu, if no imgui.ini exist */
	ImGui::SetNextWindowSize(ImVec2(50, 50), ImGuiSetCond_FirstUseEver);

	/*Create a new menu for my app*/
	ImGui::Begin("Filter options");
	
	ImGui::RadioButton("No filter", &filter_option, 0);
	ImGui::RadioButton("Average 3x3", &filter_option, 1);
	ImGui::RadioButton("Average 5x5", &filter_option, 2);
	ImGui::RadioButton("Edge detection", &filter_option, 3);
	changeFilter();
	//ImGui::ImageButton((ImTextureID)fbo.colorTex, ImVec2(fbo.width * 0.2f, fbo.height * 0.2f), ImVec2(0,0), ImVec2(1, 1), -1, ImVec4(0,0,0,0), ImVec4(1,1,1,1));
	ImGui::Image((ImTextureID)fbo.pickTex, ImVec2(fbo.width * 0.2f, fbo.height * 0.2f), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
	//ImGui::Image((ImTextureID)textureMapPtr->get_id(), ImVec2(textureMapPtr->get_width() * 0.5f, textureMapPtr->get_height() * 0.5f), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
	ImGui::Text("Instance selected %u", instance_selected);
	if (ImGui::Button("Quit")) {
		exit_glut();
	}
	ImGui::End();

	/* End with this when you want to render GUI */
	ImGui::Render();
}