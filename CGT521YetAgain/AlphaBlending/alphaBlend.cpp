#include "stdafx.h"

/* Include heders and namespaces from my CGT Library */
#include "OGLHelpers.h"
#include "MathConstants.h"
#include "Texture.h"
#include "Mesh.h"
#include "Camera.h"
#include "Trackball.h"
#include "OGLProgram.h"
#include "Material.h"
#include "DisneyLight.h"
#include "Geometries.h"
#include "ScreenGrabber.h"
#include "ProceduralTextures.h"
#include "Spotlight.h"

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
Mesh* meshPtr = nullptr;
Texture* meshTexturePtr = nullptr;
Texture* backgroundTexturePtr = nullptr;
OGLProgram* backgroundProgPtr = nullptr;
OGLProgram* meshRenderProgPtr = nullptr;
ScreenGrabber grabber;
Material mat;

struct LightContainer {
	glm::vec3 eulerAngles;
	float distance;
	Spotlight g;
	DisneyLight p;
};

LightContainer light;

GLint window = 0;

struct Locations {
	//Vertex unifroms
	GLint u_PVM = -1;
	GLint u_M = -1;
	GLint u_NormalMat = -1;
	//Fragment uniforms
	GLint u_texture = -1;
	GLint u_gamma = -1;
	GLint u_cameraPos = -1;
	GLint u_textureMap = -1;
	//Vertex attributes
	GLint a_position = -1;
	GLint a_normal = -1;
	GLint a_texture = -1;
	//Lighting related variables
	GLint u_lightPosition = -1;
	GLint u_lightIntensity = -1;
	GLint u_lightColor = -1;
	GLint u_lightRatio = -1;
	//Material related variable
	GLint u_metalicity = -1;
	GLint u_roughness = -1;
	GLint u_F0 = -1;
	GLint u_base_color = -1;
};

Locations meshRenderLoc;


//Global variables for the program logic
float seconds_elapsed;
float angle;
float ratio;

bool gammaCorrection;
bool background;

float gamma;

void passLightingState();
void reload_shaders();
void create_glut_window();
void init_program();
void init_OpenGL();
void create_glut_callbacks();
void exit_glut();

//Render functions
void renderMesh();

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
	if (ImGui::CollapsingHeader("Light position")) {
		float aperture = light.g.getAperture();
		ImGui::SliderFloat("Distance", &light.distance, 0.0f, 5.0f);
		ImGui::SliderAngle("Angle X", &light.eulerAngles.x, -180.0f, 180.0f);
		ImGui::SliderAngle("Angle Y", &light.eulerAngles.y, -180.0f, 180.0f);
		ImGui::SliderAngle("Aperture", &aperture, 0.0f, 90.0f);
		light.g.setAperture(aperture);
	}

	if (ImGui::CollapsingHeader("Light physical properties")) {
		vec3 color = light.p.getColor();
		float intensity = light.p.getIntensity();
		ImGui::ColorEdit3("Color", glm::value_ptr(color));
		ImGui::Text("R: %.2f G: %.2f B: %.2f", color.r, color.g, color.b);
		ImGui::SliderFloat("Intensity", &intensity, 0.0f, 1.0f);
		light.p.setColor(color);
		light.p.setIntensity(intensity);
		ImGui::SliderFloat("Ratio", &ratio, 0.0f, 1.0f);
	}

	if (ImGui::CollapsingHeader("Material editor")) {
		//Edit Material
		vec3 baseColor = mat.getBaseColor();
		vec3 F0 = mat.getF0();
		float metalicity = mat.getMetalicity();
		float roughness = mat.getRoughness();
		ImGui::ColorEdit3("Base color", glm::value_ptr(baseColor));
		ImGui::Text("R: %.2f G: %.2f B: %.2f", baseColor.r, baseColor.g, baseColor.b);
		ImGui::ColorEdit3("Specular Color", glm::value_ptr(F0));
		ImGui::Text("R: %.2f G: %.2f B: %.2f", F0.r, F0.g, F0.b);
		ImGui::SliderFloat("Metalicity", &metalicity, 0.0f, 1.0f, "%.3f");
		ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f, "%.3f");
		mat.setBaseColor(baseColor);
		mat.setF0(F0);
		mat.setRoughness(roughness);
		mat.setMetalicity(metalicity);
	}

	ImGui::Checkbox("Texture map", &background);
	ImGui::Checkbox("Gamma correction", &gammaCorrection);	
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
	if (ImGui::Button("Recompile shaders")) {
		reload_shaders();
	}
	ImGui::End();

	/* End with this when you want to render GUI */
	ImGui::Render();
}

void exit_glut() {
	delete meshPtr;
	delete meshTexturePtr;
	delete backgroundTexturePtr;
	delete backgroundProgPtr;
	delete meshRenderProgPtr;
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
	window = glutCreateWindow("Alpha Blending example");
}

void init_program() {
	using glm::vec3;

	/* Then, create primitives (load them from mesh) */
	meshPtr = new Mesh("../models/Amago.obj");
	meshTexturePtr = new Texture("../models/AmagoTexture.png");
	backgroundTexturePtr = new Texture("../models/world32k.jpg");
	
	if (meshPtr) {
		//Scale mesh to a unit cube, before sending it to GPU
		meshPtr->toUnitCube();
		meshPtr->sendToGPU();
	}

	if (meshTexturePtr) {
		meshTexturePtr->send_to_gpu();
	}

	if (backgroundTexturePtr) {
		backgroundTexturePtr->send_to_gpu();
	}


	gammaCorrection = false;
	background = false;
	gamma = 1.0f;
	seconds_elapsed = 0.0f;
	

	//Set the default position of the camera
	cam.setLookAt(vec3(0.0f, 0.0f, 2.5f), vec3(0.0f));
	cam.setAspectRatio(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	cam.setFovY(PI / 4.5f);
	cam.setDepthView(0.1f, 5.0f);
	//Create trackball camera
	ball.setWindowSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	//Also setup the image grabber
	grabber.resize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

	//Default position of the point light
	light.eulerAngles = glm::vec3(0.0f, 0.0f, 0.0f);
	light.distance = glm::sqrt(3.0f);
	light.p.setColor(vec3(1.0f));
	light.p.setIntensity(1.0f);
	light.g.setAperture(PI / 6.0f);
	ratio = 0.25;

	//Default material
	mat.setBaseColor(vec3(0.41f, 0.84f, 0.37f));
	mat.setF0(vec3(0.75f));
	mat.setMetalicity(0.2f);
	mat.setRoughness(0.15f);
}

void reload_shaders() {
	using std::cout;
	using std::cerr;
	using std::endl;
	/************************************************************************/
	/*                   OpenGL program creation                            */
	/************************************************************************/
	meshRenderProgPtr = new OGLProgram("shaders/disneyWorld.vert", "shaders/disneyWorld.frag");
	backgroundProgPtr = new OGLProgram("shaders/disneyWorld.vert", "shaders/disneyWorld.frag");

	if (!meshRenderProgPtr || !meshRenderProgPtr->isOK()) {
		cerr << "Something wrong in mesh render shader" << endl;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	}

	if (!backgroundProgPtr || !backgroundProgPtr->isOK()) {
		cerr << "Something wrong in background render shader" << endl;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	}

	if (meshRenderProgPtr->isOK() && backgroundProgPtr->isOK()) {
		cout << "Shaders compiled succesufully" << endl;
		glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	}

	
	/************************************************************************/
	/* Allocating variables for shaders                                     */
	/************************************************************************/
	meshRenderLoc.a_position = meshRenderProgPtr->attribLoc("Position");
	meshRenderLoc.a_normal = meshRenderProgPtr->attribLoc("Normal");
	meshRenderLoc.a_texture = meshRenderProgPtr->attribLoc("TextCoord");

	meshRenderLoc.u_PVM = meshRenderProgPtr->uniformLoc("PVM");
	meshRenderLoc.u_M = meshRenderProgPtr->uniformLoc("M");
	meshRenderLoc.u_NormalMat = meshRenderProgPtr->uniformLoc("NormalMat");

	meshRenderLoc.u_gamma = meshRenderProgPtr->uniformLoc("gamma");
	meshRenderLoc.u_cameraPos = meshRenderProgPtr->uniformLoc("cameraPosition");
	meshRenderLoc.u_textureMap = meshRenderProgPtr->uniformLoc("diffuseMap");

	meshRenderLoc.u_metalicity = meshRenderProgPtr->uniformLoc("mat.metalicity");
	meshRenderLoc.u_roughness = meshRenderProgPtr->uniformLoc("mat.roughness");
	meshRenderLoc.u_base_color = meshRenderProgPtr->uniformLoc("mat.baseColor");
	meshRenderLoc.u_F0 = meshRenderProgPtr->uniformLoc("mat.F0");

	meshRenderLoc.u_lightPosition = meshRenderProgPtr->uniformLoc("light.position");
	meshRenderLoc.u_lightColor = meshRenderProgPtr->uniformLoc("light.color");
	meshRenderLoc.u_lightIntensity = meshRenderProgPtr->uniformLoc("light.intensity");
	meshRenderLoc.u_lightRatio = meshRenderProgPtr->uniformLoc("light.ratio");

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
	glUniform3fv(meshRenderLoc.u_lightPosition, 1, glm::value_ptr(light.g.getPosition()));
	glUniform3fv(meshRenderLoc.u_lightColor, 1, glm::value_ptr(light.p.getColor()));
	glUniform1f(meshRenderLoc.u_lightIntensity, light.p.getIntensity());
	glUniform1f(meshRenderLoc.u_lightRatio, ratio);
	//Material
	glUniform1f(meshRenderLoc.u_metalicity, mat.getMetalicity());
	glUniform1f(meshRenderLoc.u_roughness, mat.getRoughness());
	glUniform3fv(meshRenderLoc.u_base_color, 1, glm::value_ptr(mat.getBaseColor()));
	glUniform3fv(meshRenderLoc.u_F0, 1, glm::value_ptr(mat.getF0()));
	
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

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	renderMesh();


	//Unbind an clean
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	/* You need to call this to draw the GUI, After unbinding your program*/
	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_FILL);
	drawGUI();
	/* But, before flushing the drawinng commands*/

	glutSwapBuffers();
}

void renderMesh() {
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

	meshRenderProgPtr->use();

	mat4 I(1.0f);
	//Model
	mat4 M = glm::scale(I, vec3(1.5f));
	//View
	mat4 V = cam.getViewMatrix() * ball.getRotation();
	//Projection
	mat4 P = cam.getProjectionMatrix();

	//Calculate Light position
	vec4 light_initial_pos = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	mat4 T = glm::translate(I, vec3(0.0f, 0.0f, light.distance));
	vec3 light_position = vec3(glm::eulerAngleXYZ(light.eulerAngles.x, light.eulerAngles.y, light.eulerAngles.z) * T * light_initial_pos);
	light.g.setPosition(light_position);
	light.g.setTarget(vec3(0.0f));


	/************************************************************************/
	/* Send uniform values to shader                                        */
	/************************************************************************/

	glUniformMatrix4fv(meshRenderLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M));
	glUniformMatrix4fv(meshRenderLoc.u_M, 1, GL_FALSE, glm::value_ptr(M));
	glUniformMatrix4fv(meshRenderLoc.u_NormalMat, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(M))));
	glUniform1f(meshRenderLoc.u_gamma, gammaCorrection ? gamma : 1.0f);

	glUniform3fv(meshRenderLoc.u_lightPosition, 1, glm::value_ptr(light.g.getPosition()));
	vec4 camera_pos = vec4(cam.getPosition(), 1.0f);
	vec3 camPosInWorld = vec3(glm::inverse(V) * camera_pos);
	glUniform3fv(meshRenderLoc.u_cameraPos, 1, glm::value_ptr(vec3(camPosInWorld)));
	glActiveTexture(GL_TEXTURE0); //Active texture unit 0
	glBindTexture(GL_TEXTURE_2D, meshTexturePtr->get_id()); //The next binded texture will be refered with the active texture unit
	if (meshRenderLoc.u_textureMap != -1) {
		glUniform1i(meshRenderLoc.u_textureMap, 0); // we bound our texture to texture unit 0
	}


	//Send light and material
	passLightingState();

	/* Draw */
	meshPtr->drawTriangles(meshRenderLoc.a_position, meshRenderLoc.a_normal, meshRenderLoc.a_texture);
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
		background = !background;
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