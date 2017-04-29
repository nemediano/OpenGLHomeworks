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
#include "ScreenGrabber.h"
#include "ProceduralTextures.h"

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
Texture* texturePtr = nullptr;
OGLProgram* programPtr = nullptr;
ScreenGrabber grabber;
MatPhong mat;
LightPhong light;

GLint window = 0;

struct Locations {
	// Location for shader variables
	GLint u_PVM = -1;
	GLint u_VM = -1;
	GLint u_NormalMat = -1;
	GLint u_texture = -1;
	GLint u_gamma = -1;
	GLint a_position = -1;
	GLint a_normal = -1;
	GLint a_texture = -1;
	//Lighting related variables
	GLint u_Light_La = -1;
	GLint u_Light_Ls = -1;
	GLint u_Light_Ld = -1;
	//Material related variable
	GLint u_Material_Ka = -1;
	GLint u_Material_Ks = -1;
	GLint u_Material_Kd = -1;
	GLint u_Material_alpha = -1;
};

Locations phongLoc;

//Global variables for the program logic
float seconds_elapsed;
float angle;


float scaleFactor;
glm::vec3 center;
bool gammaCorrection;
bool wireframe;
bool cullFace;

float gamma;
int mode;

std::vector<MatPhong> materials;

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

	if (ImGui::CollapsingHeader("Material editor")) {
		//Edit Material
		vec3 Ka = mat.getKa();
		vec3 Kd = mat.getKd();
		vec3 Ks = mat.getKs();
		float alpha = mat.getAlpha();
		ImGui::ColorEdit3("Ambient", glm::value_ptr(Ka));
		ImGui::Text("R: %.2f G: %.2f B: %.2f", Ka.r, Ka.g, Ka.b);
		ImGui::ColorEdit3("Difusse", glm::value_ptr(Kd));
		ImGui::Text("R: %.2f G: %.2f B: %.2f", Kd.r, Kd.g, Kd.b);
		ImGui::ColorEdit3("Specular", glm::value_ptr(Ks));
		ImGui::Text("R: %.2f G: %.2f B: %.2f", Ks.r, Ks.g, Ks.b);
		ImGui::SliderFloat("Alpha", &alpha, 0.0f, 128.0f, "%.2f", 3.0f);
		mat.setKa(Ka);
		mat.setKd(Kd);
		mat.setKs(Ks);
		mat.setAlpha(alpha);

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

	if (ImGui::CollapsingHeader("Light physical properties")) {
		vec3 La = light.getLa();
		vec3 Ld = light.getLd();
		vec3 Ls = light.getLs();
		ImGui::ColorEdit3("Ambient", glm::value_ptr(La));
		ImGui::Text("R: %.2f G: %.2f B: %.2f", La.r, La.g, La.b);
		ImGui::ColorEdit3("Difusse", glm::value_ptr(Ld));
		ImGui::Text("R: %.2f G: %.2f B: %.2f", Ld.r, Ld.g, Ld.b);
		ImGui::ColorEdit3("Specular", glm::value_ptr(Ls));
		ImGui::Text("R: %.2f G: %.2f B: %.2f", Ls.r, Ls.g, Ls.b);
		light.setLa(La);
		light.setLd(Ld);
		light.setLs(Ls);
	}
	
	ImGui::RadioButton("Full model", &mode, 0); ImGui::SameLine();
	ImGui::RadioButton("Difusse", &mode, 1);
	

	ImGui::Checkbox("Gamma correction", &gammaCorrection);
	ImGui::Checkbox("Wirefreme", &wireframe);
	ImGui::Checkbox("Cull", &cullFace);
	if (gammaCorrection) {
		ImGui::SliderFloat("Gamma", &gamma, 1.0f, 2.5f);
	}
	if (ImGui::Button("Quit")) {
		exit_glut();
	}
	if (ImGui::Button("Reset camera")) {
		cam.setFovY(PI / 4.5f);
		ball.resetRotation();
	}
	if (ImGui::Button("Screenshoot")) {
		grabber.grab();
	}
	if (ImGui::Button("Save Mesh")) {
		meshPtr->save("test.obj");
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
	glutInitWindowSize(1200, 800);
	window = glutCreateWindow("Navigate Mesh");
}

void init_program() {
	using glm::vec3;
	
	/* Then, create primitives (load them from mesh) */
	meshPtr = new Mesh(Geometries::pyramid());


	texturePtr = new Texture(chessBoard());
	texturePtr->save("MyStencil.png");

	if (meshPtr) {
		meshPtr->sendToGPU();
	}
	//Extract info form the mesh
	scaleFactor = meshPtr->scaleFactor();
	center = meshPtr->getBBCenter();

	gammaCorrection = false;
	wireframe = true;
	cullFace = false;
	gamma = 1.0f;
	seconds_elapsed = 0.0f;
	mode = 0;
	
	//Set the default position of the camera
	cam.setLookAt(vec3(0.0f, 0.0f, 2.0f), vec3(0.0f));
	cam.setAspectRatio(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	cam.setFovY(PI / 4.5f);
	cam.setDepthView(0.1f, 5.0f);
	//Create trackball camera
	ball.setWindowSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	//Also setup the image grabber
	grabber.resize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

	mat.setAlpha(128);
	mat.setKa(0.25f * vec3(1.0f, 1.0f, 0.0f));
	mat.setKd(0.75f * vec3(1.0f, 1.0f, 0.0f));
	mat.setKs(vec3(0.5f));

	materials.push_back(EMERALD);
	materials.push_back(JADE);
	materials.push_back(OBSIDIAN);
	materials.push_back(PEARL);
	materials.push_back(RUBY);
	materials.push_back(TURQUOISE);

	materials.push_back(BRASS);
	materials.push_back(BRONZE);
	materials.push_back(CHROME);
	materials.push_back(COPPER);
	materials.push_back(GOLD);
	materials.push_back(SILVER);

	materials.push_back(BLACK_PLASTIC);
	materials.push_back(CYAN_PLASTIC);
	materials.push_back(GREEN_PLASTIC);
	materials.push_back(RED_PLASTIC);
	materials.push_back(WHITE_PLASTIC);
	materials.push_back(YELLOW_PLASTIC);

	materials.push_back(BLACK_RUBBER);
	materials.push_back(CYAN_RUBBER);
	materials.push_back(GREEN_RUBBER);
	materials.push_back(RED_RUBBER);
	materials.push_back(WHITE_RUBBER);
	materials.push_back(YELLOW_RUBBER);
}

void reload_shaders() {
	using std::cout;
	using std::cerr;
	using std::endl;
	/************************************************************************/
	/*                   OpenGL program creation                            */
	/************************************************************************/
	programPtr = new OGLProgram("shaders/phongView.vert", "shaders/phongView.frag");
	if (!programPtr || !programPtr->isOK()) {
		cerr << "Something wrong in shader" << endl;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	} else {
		cout << "Shader compiled" << endl;
		glClearColor(0.85f, 0.85f, 0.85f, 1.0f);
	}

	/************************************************************************/
	/* Allocating variables for shaders                                     */
	/************************************************************************/
	phongLoc.u_PVM = programPtr->uniformLoc("PVM");
	phongLoc.u_VM = programPtr->uniformLoc("VM");;
	phongLoc.u_NormalMat = programPtr->uniformLoc("NormalMat");
	phongLoc.u_gamma = programPtr->uniformLoc("gamma");
	//Light
	phongLoc.u_Light_La = programPtr->uniformLoc("light.La");
	phongLoc.u_Light_Ls = programPtr->uniformLoc("light.Ls");
	phongLoc.u_Light_Ld = programPtr->uniformLoc("light.Ld");
	//Material
	phongLoc.u_Material_Ka = programPtr->uniformLoc("mat.Ka");
	phongLoc.u_Material_Ks = programPtr->uniformLoc("mat.Ks");
	phongLoc.u_Material_Kd = programPtr->uniformLoc("mat.Kd");
	phongLoc.u_Material_alpha = programPtr->uniformLoc("mat.alpha");
	//Vertex attributes location
	phongLoc.a_position = programPtr->attribLoc("Position");
	phongLoc.a_normal = programPtr->attribLoc("Normal");
	phongLoc.a_texture = programPtr->attribLoc("TextCoord");
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

	reload_shaders();

	//Activate antialliasing
	glEnable(GL_MULTISAMPLE);
	
	//Initialize some basic rendering state
	glEnable(GL_DEPTH_TEST);
	
}

void passLightingState() {
	//Light
	glUniform3fv(phongLoc.u_Light_La, 1, glm::value_ptr(light.getLa()));
	glUniform3fv(phongLoc.u_Light_Ls, 1, glm::value_ptr(light.getLs()));
	glUniform3fv(phongLoc.u_Light_Ld, 1, glm::value_ptr(light.getLd()));
	//Material
	if (mode == 0) {
		glUniform3fv(phongLoc.u_Material_Ka, 1, glm::value_ptr(mat.getKa()));
		glUniform3fv(phongLoc.u_Material_Ks, 1, glm::value_ptr(mat.getKs()));
		glUniform3fv(phongLoc.u_Material_Kd, 1, glm::value_ptr(mat.getKd()));
	} else if (mode == 1) {
		glUniform3fv(phongLoc.u_Material_Ka, 1, glm::value_ptr(glm::vec3(0.0f)));
		glUniform3fv(phongLoc.u_Material_Ks, 1, glm::value_ptr(glm::vec3(0.0f)));
		glUniform3fv(phongLoc.u_Material_Kd, 1, glm::value_ptr(mat.getKd()));
	}
	glUniform1f(phongLoc.u_Material_alpha, mat.getAlpha());
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
	using glm::mat4;
	if (wireframe) {
		glPolygonMode(GL_FRONT, GL_LINE);
		glPolygonMode(GL_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_FILL);
	}
	if (cullFace) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	} else {
		glDisable(GL_CULL_FACE);
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	programPtr->use();

	mat4 I(1.0f);
	//Model
	mat4 M = I;
	//M = glm::scale(M, vec3(0.5f, 1.0f, 0.5f));
	M = glm::scale(M, vec3(scaleFactor));
	M = glm::translate(M, -center);
	
	//View
	mat4 V = cam.getViewMatrix() * ball.getRotation();
	//Projection
	mat4 P = cam.getProjectionMatrix();

	/************************************************************************/
	/* Send uniform values to shader                                        */
	/************************************************************************/
	
	glUniformMatrix4fv(phongLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M));
	glUniformMatrix4fv(phongLoc.u_VM, 1, GL_FALSE, glm::value_ptr(V * M));
	glUniformMatrix4fv(phongLoc.u_NormalMat, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(V * M))));
	glUniform1f(phongLoc.u_gamma, gamma);
	//Send light and material
	passLightingState();

	/* Draw */
	meshPtr->drawTriangles(phongLoc.a_position, phongLoc.a_normal, phongLoc.a_texture);

	//Unbind an clean
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	/* You need to call this to draw the GUI, After unbinding your program*/
	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_FILL);
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
			reload_shaders();
		break;

		case 27:
			exit_glut();
		break;

		case ' ':
			grabber.grab();
		break;

		case 'c':
		case 'C':
			cam.setFovY(PI / 8.0f);
			ball.resetRotation();
		break;

		case 'w':
		case 'W':
			wireframe = !wireframe;
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