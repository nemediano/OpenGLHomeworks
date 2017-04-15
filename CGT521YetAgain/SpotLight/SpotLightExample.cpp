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
#include "Spotlight.h"
#include "MatPhong.h"
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
Mesh* cubeBoxPtr = nullptr;
Mesh* currentMeshPtr = nullptr;
OGLProgram* phongPtr = nullptr;
OGLProgram* stencilPtr = nullptr;
OGLProgram* phongShadowPtr = nullptr;
OGLProgram* lighPtr = nullptr;
MatPhong mat;
GLint window = 0;
// Location for shader variables
struct Locations {
	GLint u_PVM = -1;
	GLint u_VM = -1;
	GLint u_P = -1;
	GLint u_V = -1;
	GLint u_M = -1;
	GLint u_NormMat = -1;
	GLint u_ShadowMat = -1;
	GLint u_gamma = -1;

	GLint u_La = -1;
	GLint u_Ls = -1;
	GLint u_Ld = -1;

	GLint u_LightPos = -1;
	GLint u_LightPM = -1;
	GLint u_LightM = -1;
	GLint u_CameraPos = -1;
	GLint u_Stencil = -1;
	GLint u_ShadowMap = -1;

	GLint u_Ka = -1;
	GLint u_Ks = -1;
	GLint u_Kd = -1;
	GLint u_alpha = -1;

	GLint a_position = -1;
	GLint a_normal = -1;
	GLint a_texture = -1;
};

Locations phongLoc;
Locations stencilLoc;
Locations lightLoc;
Locations phongShadowLoc;

struct FBO {
	GLuint id = 0;
	GLuint depth = 0;
	GLuint test = 0;
	int width = 0;
	int height = 0;
};

FBO shadowBuffer;

//Global variables for the program logic
struct LightContainer {
	glm::vec3 eulerAngles;
	float distance;
	LightPhong properties;
	Spotlight spot;
	Texture* stencilPtr;
};

LightContainer light;

float seconds_elapsed;

float scaleFactor;
glm::vec3 meshCenter;

bool gammaCorrection;
bool drawContainer;
float gamma;
bool rotation;
glm::vec3 backgroundColor;
int currentShader;
int currentMesh;

std::vector<MatPhong> materials;

void passLightingState();

void renderGeometryPass();
void generateShadowmapPass();

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

	ImGui::Text("Mode");
	ImGui::RadioButton("Mark spaces", &currentShader, 0); ImGui::SameLine();
	ImGui::RadioButton("Stencil", &currentShader, 1); ImGui::SameLine();
	ImGui::RadioButton("Shadow", &currentShader, 2);

	ImGui::Text("Light position");
	float aperture = light.spot.getAperture();
	ImGui::SliderFloat("Distance", &light.distance, 0.0f, 5.0f);
	ImGui::SliderAngle("Angle X", &light.eulerAngles.x, -180.0f, 180.0f);
	ImGui::SliderAngle("Angle Y", &light.eulerAngles.y, -180.0f, 180.0f);
	ImGui::SliderAngle("Aperture", &aperture, 0.0f, 90.0f);
	light.spot.setAperture(aperture);

	if (ImGui::CollapsingHeader("Light physical properties")) {
		vec3 La = light.properties.getLa();
		vec3 Ld = light.properties.getLd();
		vec3 Ls = light.properties.getLs();
		ImGui::ColorEdit3("Ambient", glm::value_ptr(La));
		ImGui::Text("R: %.2f G: %.2f B: %.2f", La.r, La.g, La.b);
		ImGui::ColorEdit3("Difusse", glm::value_ptr(Ld));
		ImGui::Text("R: %.2f G: %.2f B: %.2f", Ld.r, Ld.g, Ld.b);
		ImGui::ColorEdit3("Specular", glm::value_ptr(Ls));
		ImGui::Text("R: %.2f G: %.2f B: %.2f", Ls.r, Ls.g, Ls.b);
		light.properties.setLa(La);
		light.properties.setLd(Ld);
		light.properties.setLs(Ls);
	}

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

	if (ImGui::CollapsingHeader("General options")) {
		ImGui::Checkbox("Rotation", &rotation);
		ImGui::Checkbox("Gamma correction", &gammaCorrection);
		ImGui::Checkbox("Container box", &drawContainer);
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
	ImGui::Text("Light depth approximation texture");
	ImGui::Image((ImTextureID)shadowBuffer.test, ImVec2(shadowBuffer.width * 0.25f, shadowBuffer.height * 0.25f), ImVec2(0, 1), ImVec2(1, 0), ImColor(255, 255, 255, 255), ImColor(0, 0, 0, 255));
	ImGui::End();

	

	/* End with this when you want to render GUI */
	ImGui::Render();
}

void exit_glut() {
	delete meshPtr;
	delete cubePtr;
	delete spherePtr;
	delete phongPtr;
	delete stencilPtr;
	delete light.stencilPtr;
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
	window = glutCreateWindow("Spoth light example");
}

void init_program() {
	using glm::vec3;
	using namespace lighting;
	
	/* Load assets */
	meshPtr = new Mesh("../models/teapot.obj");
	spherePtr = new Mesh(Geometries::icosphere(3));
	cubePtr = new Mesh(Geometries::cube());
	cubeBoxPtr = new Mesh(Geometries::insideOutCube());

	light.stencilPtr = new Texture("../img/newLight.png");

	if (meshPtr) {
		meshPtr->sendToGPU();
	}

	if (spherePtr) {
		spherePtr->sendToGPU();
	}

	if (cubePtr) {
		cubePtr->sendToGPU();
	}

	if (cubeBoxPtr) {
		cubeBoxPtr->sendToGPU();
	}

	//Extract info form the mesh
	scaleFactor = meshPtr->scaleFactor();
	meshCenter = meshPtr->getBBCenter();

	seconds_elapsed = 0.0f;

	

	//Create FBO for storing the shadow map
	shadowBuffer.width = shadowBuffer.height = 1024;
	//Create the texture for depth first
	glGenTextures(1, &shadowBuffer.depth);
	glBindTexture(GL_TEXTURE_2D, shadowBuffer.depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, shadowBuffer.width, shadowBuffer.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//Create the texture for testing first
	glGenTextures(1, &shadowBuffer.test);
	glBindTexture(GL_TEXTURE_2D, shadowBuffer.test);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, shadowBuffer.width, shadowBuffer.height, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
	//Create framebuffer second and attach the texture to it
	glGenFramebuffers(1, &shadowBuffer.id);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer.id);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowBuffer.test, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowBuffer.depth, 0);
	//glDrawBuffer(GL_NONE);
	
	ogl::framebufferStatus();

	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	//Set the default position of the camera
	cam.setLookAt(vec3(0.0f, 0.0f, 1.5f), vec3(0.0f));
	cam.setAspectRatio(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	cam.setFovY(PI / 4.0f);
	cam.setDepthView(0.1f, 4.0f);
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
	light.spot.setAperture(PI / 12.0f);
	//Default material
	mat.setKa(vec3(0.122f, 0.216f, 0.145f));
	mat.setKd(vec3(0.4f, 0.742f, 0.7f));
	mat.setKs(vec3(0.48f, 0.188f, 0.447f));
	mat.setAlpha(3.36f);

	backgroundColor = vec3(0.15f);

	currentShader = 1;
	currentMesh = 0;
	gammaCorrection = false;
	drawContainer = true;
	gamma = 1.0f;

	/*Load predefined materials*/

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
	using glm::vec3;

	phongPtr = new OGLProgram("shaders/phong.vert", "shaders/phong.frag");
	stencilPtr = new OGLProgram("shaders/phong.vert", "shaders/stencil.frag");
	lighPtr = new OGLProgram("shaders/genShadowMap.vert", "shaders/genShadowMap.frag");
	phongShadowPtr = new OGLProgram("shaders/phongShadow.vert", "shaders/phongShadow.frag");

	//phongShadowLoc
	if (!phongPtr || !phongPtr->isOK()) {
		cerr << "Something wrong in Phong world shader" << endl;
		backgroundColor = vec3(1.0f, 0.0f, 1.0f);
	}

	if (!stencilPtr || !stencilPtr->isOK()) {
		cerr << "Something wrong in Stencil shader" << endl;
		backgroundColor = vec3(1.0f, 0.0f, 1.0f);
	}

	if (!phongShadowPtr || !phongShadowPtr->isOK()) {
		cerr << "Something wrong in Phong shadow shader" << endl;
		backgroundColor = vec3(1.0f, 0.0f, 1.0f);
	}

	if (!lighPtr || !lighPtr->isOK()) {
		cerr << "Something wrong in light shader" << endl;
		backgroundColor = vec3(1.0f, 0.0f, 1.0f);
	}

	if (phongPtr->isOK() && stencilPtr->isOK() && lighPtr->isOK() && phongShadowPtr->isOK()) {
		cout << "Shaders compiled correctlly!" << endl;
		backgroundColor = 0.15f * vec3(1.0f);
	}

	/************************************************************************/
	/* Allocating variables for shaders                                     */
	/************************************************************************/
	//Uniforms
	phongLoc.u_PVM = phongPtr->uniformLoc("PVM");
	phongLoc.u_M = phongPtr->uniformLoc("M");
	phongLoc.u_NormMat = phongPtr->uniformLoc("NormalMat");
	phongLoc.u_gamma = phongPtr->uniformLoc("gamma");
	phongLoc.u_CameraPos = phongPtr->uniformLoc("cameraPosition");
	//Light
	phongLoc.u_LightPos = phongPtr->uniformLoc("light.position");
	phongLoc.u_LightPM = phongPtr->uniformLoc("light.PM");
	phongLoc.u_LightM = phongPtr->uniformLoc("light.M");
	phongLoc.u_La = phongPtr->uniformLoc("light.La");
	phongLoc.u_Ld = phongPtr->uniformLoc("light.Ld");
	phongLoc.u_Ls = phongPtr->uniformLoc("light.Ls");
	//Material
	phongLoc.u_alpha = phongPtr->uniformLoc("mat.alpha");
	phongLoc.u_Ka = phongPtr->uniformLoc("mat.Ka");
	phongLoc.u_Kd = phongPtr->uniformLoc("mat.Kd");
	phongLoc.u_Ks = phongPtr->uniformLoc("mat.Ks");
	//Atrributes
	phongLoc.a_position = phongPtr->attribLoc("Position");
	phongLoc.a_normal = phongPtr->attribLoc("Normal");
	phongLoc.a_texture = phongPtr->attribLoc("TextCoord");


	//Uniforms
	stencilLoc.u_PVM = stencilPtr->uniformLoc("PVM");
	stencilLoc.u_M = stencilPtr->uniformLoc("M");
	stencilLoc.u_NormMat = stencilPtr->uniformLoc("NormalMat");
	stencilLoc.u_gamma = stencilPtr->uniformLoc("gamma");
	stencilLoc.u_CameraPos = stencilPtr->uniformLoc("cameraPosition");
	stencilLoc.u_Stencil = stencilPtr->uniformLoc("lightStencil");
	//Light
	stencilLoc.u_LightPos = stencilPtr->uniformLoc("light.position");
	stencilLoc.u_LightPM = stencilPtr->uniformLoc("light.PM");
	stencilLoc.u_LightM = stencilPtr->uniformLoc("light.M");
	stencilLoc.u_La = stencilPtr->uniformLoc("light.La");
	stencilLoc.u_Ld = stencilPtr->uniformLoc("light.Ld");
	stencilLoc.u_Ls = stencilPtr->uniformLoc("light.Ls");
	//Material
	stencilLoc.u_alpha = stencilPtr->uniformLoc("mat.alpha");
	stencilLoc.u_Ka = stencilPtr->uniformLoc("mat.Ka");
	stencilLoc.u_Kd = stencilPtr->uniformLoc("mat.Kd");
	stencilLoc.u_Ks = stencilPtr->uniformLoc("mat.Ks");
	//Atrributes
	stencilLoc.a_position = stencilPtr->attribLoc("Position");
	stencilLoc.a_normal = stencilPtr->attribLoc("Normal");
	stencilLoc.a_texture = stencilPtr->attribLoc("TextCoord");
	

	//Uniforms
	phongShadowLoc.u_PVM = phongShadowPtr->uniformLoc("PVM");
	phongShadowLoc.u_M = phongShadowPtr->uniformLoc("M");
	phongShadowLoc.u_NormMat = phongShadowPtr->uniformLoc("NormalMat");
	phongShadowLoc.u_ShadowMat = phongShadowPtr->uniformLoc("ShadowMat");
	phongShadowLoc.u_gamma = phongShadowPtr->uniformLoc("gamma");
	phongShadowLoc.u_CameraPos = phongShadowPtr->uniformLoc("cameraPosition");
	phongShadowLoc.u_Stencil = phongShadowPtr->uniformLoc("lightStencil");
	phongShadowLoc.u_ShadowMap = phongShadowPtr->uniformLoc("shadowMap");
	//Light
	phongShadowLoc.u_LightPos = phongShadowPtr->uniformLoc("light.position");
	phongShadowLoc.u_LightPM = phongShadowPtr->uniformLoc("light.PM");
	phongShadowLoc.u_LightM = phongShadowPtr->uniformLoc("light.M");
	phongShadowLoc.u_La = phongShadowPtr->uniformLoc("light.La");
	phongShadowLoc.u_Ld = phongShadowPtr->uniformLoc("light.Ld");
	phongShadowLoc.u_Ls = phongShadowPtr->uniformLoc("light.Ls");
	//Material
	phongShadowLoc.u_alpha = phongShadowPtr->uniformLoc("mat.alpha");
	phongShadowLoc.u_Ka = phongShadowPtr->uniformLoc("mat.Ka");
	phongShadowLoc.u_Kd = phongShadowPtr->uniformLoc("mat.Kd");
	phongShadowLoc.u_Ks = phongShadowPtr->uniformLoc("mat.Ks");
	//Atrributes
	phongShadowLoc.a_position = phongShadowPtr->attribLoc("Position");
	phongShadowLoc.a_normal = phongShadowPtr->attribLoc("Normal");
	phongShadowLoc.a_texture = phongShadowPtr->attribLoc("TextCoord");


	//For the shadowmap creation
	lightLoc.u_PVM = lighPtr->uniformLoc("PVM");
	lightLoc.a_position = lighPtr->attribLoc("Position");

}

void passLightingState() {
	using glm::vec3;
	if (currentShader == 0) {
		glUniform3fv(phongLoc.u_Ka, 1, glm::value_ptr(mat.getKa()));
		glUniform3fv(phongLoc.u_Ks, 1, glm::value_ptr(mat.getKs()));
		glUniform3fv(phongLoc.u_Kd, 1, glm::value_ptr(mat.getKd()));
		glUniform1f(phongLoc.u_alpha, mat.getAlpha());
		glUniform3fv(phongLoc.u_La, 1, glm::value_ptr(light.properties.getLa()));
		glUniform3fv(phongLoc.u_Ls, 1, glm::value_ptr(light.properties.getLs()));
		glUniform3fv(phongLoc.u_Ld, 1, glm::value_ptr(light.properties.getLd()));
	} else if (currentShader == 1) {
		glUniform3fv(stencilLoc.u_Ka, 1, glm::value_ptr(mat.getKa()));
		glUniform3fv(stencilLoc.u_Ks, 1, glm::value_ptr(mat.getKs()));
		glUniform3fv(stencilLoc.u_Kd, 1, glm::value_ptr(mat.getKd()));		
		glUniform1f(stencilLoc.u_alpha, mat.getAlpha());
		glUniform3fv(stencilLoc.u_La, 1, glm::value_ptr(light.properties.getLa()));
		glUniform3fv(stencilLoc.u_Ls, 1, glm::value_ptr(light.properties.getLs()));
		glUniform3fv(stencilLoc.u_Ld, 1, glm::value_ptr(light.properties.getLd()));
	} else if (currentShader == 2) {
		glUniform3fv(phongShadowLoc.u_Ka, 1, glm::value_ptr(mat.getKa()));
		glUniform3fv(phongShadowLoc.u_Ks, 1, glm::value_ptr(mat.getKs()));
		glUniform3fv(phongShadowLoc.u_Kd, 1, glm::value_ptr(mat.getKd()));
		glUniform1f(phongShadowLoc.u_alpha, mat.getAlpha());
		glUniform3fv(phongShadowLoc.u_La, 1, glm::value_ptr(light.properties.getLa()));
		glUniform3fv(phongShadowLoc.u_Ls, 1, glm::value_ptr(light.properties.getLs()));
		glUniform3fv(phongShadowLoc.u_Ld, 1, glm::value_ptr(light.properties.getLd()));
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
	//Select mesh to render
	change_mesh();

	generateShadowmapPass();
	renderGeometryPass();
	
	//Unbind an clean
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);

	/* You need to call this to draw the GUI, After unbinding your program*/
	drawGUI();
	/* But, before flushing the drawinng commands*/

	glutSwapBuffers();
}

void renderGeometryPass() {
	using glm::vec3;
	using glm::vec4;
	using glm::vec2;
	using glm::mat4;

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
	light.spot.setPosition(light_position);
	light.spot.setTarget(vec3(0.0f));

	//We are going to render to the default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	glDrawBuffer(GL_BACK);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (currentShader == 0) {
		phongPtr->use();
		/************************************************************************/
		/* Send uniform values to shader                                        */
		/************************************************************************/
		glUniformMatrix4fv(phongLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M));
		glUniformMatrix4fv(phongLoc.u_M, 1, GL_FALSE, glm::value_ptr(M));
		glUniformMatrix4fv(phongLoc.u_NormMat, 1, GL_FALSE, glm::value_ptr(glm::inverse(glm::transpose(M))));
		glUniform1f(phongLoc.u_gamma, gamma);
		glUniform3fv(phongLoc.u_LightPos, 1, glm::value_ptr(light_position));
		glUniformMatrix4fv(phongLoc.u_LightPM, 1, GL_FALSE, glm::value_ptr(light.spot.getPM()));
		glUniformMatrix4fv(phongLoc.u_LightM, 1, GL_FALSE, glm::value_ptr(light.spot.getM()));
		vec4 camera_pos = vec4(cam.getPosition(), 1.0f);
		vec3 camPosInWorld = vec3(glm::inverse(V) * camera_pos);
		glUniform3fv(phongLoc.u_CameraPos, 1, glm::value_ptr(vec3(camPosInWorld)));

		//Send light and material
		passLightingState();
		/* Draw */
		currentMeshPtr->drawTriangles(phongLoc.a_position, phongLoc.a_normal, phongLoc.a_texture);

		//Draw the exterior box
		if (drawContainer) {
			mat4 M_box = mat4(1.0f);
			M_box = glm::scale(M_box, vec3(3.0f));
			glUniformMatrix4fv(phongLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M_box));
			glUniformMatrix4fv(phongLoc.u_M, 1, GL_FALSE, glm::value_ptr(M_box));
			glUniformMatrix4fv(phongLoc.u_NormMat, 1, GL_FALSE, glm::value_ptr(glm::inverse(glm::transpose(M_box))));
			glUniform3fv(phongLoc.u_Ka, 1, glm::value_ptr(vec3(0.25f)));
			glUniform3fv(phongLoc.u_Ks, 1, glm::value_ptr(vec3(0.0f)));
			glUniform3fv(phongLoc.u_Kd, 1, glm::value_ptr(vec3(0.75f)));
			glUniform1f(phongLoc.u_alpha, 1.0f);
			cubeBoxPtr->drawTriangles(phongLoc.a_position, phongLoc.a_normal, phongLoc.a_texture);
		}


	} else if (currentShader == 1) {
		stencilPtr->use();
		/************************************************************************/
		/* Send uniform values to shader                                        */
		/************************************************************************/
		glUniformMatrix4fv(stencilLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M));
		glUniformMatrix4fv(stencilLoc.u_M, 1, GL_FALSE, glm::value_ptr(M));
		glUniformMatrix4fv(stencilLoc.u_NormMat, 1, GL_FALSE, glm::value_ptr(glm::inverse(glm::transpose(M))));
		glUniform1f(stencilLoc.u_gamma, gamma);
		glUniform3fv(stencilLoc.u_LightPos, 1, glm::value_ptr(light_position));
		glUniformMatrix4fv(stencilLoc.u_LightPM, 1, GL_FALSE, glm::value_ptr(light.spot.getPM()));
		glUniformMatrix4fv(stencilLoc.u_LightM, 1, GL_FALSE, glm::value_ptr(light.spot.getM()));
		vec4 camera_pos = vec4(cam.getPosition(), 1.0f);
		vec3 camPosInWorld = vec3(glm::inverse(V) * camera_pos);
		glUniform3fv(stencilLoc.u_CameraPos, 1, glm::value_ptr(vec3(camPosInWorld)));
		glActiveTexture(GL_TEXTURE0); //Active texture unit 0
		glBindTexture(GL_TEXTURE_2D, light.stencilPtr->get_id()); //The next binded texture will be refered with the active texture unit
		if (stencilLoc.u_Stencil != -1) {
			glUniform1i(stencilLoc.u_Stencil, 0); // we bound our texture to texture unit 0
		}
		//Send light and material
		passLightingState();
		/* Draw */
		currentMeshPtr->drawTriangles(stencilLoc.a_position, stencilLoc.a_normal, stencilLoc.a_texture);

		if (drawContainer) {
			//Draw the exterior box
			mat4 M_box = mat4(1.0f);
			M_box = glm::scale(M_box, vec3(3.0f));
			glUniformMatrix4fv(stencilLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M_box));
			glUniformMatrix4fv(stencilLoc.u_M, 1, GL_FALSE, glm::value_ptr(M_box));
			glUniformMatrix4fv(stencilLoc.u_NormMat, 1, GL_FALSE, glm::value_ptr(glm::inverse(glm::transpose(M_box))));
			glUniform3fv(stencilLoc.u_Ka, 1, glm::value_ptr(vec3(0.25f)));
			glUniform3fv(stencilLoc.u_Ks, 1, glm::value_ptr(vec3(0.0f)));
			glUniform3fv(stencilLoc.u_Kd, 1, glm::value_ptr(vec3(0.75f)));
			glUniform1f(stencilLoc.u_alpha, 1.0f);
			cubeBoxPtr->drawTriangles(stencilLoc.a_position, stencilLoc.a_normal, stencilLoc.a_texture);
		}

	} else if (currentShader == 2) {
		phongShadowPtr->use();
		/************************************************************************/
		/* Send uniform values to shader                                        */
		/************************************************************************/
		
		mat4 biasMat = glm::scale(I, vec3(0.5f));
		biasMat = glm::translate(biasMat, vec3(0.5f));
		mat4 shadowMat = biasMat * light.spot.getPM() * M;
		glUniformMatrix4fv(phongShadowLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M));
		glUniformMatrix4fv(phongShadowLoc.u_M, 1, GL_FALSE, glm::value_ptr(M));
		glUniformMatrix4fv(phongShadowLoc.u_ShadowMat, 1, GL_FALSE, glm::value_ptr(glm::inverse(glm::transpose(shadowMat))));
		glUniformMatrix4fv(phongShadowLoc.u_NormMat, 1, GL_FALSE, glm::value_ptr(glm::inverse(glm::transpose(M))));
		glUniform1f(phongShadowLoc.u_gamma, gamma);
		glUniform3fv(phongShadowLoc.u_LightPos, 1, glm::value_ptr(light_position));
		glUniformMatrix4fv(phongShadowLoc.u_LightPM, 1, GL_FALSE, glm::value_ptr(light.spot.getPM()));
		glUniformMatrix4fv(phongShadowLoc.u_LightM, 1, GL_FALSE, glm::value_ptr(light.spot.getM()));
		vec4 camera_pos = vec4(cam.getPosition(), 1.0f);
		vec3 camPosInWorld = vec3(glm::inverse(V) * camera_pos);
		glUniform3fv(phongShadowLoc.u_CameraPos, 1, glm::value_ptr(vec3(camPosInWorld)));
		glActiveTexture(GL_TEXTURE0); //Active texture unit 0
		glBindTexture(GL_TEXTURE_2D, light.stencilPtr->get_id()); //The next binded texture will be refered with the active texture unit
		if (phongShadowLoc.u_Stencil != -1) {
			glUniform1i(phongShadowLoc.u_Stencil, 0); // we bound our texture to texture unit 0
		}
		glActiveTexture(GL_TEXTURE1); //Active texture unit 1
		glBindTexture(GL_TEXTURE_2D, shadowBuffer.depth); //The next binded texture will be refered with the active texture unit
		if (phongShadowLoc.u_ShadowMap != -1) {
			glUniform1i(phongShadowLoc.u_ShadowMap, 1); // we bound our texture to texture unit 0
		}
		//Send light and material
		passLightingState();
		/* Draw */
		currentMeshPtr->drawTriangles(phongShadowLoc.a_position, phongShadowLoc.a_normal, phongShadowLoc.a_texture);

		if (drawContainer) {
			//Draw the exterior box
			mat4 M_box = mat4(1.0f);
			M_box = glm::scale(M_box, vec3(3.0f));
			mat4 shadowMat = biasMat * light.spot.getPM() * M_box;
			glUniformMatrix4fv(phongShadowLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M_box));
			glUniformMatrix4fv(phongShadowLoc.u_M, 1, GL_FALSE, glm::value_ptr(M_box));
			glUniformMatrix4fv(phongShadowLoc.u_NormMat, 1, GL_FALSE, glm::value_ptr(glm::inverse(glm::transpose(M_box))));
			glUniformMatrix4fv(phongShadowLoc.u_ShadowMat, 1, GL_FALSE, glm::value_ptr(glm::inverse(glm::transpose(shadowMat))));
			glUniform3fv(phongShadowLoc.u_Ka, 1, glm::value_ptr(vec3(0.25f)));
			glUniform3fv(phongShadowLoc.u_Ks, 1, glm::value_ptr(vec3(0.0f)));
			glUniform3fv(phongShadowLoc.u_Kd, 1, glm::value_ptr(vec3(0.75f)));
			glUniform1f(phongShadowLoc.u_alpha, 1.0f);
			cubeBoxPtr->drawTriangles(phongShadowLoc.a_position, phongShadowLoc.a_normal, phongShadowLoc.a_texture);
		}

	}
}
void generateShadowmapPass() {
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;
	//We render to the shadowmap
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer.id);
	glViewport(0, 0, shadowBuffer.width, shadowBuffer.height);
	//We only need the depth in the shadow map
	//glDrawBuffer(GL_NONE);
	glDrawBuffer(GL_BACK);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Enable Polygon offset for fighting against Peter Panning
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2.0f, 4.0f);

	//Calculate Light position
	mat4 I(1.0f);
	vec4 light_initial_pos = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	mat4 T = glm::translate(I, vec3(0.0f, 0.0f, light.distance));
	vec3 light_position = vec3(glm::eulerAngleXYZ(light.eulerAngles.x, light.eulerAngles.y, light.eulerAngles.z) * T * light_initial_pos);
	light.spot.setPosition(light_position);
	light.spot.setTarget(vec3(0.0f));
	
	//View Projection, sice we have updated the light position.
	/*This is correct since for tthe light is his position or Model, 
	but from the shader program is the cammera placement or his View.*/
	mat4 PV = light.spot.getPM();

	lighPtr->use();

	mat4 M = rotation ? glm::rotate(I, TAU / 10.0f * seconds_elapsed, vec3(0.0f, 1.0f, 0.0f)) : I;
	M = glm::scale(M, vec3(scaleFactor));
	M = glm::translate(M, -meshCenter);
	glUniformMatrix4fv(lightLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(PV * M));

	currentMeshPtr->drawTriangles(lightLoc.a_position, -1, -1);
	if (drawContainer) {
		mat4 M_box = mat4(1.0f);
		M_box = glm::scale(M_box, vec3(3.0f));
		glUniformMatrix4fv(lightLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(PV * M_box));
		cubeBoxPtr->drawTriangles(lightLoc.a_position, -1, -1);
	}
	//Return to no polygon offset after this draw
	glDisable(GL_POLYGON_OFFSET_FILL);
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
			currentShader = 0;
		break;

		case GLUT_KEY_F2:
			currentShader = 1;
		break;

		case GLUT_KEY_F3:
			currentShader = 2;
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