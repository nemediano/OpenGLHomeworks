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
Texture* normalsTexturePtr = nullptr;
Texture* diffuseTexturePtr = nullptr;
Texture* specularTexturePtr = nullptr;
OGLProgram* lightProgPtr = nullptr;
OGLProgram* lightTextProgPtr = nullptr;
OGLProgram* bumpProgPtr = nullptr;
OGLProgram* bumpTextProgPtr = nullptr;
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

Locations lightLoc;
Locations lightTextLoc;
Locations bumpLoc;
Locations bumpTextLoc;

//Global variables for the program logic
float seconds_elapsed;
float angle;
float ratio;

bool gammaCorrection;
bool wireframe;
bool cullFace;
bool textureMap;

float gamma;
float mixValue;
int mode;

void passLightingState();
void reload_shaders();
void create_glut_window();
void init_program();
void init_OpenGL();
void create_glut_callbacks();
void exit_glut();

//Display functions
void simpleLighting();
void lightingTextures();
void bumpMapping();
void bumpMappingTextures();

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
	ImGui::RadioButton("Lighting (textures)", &mode, 1);
	ImGui::RadioButton("Bump mapping", &mode, 2);
	ImGui::RadioButton("Bump map (textures)", &mode, 3);

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

	ImGui::Checkbox("Gamma correction", &gammaCorrection);
	ImGui::Checkbox("Wirefreme", &wireframe);
	ImGui::Checkbox("Cull", &cullFace);
	ImGui::Checkbox("Texture map", &textureMap);
	if (gammaCorrection) {
		ImGui::SliderFloat("Gamma", &gamma, 1.0f, 2.5f);
	}
	if (textureMap) {
		ImGui::SliderFloat("Mix", &mixValue, 0.0f, 1.0f);
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
	delete normalsTexturePtr;
	delete meshPtr;
	delete lightProgPtr;
	delete bumpProgPtr;
	delete bumpTextProgPtr;
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
	window = glutCreateWindow("Bump mapping example");
}

void init_program() {
	using glm::vec3;

	/* Then, create primitives (load them from mesh) */
	meshPtr = new Mesh("../models/Rham-Phorynchus.obj");
	//meshPtr = new Mesh(Geometries::icosphere());
	normalsTexturePtr = new Texture("../models/rhambakes2_normals.png");
	diffuseTexturePtr = new Texture("../models/rham_diff.png");
	specularTexturePtr = new Texture("../models/rham_spec.png");
	if (meshPtr) {
		//Scale mesh to a unit cube, before sending it to GPU
		meshPtr->toUnitCube();
		meshPtr->sendToGPU();
	}

	if (normalsTexturePtr) {
		normalsTexturePtr->send_to_gpu();
	}
	
	if (specularTexturePtr) {
		specularTexturePtr->send_to_gpu();
	}

	if (diffuseTexturePtr) {
		diffuseTexturePtr->send_to_gpu();
	}

	gammaCorrection = false;
	wireframe = false;
	cullFace = true;
	textureMap = false;
	gamma = 1.0f;
	mixValue = 0.75f;
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

	//Default position of the spotlight
	light.eulerAngles = glm::vec3(-TAU / 4.0f, 0, 0.0f);
	light.distance = glm::sqrt(3.0f);
	light.p.setColor(vec3(1.0f));
	light.p.setIntensity(1.0f);
	light.g.setAperture(PI / 6.0f);
	ratio = 0.25;

	//Default material
	mat.setBaseColor(vec3(0.41f, 0.84f, 0.37f));
	mat.setF0(vec3(0.62f, 0.68f, 0.16f));
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
	lightProgPtr = new OGLProgram("shaders/disneyWorld.vert", "shaders/disneyWorld.frag");
	bumpProgPtr = new OGLProgram("shaders/bumpWorld.vert", "shaders/bumpWorld.frag");
	lightTextProgPtr = new OGLProgram("shaders/disneyWorld.vert", "shaders/disneyTextWorld.frag");
	bumpTextProgPtr = new OGLProgram("shaders/bumpWorld.vert", "shaders/bumpTextWorld.frag");

	if (!lightProgPtr || !lightProgPtr->isOK()) {
		cerr << "Something wrong in lighting shader" << endl;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	}

	if (!lightTextProgPtr || !lightTextProgPtr->isOK()) {
		cerr << "Something wrong in lighting with textures shader" << endl;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	}

	if (!bumpProgPtr || !bumpProgPtr->isOK()) {
		cerr << "Something wrong in simple bump mapping shader" << endl;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	}

	if (!bumpTextProgPtr || !bumpTextProgPtr->isOK()) {
		cerr << "Something wrong in bump mapping shader with textures" << endl;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	}


	if (lightProgPtr->isOK() && bumpProgPtr->isOK() && lightTextProgPtr->isOK() && bumpTextProgPtr->isOK()) {
		cout << "Shader compiled" << endl;
		glClearColor(0.7f, 0.7f, 1.0f, 1.0f);
	}

	/************************************************************************/
	/* Allocating variables for shaders                                     */
	/************************************************************************/
	lightLoc.a_position = lightProgPtr->attribLoc("Position");
	lightLoc.a_normal = lightProgPtr->attribLoc("Normal");
	
	lightLoc.u_PVM = lightProgPtr->uniformLoc("PVM");
	lightLoc.u_M = lightProgPtr->uniformLoc("M");
	lightLoc.u_NormalMat = lightProgPtr->uniformLoc("NormalMat");
	
	lightLoc.u_gamma = lightProgPtr->uniformLoc("gamma");
	lightLoc.u_cameraPos = lightProgPtr->uniformLoc("cameraPosition");

	lightLoc.u_metalicity = lightProgPtr->uniformLoc("mat.metalicity");
	lightLoc.u_roughness = lightProgPtr->uniformLoc("mat.roughness");
	lightLoc.u_base_color = lightProgPtr->uniformLoc("mat.baseColor");
	lightLoc.u_F0 = lightProgPtr->uniformLoc("mat.F0");

	lightLoc.u_lightPosition = lightProgPtr->uniformLoc("light.position");
	lightLoc.u_lightColor = lightProgPtr->uniformLoc("light.color");
	lightLoc.u_lightIntensity = lightProgPtr->uniformLoc("light.intensity");
	lightLoc.u_lightRatio = lightProgPtr->uniformLoc("light.ratio");


	bumpLoc.a_position = bumpProgPtr->attribLoc("Position");
	bumpLoc.a_texture = bumpProgPtr->attribLoc("TextCoord");

	bumpLoc.u_PVM = bumpProgPtr->uniformLoc("PVM");
	bumpLoc.u_M = bumpProgPtr->uniformLoc("M");
	bumpLoc.u_NormalMat = bumpProgPtr->uniformLoc("NormalMat");
	bumpLoc.u_normalsMap = bumpProgPtr->uniformLoc("normalsMap");

	bumpLoc.u_gamma = bumpProgPtr->uniformLoc("gamma");
	bumpLoc.u_cameraPos = bumpProgPtr->uniformLoc("cameraPosition");

	bumpLoc.u_metalicity = bumpProgPtr->uniformLoc("mat.metalicity");
	bumpLoc.u_roughness = bumpProgPtr->uniformLoc("mat.roughness");
	bumpLoc.u_base_color = bumpProgPtr->uniformLoc("mat.baseColor");
	bumpLoc.u_F0 = bumpProgPtr->uniformLoc("mat.F0");

	bumpLoc.u_lightPosition = bumpProgPtr->uniformLoc("light.position");
	bumpLoc.u_lightColor = bumpProgPtr->uniformLoc("light.color");
	bumpLoc.u_lightIntensity = bumpProgPtr->uniformLoc("light.intensity");
	bumpLoc.u_lightRatio = bumpProgPtr->uniformLoc("light.ratio");


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

	lightTextLoc.u_lightPosition = lightTextProgPtr->uniformLoc("light.position");
	lightTextLoc.u_lightColor = lightTextProgPtr->uniformLoc("light.color");
	lightTextLoc.u_lightIntensity = lightTextProgPtr->uniformLoc("light.intensity");
	lightTextLoc.u_lightRatio = lightTextProgPtr->uniformLoc("light.ratio");

	bumpTextLoc.a_position = bumpTextProgPtr->attribLoc("Position");
	bumpTextLoc.a_texture = bumpTextProgPtr->attribLoc("TextCoord");

	bumpTextLoc.u_PVM = bumpTextProgPtr->uniformLoc("PVM");
	bumpTextLoc.u_M = bumpTextProgPtr->uniformLoc("M");
	bumpTextLoc.u_NormalMat = bumpTextProgPtr->uniformLoc("NormalMat");
	bumpTextLoc.u_normalsMap = bumpTextProgPtr->uniformLoc("normalsMap");
	bumpTextLoc.u_diffuseMap = bumpTextProgPtr->uniformLoc("diffuseMap");
	bumpTextLoc.u_specularMap = bumpTextProgPtr->uniformLoc("specularMap");

	bumpTextLoc.u_gamma = bumpTextProgPtr->uniformLoc("gamma");
	bumpTextLoc.u_cameraPos = bumpTextProgPtr->uniformLoc("cameraPosition");

	bumpTextLoc.u_metalicity = bumpTextProgPtr->uniformLoc("mat.metalicity");
	bumpTextLoc.u_roughness = bumpTextProgPtr->uniformLoc("mat.roughness");
	

	bumpTextLoc.u_lightPosition = bumpTextProgPtr->uniformLoc("light.position");
	bumpTextLoc.u_lightColor = bumpTextProgPtr->uniformLoc("light.color");
	bumpTextLoc.u_lightIntensity = bumpTextProgPtr->uniformLoc("light.intensity");
	bumpTextLoc.u_lightRatio = bumpTextProgPtr->uniformLoc("light.ratio");
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
			glUniform3fv(lightLoc.u_lightPosition, 1, glm::value_ptr(light.g.getPosition()));
			glUniform3fv(lightLoc.u_lightColor, 1, glm::value_ptr(light.p.getColor()));
			glUniform1f(lightLoc.u_lightIntensity, light.p.getIntensity());
			glUniform1f(lightLoc.u_lightRatio, ratio);
			//Material
			glUniform1f(lightLoc.u_metalicity, mat.getMetalicity());
			glUniform1f(lightLoc.u_roughness, mat.getRoughness());
			glUniform3fv(lightLoc.u_base_color, 1, glm::value_ptr(mat.getBaseColor()));
			glUniform3fv(lightLoc.u_F0, 1, glm::value_ptr(mat.getF0()));
		break;

		case 1:
			//Light
			glUniform3fv(lightTextLoc.u_lightPosition, 1, glm::value_ptr(light.g.getPosition()));
			glUniform3fv(lightTextLoc.u_lightColor, 1, glm::value_ptr(light.p.getColor()));
			glUniform1f(lightTextLoc.u_lightIntensity, light.p.getIntensity());
			glUniform1f(lightTextLoc.u_lightRatio, ratio);
			//Material
			glUniform1f(lightTextLoc.u_metalicity, mat.getMetalicity());
			glUniform1f(lightTextLoc.u_roughness, mat.getRoughness());
			break;

		case 2:
			//Light
			glUniform3fv(bumpLoc.u_lightPosition, 1, glm::value_ptr(light.g.getPosition()));
			glUniform3fv(bumpLoc.u_lightColor, 1, glm::value_ptr(light.p.getColor()));
			glUniform1f(bumpLoc.u_lightIntensity, light.p.getIntensity());
			glUniform1f(bumpLoc.u_lightRatio, ratio);
			//Material
			glUniform1f(bumpLoc.u_metalicity, mat.getMetalicity());
			glUniform1f(bumpLoc.u_roughness, mat.getRoughness());
			glUniform3fv(bumpLoc.u_base_color, 1, glm::value_ptr(mat.getBaseColor()));
			glUniform3fv(bumpLoc.u_F0, 1, glm::value_ptr(mat.getF0()));
		break;

		case 3:
			//Light
			glUniform3fv(bumpTextLoc.u_lightPosition, 1, glm::value_ptr(light.g.getPosition()));
			glUniform3fv(bumpTextLoc.u_lightColor, 1, glm::value_ptr(light.p.getColor()));
			glUniform1f(bumpTextLoc.u_lightIntensity, light.p.getIntensity());
			glUniform1f(bumpTextLoc.u_lightRatio, ratio);
			//Material
			glUniform1f(bumpTextLoc.u_metalicity, mat.getMetalicity());
			glUniform1f(bumpTextLoc.u_roughness, mat.getRoughness());
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
			simpleLighting();
		break;

		case 1:
			lightingTextures();
		break;

		case 2:
			bumpMapping();
		break;

		case 3:
			bumpMappingTextures();
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

void simpleLighting() {
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

	lightProgPtr->use();

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

	glUniformMatrix4fv(lightLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M));
	glUniformMatrix4fv(lightLoc.u_M, 1, GL_FALSE, glm::value_ptr(M));
	glUniformMatrix4fv(lightLoc.u_NormalMat, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(M))));
	glUniform1f(lightLoc.u_gamma, gamma);

	glUniform3fv(lightLoc.u_lightPosition, 1, glm::value_ptr(light.g.getPosition()));
	vec4 camera_pos = vec4(cam.getPosition(), 1.0f);
	vec3 camPosInWorld = vec3(glm::inverse(V) * camera_pos);
	glUniform3fv(lightLoc.u_cameraPos, 1, glm::value_ptr(vec3(camPosInWorld)));
	//Send light and material
	passLightingState();

	/* Draw */
	meshPtr->drawTriangles(lightLoc.a_position, lightLoc.a_normal, lightLoc.a_texture);
}

void lightingTextures() {
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

	lightTextProgPtr->use();

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

	glActiveTexture(GL_TEXTURE1); //Active texture unit 1
	glBindTexture(GL_TEXTURE_2D, specularTexturePtr->get_id()); //The next binded texture will be refered with the active texture unit
	if (lightTextLoc.u_specularMap != -1) {
		glUniform1i(lightTextLoc.u_specularMap, 1); // we bound our texture to texture unit 1
	}
	
	/* Draw */
	meshPtr->drawTriangles(lightTextLoc.a_position, lightTextLoc.a_normal, lightTextLoc.a_texture);
}

void bumpMapping() {
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

	bumpProgPtr->use();

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

	glUniformMatrix4fv(bumpLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M));
	glUniformMatrix4fv(bumpLoc.u_M, 1, GL_FALSE, glm::value_ptr(M));
	glUniformMatrix4fv(bumpLoc.u_NormalMat, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(M))));
	glUniform1f(bumpLoc.u_gamma, gamma);

	glUniform3fv(bumpLoc.u_lightPosition, 1, glm::value_ptr(light.g.getPosition()));
	vec4 camera_pos = vec4(cam.getPosition(), 1.0f);
	vec3 camPosInWorld = vec3(glm::inverse(V) * camera_pos);
	glUniform3fv(bumpLoc.u_cameraPos, 1, glm::value_ptr(vec3(camPosInWorld)));
	//Send light and material
	passLightingState();
	glActiveTexture(GL_TEXTURE0); //Active texture unit 0
	glBindTexture(GL_TEXTURE_2D, normalsTexturePtr->get_id()); //The next binded texture will be refered with the active texture unit
	if (bumpLoc.u_normalsMap != -1) {
		glUniform1i(bumpLoc.u_normalsMap, 0); // we bound our texture to texture unit 0
	}
	/* Draw */
	meshPtr->drawTriangles(bumpLoc.a_position, bumpLoc.a_normal, bumpLoc.a_texture);
}

void bumpMappingTextures() {
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

	bumpTextProgPtr->use();

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

	glUniformMatrix4fv(bumpTextLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M));
	glUniformMatrix4fv(bumpTextLoc.u_M, 1, GL_FALSE, glm::value_ptr(M));
	glUniformMatrix4fv(bumpTextLoc.u_NormalMat, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(M))));
	glUniform1f(bumpTextLoc.u_gamma, gamma);

	glUniform3fv(bumpTextLoc.u_lightPosition, 1, glm::value_ptr(light.g.getPosition()));
	vec4 camera_pos = vec4(cam.getPosition(), 1.0f);
	vec3 camPosInWorld = vec3(glm::inverse(V) * camera_pos);
	glUniform3fv(bumpTextLoc.u_cameraPos, 1, glm::value_ptr(vec3(camPosInWorld)));
	//Send light and material
	passLightingState();
	glActiveTexture(GL_TEXTURE0); //Active texture unit 0
	glBindTexture(GL_TEXTURE_2D, normalsTexturePtr->get_id()); //The next binded texture will be refered with the active texture unit
	if (bumpTextLoc.u_normalsMap != -1) {
		glUniform1i(bumpTextLoc.u_normalsMap, 0); // we bound our texture to texture unit 0
	}
	glActiveTexture(GL_TEXTURE1); //Active texture unit 1
	glBindTexture(GL_TEXTURE_2D, diffuseTexturePtr->get_id()); //The next binded texture will be refered with the active texture unit
	if (bumpTextLoc.u_diffuseMap != -1) {
		glUniform1i(bumpTextLoc.u_diffuseMap, 1); // we bound our texture to texture unit 1
	}
	glActiveTexture(GL_TEXTURE2); //Active texture unit 2
	glBindTexture(GL_TEXTURE_2D, specularTexturePtr->get_id()); //The next binded texture will be refered with the active texture unit
	if (bumpTextLoc.u_specularMap != -1) {
		glUniform1i(bumpTextLoc.u_specularMap, 2); // we bound our texture to texture unit 2
	}
	/* Draw */
	meshPtr->drawTriangles(bumpTextLoc.a_position, bumpTextLoc.a_normal, bumpTextLoc.a_texture);
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

		case GLUT_KEY_F4:
			mode = 3;
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