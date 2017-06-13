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
Texture* diffuseTexturePtr = nullptr;
OGLProgram* lightTextProgPtr = nullptr;
OGLProgram* explodeProgPtr = nullptr;
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
	GLint u_normalsMap = -1;
	GLint u_diffuseMap = -1;
	GLint u_specularMap = -1;
	//Geometry shader uniforms
	GLint u_PV = -1;
	GLint u_frequency = -1;
	GLint u_amplitude = -1;
	GLint u_time = -1;
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

Locations lightTextLoc;
Locations explodeLoc;

//Global variables for the program logic
float seconds_elapsed;
float angle;
float ratio;

bool gammaCorrection;
bool wireframe;
bool cullFace;

float gamma;
int mode;
float frequency;
float amplitude;
float displacement;

void passLightingState();
void reload_shaders();
void create_glut_window();
void init_program();
void init_OpenGL();
void create_glut_callbacks();
void exit_glut();

//Display functions
void explodeGeometry();
void lightingTextures();

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
	ImGui::RadioButton("Simple lighting", &mode, 0);
	ImGui::RadioButton("Exploding oscilate", &mode, 1);
	ImGui::RadioButton("Exploding control", &mode, 2);

	if (mode == 1) {
		ImGui::SliderFloat("Frequency", &frequency, 0.0f, 5.0f, "%.3f", 2.0);
		ImGui::SliderFloat("Amplitude", &amplitude, 0.0f, 5.0f, "%.3f", 2.0);
	}

	if (mode == 2) {
		ImGui::SliderFloat("Displacement", &displacement, 0.0f, 1.0f, "%.3f");
		ImGui::SliderFloat("Amplitude", &amplitude, 0.0f, 5.0f, "%.3f", 2.0);
	}
		
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
		//vec3 baseColor = mat.getBaseColor();
		vec3 F0 = mat.getF0();
		float metalicity = mat.getMetalicity();
		float roughness = mat.getRoughness();
		//ImGui::ColorEdit3("Base color", glm::value_ptr(baseColor));
		//ImGui::Text("R: %.2f G: %.2f B: %.2f", baseColor.r, baseColor.g, baseColor.b);
		ImGui::ColorEdit3("Specular Color", glm::value_ptr(F0));
		ImGui::Text("R: %.2f G: %.2f B: %.2f", F0.r, F0.g, F0.b);
		ImGui::SliderFloat("Metalicity", &metalicity, 0.0f, 1.0f, "%.3f");
		ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f, "%.3f");
		//mat.setBaseColor(baseColor);
		mat.setF0(F0);
		mat.setRoughness(roughness);
		mat.setMetalicity(metalicity);
	}

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

	ImGui::End();

	/* End with this when you want to render GUI */
	ImGui::Render();
}

void exit_glut() {
	delete diffuseTexturePtr;
	delete explodeProgPtr;
	delete meshPtr;
	delete lightTextProgPtr;
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
	glutInitWindowSize(1200, 900);
	window = glutCreateWindow("Explode using geometry shader");
}

void init_program() {
	using glm::vec3;

	/* Then, create primitives (load them from mesh) */
	meshPtr = new Mesh(Geometries::sphere());
	diffuseTexturePtr = new Texture("../models/world32k.jpg");

	if (meshPtr) {
		//Scale mesh to a unit cube, before sending it to GPU
		meshPtr->toUnitCube();
		meshPtr->sendToGPU();
	}

	if (diffuseTexturePtr) {
		diffuseTexturePtr->send_to_gpu();
	}

	gammaCorrection = false;
	wireframe = false;
	cullFace = false;
	gamma = 1.0f;
	seconds_elapsed = 0.0f;
	mode = 0;
	frequency = 3.0f;
	amplitude = 0.2f;

	//Set the default position of the camera
	cam.setLookAt(vec3(0.0f, 0.0f, 2.0f), vec3(0.0f));
	cam.setAspectRatio(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	cam.setFovY(PI / 4.5f);
	cam.setDepthView(0.1f, 5.0f);
	//Create trackball camera
	ball.setWindowSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	//Also setup the image grabber
	grabber.resize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

	//Default position of the spotlight
	light.eulerAngles = glm::vec3(0.0f, 0.0f, 0.0f);
	light.distance = 3.0f;
	light.p.setColor(vec3(1.0f));
	light.p.setIntensity(1.0f);
	light.g.setAperture(PI / 6.0f);
	ratio = 0.25;

	//Default material
	mat.setBaseColor(vec3(0.41f, 0.84f, 0.37f));
	mat.setF0(vec3(0.5f));
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
	lightTextProgPtr = new OGLProgram("shaders/disneyWorld.vert", "shaders/disneyTextWorld.frag");
	explodeProgPtr = new OGLProgram("shaders/explode.vert", "shaders/explode.frag", "shaders/explode.geom");

	if (!lightTextProgPtr || !lightTextProgPtr->isOK()) {
		cerr << "Something wrong in lighting with textures shader" << endl;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	}

	if (!explodeProgPtr || !explodeProgPtr->isOK()) {
		cerr << "Something wrong in exploding with geometry shader" << endl;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	}

	if (lightTextProgPtr->isOK() && explodeProgPtr->isOK()) {
		cout << "Shaders compiled!" << endl;
		glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	}

	/************************************************************************/
	/* Allocating variables for shaders                                     */
	/************************************************************************/
	
	lightTextLoc.a_position = lightTextProgPtr->attribLoc("Position");
	lightTextLoc.a_normal = lightTextProgPtr->attribLoc("Normal");
	lightTextLoc.a_texture = lightTextProgPtr->attribLoc("TextCoord");

	lightTextLoc.u_PVM = lightTextProgPtr->uniformLoc("PVM");
	lightTextLoc.u_M = lightTextProgPtr->uniformLoc("M");
	lightTextLoc.u_NormalMat = lightTextProgPtr->uniformLoc("NormalMat");
	lightTextLoc.u_diffuseMap = lightTextProgPtr->uniformLoc("diffuseMap");
	lightTextLoc.u_specularMap = lightTextProgPtr->uniformLoc("specularMap");

	lightTextLoc.u_gamma = lightTextProgPtr->uniformLoc("gamma");
	lightTextLoc.u_cameraPos = lightTextProgPtr->uniformLoc("cameraPosition");

	lightTextLoc.u_metalicity = lightTextProgPtr->uniformLoc("mat.metalicity");
	lightTextLoc.u_roughness = lightTextProgPtr->uniformLoc("mat.roughness");
	lightTextLoc.u_F0 = lightTextProgPtr->uniformLoc("mat.F0");

	lightTextLoc.u_lightPosition = lightTextProgPtr->uniformLoc("light.position");
	lightTextLoc.u_lightColor = lightTextProgPtr->uniformLoc("light.color");
	lightTextLoc.u_lightIntensity = lightTextProgPtr->uniformLoc("light.intensity");
	lightTextLoc.u_lightRatio = lightTextProgPtr->uniformLoc("light.ratio");

	/************************************************************************/
	explodeLoc.a_position = explodeProgPtr->attribLoc("Position");
	explodeLoc.a_normal = explodeProgPtr->attribLoc("Normal");
	explodeLoc.a_texture = explodeProgPtr->attribLoc("TextCoord");

	explodeLoc.u_PVM = explodeProgPtr->uniformLoc("PVM");
	explodeLoc.u_M = explodeProgPtr->uniformLoc("M");
	explodeLoc.u_PV = explodeProgPtr->uniformLoc("PV");
	explodeLoc.u_NormalMat = explodeProgPtr->uniformLoc("NormalMat");
	explodeLoc.u_diffuseMap = explodeProgPtr->uniformLoc("diffuseMap");
	explodeLoc.u_specularMap = explodeProgPtr->uniformLoc("specularMap");

	explodeLoc.u_frequency = explodeProgPtr->uniformLoc("frequency");
	explodeLoc.u_amplitude = explodeProgPtr->uniformLoc("amplitude");
	explodeLoc.u_time = explodeProgPtr->uniformLoc("time");

	explodeLoc.u_gamma = explodeProgPtr->uniformLoc("gamma");
	explodeLoc.u_cameraPos = explodeProgPtr->uniformLoc("cameraPosition");

	explodeLoc.u_metalicity = explodeProgPtr->uniformLoc("mat.metalicity");
	explodeLoc.u_roughness = explodeProgPtr->uniformLoc("mat.roughness");
	explodeLoc.u_F0 = explodeProgPtr->uniformLoc("mat.F0");

	explodeLoc.u_lightPosition = explodeProgPtr->uniformLoc("light.position");
	explodeLoc.u_lightColor = explodeProgPtr->uniformLoc("light.color");
	explodeLoc.u_lightIntensity = explodeProgPtr->uniformLoc("light.intensity");
	explodeLoc.u_lightRatio = explodeProgPtr->uniformLoc("light.ratio");
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
	switch (mode) {
		case 0:
			//Light
			glUniform3fv(lightTextLoc.u_lightPosition, 1, glm::value_ptr(light.g.getPosition()));
			glUniform3fv(lightTextLoc.u_lightColor, 1, glm::value_ptr(light.p.getColor()));
			glUniform1f(lightTextLoc.u_lightIntensity, light.p.getIntensity());
			glUniform1f(lightTextLoc.u_lightRatio, ratio);
			//Material
			glUniform1f(lightTextLoc.u_metalicity, mat.getMetalicity());
			glUniform1f(lightTextLoc.u_roughness, mat.getRoughness());
			glUniform3fv(lightTextLoc.u_F0, 1, glm::value_ptr(mat.getF0()));
		break;

		case 1:
		case 2:
			//Light
			glUniform3fv(explodeLoc.u_lightPosition, 1, glm::value_ptr(light.g.getPosition()));
			glUniform3fv(explodeLoc.u_lightColor, 1, glm::value_ptr(light.p.getColor()));
			glUniform1f(explodeLoc.u_lightIntensity, light.p.getIntensity());
			glUniform1f(explodeLoc.u_lightRatio, ratio);
			//Material
			glUniform1f(explodeLoc.u_metalicity, mat.getMetalicity());
			glUniform1f(explodeLoc.u_roughness, mat.getRoughness());
			glUniform3fv(explodeLoc.u_F0, 1, glm::value_ptr(mat.getF0()));
		break;
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
	grabber.resize(new_window_width, new_window_height);
}

void display() {

	if (wireframe) {
		glPolygonMode(GL_FRONT, GL_LINE);
		glPolygonMode(GL_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_FILL);
	}
	if (cullFace) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
	else {
		glDisable(GL_CULL_FACE);
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch (mode) {
		case 0:
			lightingTextures();
		break;

		case 1:
		case 2:
			explodeGeometry();
		break;
	}

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

void lightingTextures() {
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

	lightTextProgPtr->use();

	mat4 I(1.0f);
	//Model
	mat4 M = glm::rotate(PI, vec3(0.0f, 1.0f, 0.0f));
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

	glUniformMatrix4fv(lightTextLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M));
	glUniformMatrix4fv(lightTextLoc.u_M, 1, GL_FALSE, glm::value_ptr(M));
	glUniformMatrix4fv(lightTextLoc.u_NormalMat, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(M))));
	glUniform1f(lightTextLoc.u_gamma, gamma);

	glUniform3fv(lightTextLoc.u_lightPosition, 1, glm::value_ptr(light.g.getPosition()));
	vec4 camera_pos = vec4(cam.getPosition(), 1.0f);
	vec3 camPosInWorld = vec3(glm::inverse(V) * camera_pos);
	glUniform3fv(lightTextLoc.u_cameraPos, 1, glm::value_ptr(vec3(camPosInWorld)));
	//Send light and material
	passLightingState();

	glActiveTexture(GL_TEXTURE0); //Active texture unit 0
	glBindTexture(GL_TEXTURE_2D, diffuseTexturePtr->get_id()); //The next binded texture will be refered with the active texture unit
	if (lightTextLoc.u_diffuseMap != -1) {
		glUniform1i(lightTextLoc.u_diffuseMap, 0); // we bound our texture to texture unit 0
	}

	/* Draw */
	meshPtr->drawTriangles(lightTextLoc.a_position, lightTextLoc.a_normal, lightTextLoc.a_texture);
}

void explodeGeometry() {
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

	explodeProgPtr->use();

	mat4 I(1.0f);
	//Model
	mat4 M = glm::rotate(PI, vec3(0.0f, 1.0f, 0.0f));
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

	glUniformMatrix4fv(explodeLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M));
	glUniformMatrix4fv(explodeLoc.u_PV, 1, GL_FALSE, glm::value_ptr(P * V));
	glUniformMatrix4fv(explodeLoc.u_M, 1, GL_FALSE, glm::value_ptr(M));
	glUniformMatrix4fv(explodeLoc.u_NormalMat, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(M))));
	glUniform1f(explodeLoc.u_gamma, gamma);

	glUniform3fv(explodeLoc.u_lightPosition, 1, glm::value_ptr(light.g.getPosition()));
	vec4 camera_pos = vec4(cam.getPosition(), 1.0f);
	vec3 camPosInWorld = vec3(glm::inverse(V) * camera_pos);
	glUniform3fv(explodeLoc.u_cameraPos, 1, glm::value_ptr(vec3(camPosInWorld)));
	glUniform1f(explodeLoc.u_amplitude, amplitude);
	if (mode == 1) {
		glUniform1f(explodeLoc.u_frequency, frequency);
		glUniform1f(explodeLoc.u_time, seconds_elapsed);
	} else {
		glUniform1f(explodeLoc.u_frequency, 1.0f);
		glUniform1f(explodeLoc.u_time, PI * (displacement + 1.5f));
	}
	
	//Send light and material
	passLightingState();

	glActiveTexture(GL_TEXTURE0); //Active texture unit 0
	glBindTexture(GL_TEXTURE_2D, diffuseTexturePtr->get_id()); //The next binded texture will be refered with the active texture unit
	if (explodeLoc.u_diffuseMap != -1) {
		glUniform1i(explodeLoc.u_diffuseMap, 0); // we bound our texture to texture unit 0
	}

	/* Draw */
	meshPtr->drawTriangles(explodeLoc.a_position, explodeLoc.a_normal, explodeLoc.a_texture);
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

	switch (key) {
		case GLUT_KEY_F1:
			mode = 0;
		break;

		case GLUT_KEY_F2:
			mode = 1;
		break;

		case GLUT_KEY_F3:
			mode = 2;
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