#include "stdafx.h"

/* Include heders and namespaces from my CGT Library */
#include "OGLHelpers.h"
#include "MathConstants.h"
#include "Mesh.h"
#include "Camera.h"
#include "Trackball.h"
#include "OGLProgram.h"
#include "Material.h"
#include "DisneyLight.h"
#include "Geometries.h"
#include "ScreenGrabber.h"
#include "Spotlight.h"
#include "Cubemap.h"

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
Mesh* cubePtr = nullptr;
Texture* meshTexturePtr = nullptr;
CubeMap* skyBoxTexturePtr = nullptr;
OGLProgram* skyBoxProgPtr = nullptr;
OGLProgram* reflectionProgPtr = nullptr;
OGLProgram* refractionProgPtr = nullptr;
OGLProgram* envMapProgPtr = nullptr;

ScreenGrabber grabber;
Material materialMesh;


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
	GLint u_alpha = -1;
	GLint u_cubeMap = -1;
	GLint u_mixFactor = -1;
	GLint u_refractRatio = -1;
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

Locations reflectionLoc;
Locations refractionLoc;
Locations envMapLoc;
Locations skyBoxRenderLoc;


//Global variables for the program logic
float seconds_elapsed;
float angle;
float ratio;

bool gammaCorrection;
int mode;

float gamma;
float mixFactor;
float refractRatio;

void reload_shaders();
void create_glut_window();
void init_program();
void init_OpenGL();
void create_glut_callbacks();
void exit_glut();

//Render functions
void reflection();
void refraction();
void enviromentalMapping();
void renderSkyBox();

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

	ImGui::RadioButton("Reflection", &mode, 0);
	ImGui::RadioButton("Refraction", &mode, 1);
	ImGui::RadioButton("Enviromental mapping", &mode, 2);

	ImGui::SliderFloat("Mix Factor", &mixFactor, 0.0f, 1.0f);
	if (mode == 1) {
		ImGui::SliderFloat("Refraction Ratio", &refractRatio, 0.0f, 1.0f);
	}
	
	if (ImGui::CollapsingHeader("Light position")) {
		ImGui::SliderFloat("Distance", &light.distance, 0.0f, 5.0f);
		ImGui::SliderAngle("Angle X", &light.eulerAngles.x, -180.0f, 180.0f);
		ImGui::SliderAngle("Angle Y", &light.eulerAngles.y, -180.0f, 180.0f);
	}

	if (ImGui::CollapsingHeader("Light physical properties")) {
		vec3 color = light.p.getColor();
		float intensity = light.p.getIntensity();
		ImGui::ColorEdit3("Color", glm::value_ptr(color));
		ImGui::SliderFloat("Intensity", &intensity, 0.0f, 1.0f);
		light.p.setColor(color);
		light.p.setIntensity(intensity);
		ImGui::SliderFloat("Ratio", &ratio, 0.0f, 1.0f);
	}

	if (ImGui::CollapsingHeader("Material editor")) {
		//Edit Material
		vec3 F0 = materialMesh.getF0();
		float metalicity = materialMesh.getMetalicity();
		float roughness = materialMesh.getRoughness();

		ImGui::ColorEdit3("Specular Color M", glm::value_ptr(F0));
		ImGui::SliderFloat("Metalicity M", &metalicity, 0.0f, 1.0f, "%.3f");
		ImGui::SliderFloat("Roughness M", &roughness, 0.0f, 1.0f, "%.3f");

		materialMesh.setF0(F0);
		materialMesh.setRoughness(roughness);
		materialMesh.setMetalicity(metalicity);
	}

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
	delete cubePtr;
	delete reflectionProgPtr;
	delete refractionProgPtr;
	delete envMapProgPtr;
	delete meshTexturePtr;
	delete skyBoxProgPtr;
	delete skyBoxTexturePtr;
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
	window = glutCreateWindow("Enviromental mapping example");
}

void init_program() {
	using glm::vec3;

	/* Then, create primitives (load them from mesh) */
	meshPtr = new Mesh("../models/Amago.obj");
	//meshPtr = new Mesh(Geometries::icosphere());
	cubePtr = new Mesh(Geometries::insideOutCube());
	meshTexturePtr = new Texture("../models/AmagoTexture.png");
	std::vector<std::string> faces;
	faces.push_back("../img/skybox/right.jpg");
	faces.push_back("../img/skybox/left.jpg");
	faces.push_back("../img/skybox/bottom.jpg");
	faces.push_back("../img/skybox/top.jpg");
	faces.push_back("../img/skybox/back.jpg");
	faces.push_back("../img/skybox/front.jpg");
	skyBoxTexturePtr = new CubeMap(faces);

	if (meshPtr) {
		//Scale mesh to a unit cube, before sending it to GPU
		meshPtr->toUnitCube();
		meshPtr->sendToGPU();
	}

	if (cubePtr) {
		//Cube start as unit cube (volume 1), center in the oringin.
		//The cube box needs to have coordinates on [-1, 1], we need to scale it 
		//before using
		cubePtr->transform(glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)));
		cubePtr->sendToGPU();
	}

	if (meshTexturePtr) {
		meshTexturePtr->send_to_gpu();
	}

	gammaCorrection = false;
	mode = 0;
	gamma = 1.0f;
	seconds_elapsed = 0.0f;
	mixFactor = 0.5f;
	refractRatio = 1.0f;

	//Set the default position of the camera
	cam.setLookAt(vec3(0.0f, 0.0f, 3.0f), vec3(0.0f));
	cam.setAspectRatio(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	cam.setFovY(PI / 4.5f);
	cam.setDepthView(0.1f, 9.0f);
	//Create trackball camera
	ball.setWindowSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	//Also setup the image grabber
	grabber.resize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

	//Default position of the point light
	light.eulerAngles = glm::vec3(0.0f, 0.0f, 0.0f);
	light.distance = 4.0f;
	light.p.setColor(vec3(1.0f));
	light.p.setIntensity(1.0f);
	ratio = 0.25;

	//Default material
	materialMesh.setBaseColor(vec3(0.0f, 0.0f, 0.0f));//We will use texture for this
	materialMesh.setF0(vec3(0.66f, 0.67f, 0.52f));
	materialMesh.setMetalicity(0.2f);
	materialMesh.setRoughness(0.15f);
}

void reload_shaders() {
	using std::cout;
	using std::cerr;
	using std::endl;
	/************************************************************************/
	/*                   OpenGL program creation                            */
	/************************************************************************/
	reflectionProgPtr = new OGLProgram("shaders/disneyWorld.vert", "shaders/reflection.frag");
	refractionProgPtr = new OGLProgram("shaders/disneyWorld.vert", "shaders/refraction.frag");
	//envMapProgPtr = new OGLProgram("shaders/disneyWorld.vert", "shaders/envMap.frag");
	envMapProgPtr = new OGLProgram("shaders/disneyWorld.vert", "shaders/disneyWorldTexture.frag");
	skyBoxProgPtr = new OGLProgram("shaders/skyBox.vert", "shaders/skyBox.frag");

	if (!reflectionProgPtr || !reflectionProgPtr->isOK()) {
		cerr << "Something wrong in reflection shader" << endl;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	}

	if (!refractionProgPtr || !refractionProgPtr->isOK()) {
		cerr << "Something wrong in refraction shader" << endl;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	}

	if (!envMapProgPtr || !envMapProgPtr->isOK()) {
		cerr << "Something wrong in enviroment map shader" << endl;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	}

	if (!skyBoxProgPtr || !skyBoxProgPtr->isOK()) {
		cerr << "Something wrong in render skybox shader" << endl;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	}

	if (reflectionProgPtr->isOK() && refractionProgPtr->isOK() &&
		envMapProgPtr->isOK() && skyBoxProgPtr->isOK()) {
		cout << "Shaders compiled succesufully" << endl;
		glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	}

	/************************************************************************/
	/* Allocating variables for shaders                                     */
	/************************************************************************/
	
	/* Reflection */
	reflectionLoc.a_position = reflectionProgPtr->attribLoc("Position");
	reflectionLoc.a_normal = reflectionProgPtr->attribLoc("Normal");
	reflectionLoc.a_texture = reflectionProgPtr->attribLoc("TextCoord");

	reflectionLoc.u_PVM = reflectionProgPtr->uniformLoc("PVM");
	reflectionLoc.u_M = reflectionProgPtr->uniformLoc("M");
	reflectionLoc.u_NormalMat = reflectionProgPtr->uniformLoc("NormalMat");

	reflectionLoc.u_gamma = reflectionProgPtr->uniformLoc("gamma");
	reflectionLoc.u_cameraPos = reflectionProgPtr->uniformLoc("cameraPosition");
	reflectionLoc.u_textureMap = reflectionProgPtr->uniformLoc("diffuseMap");
	reflectionLoc.u_mixFactor = reflectionProgPtr->uniformLoc("mixFactor");
	reflectionLoc.u_cubeMap = reflectionProgPtr->uniformLoc("skybox");

	reflectionLoc.u_metalicity = reflectionProgPtr->uniformLoc("mat.metalicity");
	reflectionLoc.u_roughness = reflectionProgPtr->uniformLoc("mat.roughness");
	reflectionLoc.u_base_color = reflectionProgPtr->uniformLoc("mat.baseColor");
	reflectionLoc.u_F0 = reflectionProgPtr->uniformLoc("mat.F0");

	reflectionLoc.u_lightPosition = reflectionProgPtr->uniformLoc("light.position");
	reflectionLoc.u_lightColor = reflectionProgPtr->uniformLoc("light.color");
	reflectionLoc.u_lightIntensity = reflectionProgPtr->uniformLoc("light.intensity");
	reflectionLoc.u_lightRatio = reflectionProgPtr->uniformLoc("light.ratio");

	/* Refraction */
	refractionLoc.a_position = refractionProgPtr->attribLoc("Position");
	refractionLoc.a_normal = refractionProgPtr->attribLoc("Normal");
	refractionLoc.a_texture = refractionProgPtr->attribLoc("TextCoord");

	refractionLoc.u_PVM = refractionProgPtr->uniformLoc("PVM");
	refractionLoc.u_M = refractionProgPtr->uniformLoc("M");
	refractionLoc.u_NormalMat = refractionProgPtr->uniformLoc("NormalMat");

	refractionLoc.u_gamma = refractionProgPtr->uniformLoc("gamma");
	refractionLoc.u_cameraPos = refractionProgPtr->uniformLoc("cameraPosition");
	refractionLoc.u_textureMap = refractionProgPtr->uniformLoc("diffuseMap");
	refractionLoc.u_mixFactor = refractionProgPtr->uniformLoc("mixFactor");
	refractionLoc.u_refractRatio = refractionProgPtr->uniformLoc("refractRatio");
	refractionLoc.u_cubeMap = refractionProgPtr->uniformLoc("skybox");

	refractionLoc.u_metalicity = refractionProgPtr->uniformLoc("mat.metalicity");
	refractionLoc.u_roughness = refractionProgPtr->uniformLoc("mat.roughness");
	refractionLoc.u_base_color = refractionProgPtr->uniformLoc("mat.baseColor");
	refractionLoc.u_F0 = refractionProgPtr->uniformLoc("mat.F0");

	refractionLoc.u_lightPosition = refractionProgPtr->uniformLoc("light.position");
	refractionLoc.u_lightColor = refractionProgPtr->uniformLoc("light.color");
	refractionLoc.u_lightIntensity = refractionProgPtr->uniformLoc("light.intensity");
	refractionLoc.u_lightRatio = refractionProgPtr->uniformLoc("light.ratio");

	/* Enviromental mapping */
	envMapLoc.a_position = envMapProgPtr->attribLoc("Position");
	envMapLoc.a_normal = envMapProgPtr->attribLoc("Normal");
	envMapLoc.a_texture = envMapProgPtr->attribLoc("TextCoord");

	envMapLoc.u_PVM = envMapProgPtr->uniformLoc("PVM");
	envMapLoc.u_M = envMapProgPtr->uniformLoc("M");
	envMapLoc.u_NormalMat = envMapProgPtr->uniformLoc("NormalMat");

	envMapLoc.u_gamma = envMapProgPtr->uniformLoc("gamma");
	envMapLoc.u_cameraPos = envMapProgPtr->uniformLoc("cameraPosition");
	envMapLoc.u_textureMap = envMapProgPtr->uniformLoc("diffuseMap");

	envMapLoc.u_metalicity = envMapProgPtr->uniformLoc("mat.metalicity");
	envMapLoc.u_roughness = envMapProgPtr->uniformLoc("mat.roughness");
	envMapLoc.u_base_color = envMapProgPtr->uniformLoc("mat.baseColor");
	envMapLoc.u_F0 = envMapProgPtr->uniformLoc("mat.F0");

	envMapLoc.u_lightPosition = envMapProgPtr->uniformLoc("light.position");
	envMapLoc.u_lightColor = envMapProgPtr->uniformLoc("light.color");
	envMapLoc.u_lightIntensity = envMapProgPtr->uniformLoc("light.intensity");
	envMapLoc.u_lightRatio = envMapProgPtr->uniformLoc("light.ratio");

	/* For rendering the sky box */
	skyBoxRenderLoc.a_position = skyBoxProgPtr->attribLoc("Position");
	skyBoxRenderLoc.u_PVM = skyBoxProgPtr->uniformLoc("PVM");
	skyBoxRenderLoc.u_M = skyBoxProgPtr->uniformLoc("M");
	skyBoxRenderLoc.u_cubeMap = skyBoxProgPtr->uniformLoc("skybox");
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
	/* Clear both buffers */
	glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Render mesh according to mode*/
	switch (mode) {
		case 0:
			reflection();
		break;

		case 1:
			refraction();
		break;

		case 2:
			enviromentalMapping();
		break;

		default:
		break;
	}

	renderSkyBox();

	//Unbind an clean
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	/* You need to call this to draw the GUI, After unbinding your program*/
	drawGUI();
	/* But, before flushing the drawinng commands*/

	glutSwapBuffers();
}


void reflection() {
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

	reflectionProgPtr->use();

	mat4 I(1.0f);
	//Model
	mat4 M = I;
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

	glUniformMatrix4fv(reflectionLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M));
	glUniformMatrix4fv(reflectionLoc.u_M, 1, GL_FALSE, glm::value_ptr(M));
	glUniformMatrix4fv(reflectionLoc.u_NormalMat, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(M))));
	glUniform1f(reflectionLoc.u_gamma, gammaCorrection ? gamma : 1.0f);
	glUniform1f(reflectionLoc.u_mixFactor, mixFactor);

	glUniform3fv(reflectionLoc.u_lightPosition, 1, glm::value_ptr(light.g.getPosition()));
	vec4 camera_pos = vec4(cam.getPosition(), 1.0f);
	vec3 camPosInWorld = vec3(glm::inverse(V) * camera_pos);
	glUniform3fv(reflectionLoc.u_cameraPos, 1, glm::value_ptr(vec3(camPosInWorld)));
	glActiveTexture(GL_TEXTURE0); //Active texture unit 0
	glBindTexture(GL_TEXTURE_2D, meshTexturePtr->get_id()); //The next binded texture will be refered with the active texture unit
	if (reflectionLoc.u_textureMap != -1) {
		glUniform1i(reflectionLoc.u_textureMap, 0); // we bound our texture to texture unit 0
	}

	glActiveTexture(GL_TEXTURE1); //Active texture unit 1
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTexturePtr->get_id()); //The next binded texture will be refered with the active texture unit
	if (reflectionLoc.u_cubeMap != -1) {
		glUniform1i(reflectionLoc.u_cubeMap, 1); // we bound our texture to texture unit 1
	}

	//Material
	glUniform1f(reflectionLoc.u_metalicity, materialMesh.getMetalicity());
	glUniform1f(reflectionLoc.u_roughness, materialMesh.getRoughness());
	glUniform3fv(reflectionLoc.u_base_color, 1, glm::value_ptr(materialMesh.getBaseColor()));
	glUniform3fv(reflectionLoc.u_F0, 1, glm::value_ptr(materialMesh.getF0()));

	//Light
	glUniform3fv(reflectionLoc.u_lightPosition, 1, glm::value_ptr(light.g.getPosition()));
	glUniform3fv(reflectionLoc.u_lightColor, 1, glm::value_ptr(light.p.getColor()));
	glUniform1f(reflectionLoc.u_lightIntensity, light.p.getIntensity());
	glUniform1f(reflectionLoc.u_lightRatio, ratio);

	/* Draw */
	meshPtr->drawTriangles(reflectionLoc.a_position, reflectionLoc.a_normal, reflectionLoc.a_texture);
}

void refraction() {
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

	refractionProgPtr->use();

	mat4 I(1.0f);
	//Model
	mat4 M = I;
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

	glUniformMatrix4fv(refractionLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M));
	glUniformMatrix4fv(refractionLoc.u_M, 1, GL_FALSE, glm::value_ptr(M));
	glUniformMatrix4fv(refractionLoc.u_NormalMat, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(M))));
	glUniform1f(refractionLoc.u_gamma, gammaCorrection ? gamma : 1.0f);
	glUniform1f(refractionLoc.u_mixFactor, mixFactor);
	glUniform1f(refractionLoc.u_refractRatio, refractRatio);

	glUniform3fv(refractionLoc.u_lightPosition, 1, glm::value_ptr(light.g.getPosition()));
	vec4 camera_pos = vec4(cam.getPosition(), 1.0f);
	vec3 camPosInWorld = vec3(glm::inverse(V) * camera_pos);
	glUniform3fv(refractionLoc.u_cameraPos, 1, glm::value_ptr(vec3(camPosInWorld)));
	glActiveTexture(GL_TEXTURE0); //Active texture unit 0
	glBindTexture(GL_TEXTURE_2D, meshTexturePtr->get_id()); //The next binded texture will be refered with the active texture unit
	if (refractionLoc.u_textureMap != -1) {
		glUniform1i(refractionLoc.u_textureMap, 0); // we bound our texture to texture unit 0
	}

	glActiveTexture(GL_TEXTURE1); //Active texture unit 1
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTexturePtr->get_id()); //The next binded texture will be refered with the active texture unit
	if (refractionLoc.u_cubeMap != -1) {
		glUniform1i(refractionLoc.u_cubeMap, 1); // we bound our texture to texture unit 1
	}

	//Material
	glUniform1f(refractionLoc.u_metalicity, materialMesh.getMetalicity());
	glUniform1f(refractionLoc.u_roughness, materialMesh.getRoughness());
	glUniform3fv(refractionLoc.u_base_color, 1, glm::value_ptr(materialMesh.getBaseColor()));
	glUniform3fv(refractionLoc.u_F0, 1, glm::value_ptr(materialMesh.getF0()));

	//Light
	glUniform3fv(refractionLoc.u_lightPosition, 1, glm::value_ptr(light.g.getPosition()));
	glUniform3fv(refractionLoc.u_lightColor, 1, glm::value_ptr(light.p.getColor()));
	glUniform1f(refractionLoc.u_lightIntensity, light.p.getIntensity());
	glUniform1f(refractionLoc.u_lightRatio, ratio);

	/* Draw */
	meshPtr->drawTriangles(refractionLoc.a_position, refractionLoc.a_normal, refractionLoc.a_texture);
}

void enviromentalMapping() {
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

	envMapProgPtr->use();

	mat4 I(1.0f);
	//Model
	mat4 M = I;
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

	glUniformMatrix4fv(envMapLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M));
	glUniformMatrix4fv(envMapLoc.u_M, 1, GL_FALSE, glm::value_ptr(M));
	glUniformMatrix4fv(envMapLoc.u_NormalMat, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(M))));
	glUniform1f(envMapLoc.u_gamma, gammaCorrection ? gamma : 1.0f);

	glUniform3fv(envMapLoc.u_lightPosition, 1, glm::value_ptr(light.g.getPosition()));
	vec4 camera_pos = vec4(cam.getPosition(), 1.0f);
	vec3 camPosInWorld = vec3(glm::inverse(V) * camera_pos);
	glUniform3fv(envMapLoc.u_cameraPos, 1, glm::value_ptr(vec3(camPosInWorld)));
	glActiveTexture(GL_TEXTURE0); //Active texture unit 0
	glBindTexture(GL_TEXTURE_2D, meshTexturePtr->get_id()); //The next binded texture will be refered with the active texture unit
	if (envMapLoc.u_textureMap != -1) {
		glUniform1i(envMapLoc.u_textureMap, 0); // we bound our texture to texture unit 0
	}

	//Material
	glUniform1f(envMapLoc.u_metalicity, materialMesh.getMetalicity());
	glUniform1f(envMapLoc.u_roughness, materialMesh.getRoughness());
	glUniform3fv(envMapLoc.u_base_color, 1, glm::value_ptr(materialMesh.getBaseColor()));
	glUniform3fv(envMapLoc.u_F0, 1, glm::value_ptr(materialMesh.getF0()));

	//Light
	glUniform3fv(envMapLoc.u_lightPosition, 1, glm::value_ptr(light.g.getPosition()));
	glUniform3fv(envMapLoc.u_lightColor, 1, glm::value_ptr(light.p.getColor()));
	glUniform1f(envMapLoc.u_lightIntensity, light.p.getIntensity());
	glUniform1f(envMapLoc.u_lightRatio, ratio);

	/* Draw */
	meshPtr->drawTriangles(envMapLoc.a_position, envMapLoc.a_normal, envMapLoc.a_texture);
}

void renderSkyBox() {
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

	skyBoxProgPtr->use();

	mat4 I(1.0f);
	//Model
	mat4 M = glm::scale(I, glm::vec3(3.5f));

	//View
	mat4 V = cam.getViewMatrix() * ball.getRotation();
	//Projection
	mat4 P = cam.getProjectionMatrix();

	/************************************************************************/
	/* Send uniform values to shader                                        */
	/************************************************************************/

	glUniformMatrix4fv(skyBoxRenderLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M));
	glUniformMatrix4fv(skyBoxRenderLoc.u_M, 1, GL_FALSE, glm::value_ptr(M));

	glActiveTexture(GL_TEXTURE0); //Active texture unit 0
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTexturePtr->get_id()); //The next binded texture will be refered with the active texture unit
	if (skyBoxRenderLoc.u_cubeMap != -1) {
		glUniform1i(skyBoxRenderLoc.u_cubeMap, 0); // we bound our texture to texture unit 0
	}

	/* Draw */
	cubePtr->drawTriangles(skyBoxRenderLoc.a_position, skyBoxRenderLoc.a_normal, skyBoxRenderLoc.a_texture);

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
		cam.setFovY(PI / 4.5f);
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