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
#include "Camera.h"
#include "Trackball.h"
#include "OGLProgram.h"
#include "Geometries.h"
using namespace ogl;
using namespace math;
using namespace camera;
using namespace ogl;

/* CGT Library related*/
Camera cam;
Trackball ball;
OGLProgram* programFacesPtr = nullptr;
OGLProgram* programRayTracePhongPtr = nullptr;
OGLProgram* programRayTraceCkTrrncPtr = nullptr;

#define BUFFER_OFFSET(i) ((char*) nullptr + (i))

struct FBO {
	GLuint id = 0;
	GLuint depthTex = 0;
	GLuint backFacesTex = 0;
	GLuint renderTex = 0;
	int width = 0;
	int height = 0;
};

struct Mesh {
	GLuint vertex_id = 0;
	GLuint index_id = 0;
	int num_vertex = 0;
	int num_indices = 0;
};

struct Locations {
	//Attributes
	GLint a_pos = -1;
	GLint a_tex = -1;
	//Uniforms
	GLint u_Q = -1;
	GLint u_PVM = -1;
	GLint u_scene = -1;
	GLint u_Tex = -1;
	GLint u_Ka = -1;
	GLint u_Ks = -1;
	GLint u_Kd = -1;
	GLint u_La = -1;
	GLint u_Ls = -1;
	GLint u_Ld = -1;
	GLint u_alpha = -1;
	GLint u_m = -1;
	GLint u_eta = -1;
	GLint u_option = -1;
};

struct LightPhong {
	glm::vec3 La = glm::vec3(1.0f);
	glm::vec3 Ld = glm::vec3(1.0f);
	glm::vec3 Ls = glm::vec3(1.0f);
	glm::vec3 pos = glm::vec3(0.0f);
	glm::vec3 target = glm::vec3(0.0f);
	float aperture = 0;
};

struct MaterialPhong {
	glm::vec3 Ka = glm::vec3(0.25f);
	glm::vec3 Kd = glm::vec3(0.5f);
	glm::vec3 Ks = glm::vec3(1.0f);
	float alpha = 1.0f;
};

struct MaterialCookTorrance {
	glm::vec3 Ka = glm::vec3(0.25f);
	glm::vec3 Kd = glm::vec3(0.5f);
	glm::vec3 Ks = glm::vec3(1.0f);
	float m = 1.0f;
	float eta = 1.0f;
};

LightPhong light;
MaterialPhong matPhong;
MaterialCookTorrance matCT;

FBO fbo;
Locations facesLoc;
Locations rayTracerPhongLoc;
Locations rayTracerCkTrrncLoc;
Mesh cube;

GLint window = 0;

//Global variables for the program logic
const int NUM_SAMPLES = 1;
GLenum texture_target;
float seconds_elapsed;
bool rotate;
bool wireframe;
glm::vec3 cubeTranslation;
float cubeScale;
glm::vec3 backgroundColor;
int current_scene;
int current_light_model;
int current_option;

void create_glut_window();
void init_program();
void init_OpenGL();
void create_fbo(int width, int height);
void delete_fbo();
void reload_shaders();
void pass_light_material();
void reset_camera();
void create_cube();
void drawCube(int pass);
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
	/* Always start with this call*/
	ImGui_ImplGLUT_NewFrame(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	/* Position of the menu, if no imgui.ini exist */
	ImGui::SetNextWindowSize(ImVec2(50, 50), ImGuiSetCond_FirstUseEver);

	ImGui::Begin("Raytracer options");
	if (ImGui::Button("Reload shader")) {
		reload_shaders();
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset camera")) {
		reset_camera();
	}
		
	ImGui::Text("Select scene:");
	ImGui::RadioButton("Sphere", &current_scene, 0); ImGui::SameLine();
	ImGui::RadioButton("Figure", &current_scene, 1); ImGui::SameLine();
	ImGui::RadioButton("CSG operation", &current_scene, 2);

	ImGui::Text("Select Lighting Model:");
	ImGui::RadioButton("Phong", &current_light_model, 0); ImGui::SameLine();
	ImGui::RadioButton("Cook Torrence", &current_light_model, 1); //ImGui::SameLine();
	//ImGui::RadioButton("Disney", &current_light_model, 2);

	if (current_light_model == 1) {
		ImGui::RadioButton("Full", &current_option, 0); ImGui::SameLine();
		ImGui::RadioButton("F", &current_option, 1); ImGui::SameLine();
		ImGui::RadioButton("D", &current_option, 2); ImGui::SameLine();
		ImGui::RadioButton("G", &current_option, 3);
	}

	if (ImGui::TreeNode("Light properties")) {
		ImGui::ColorEdit3("Ambient:", glm::value_ptr(light.La));
		ImGui::ColorEdit3("Diffuse:", glm::value_ptr(light.Ld));
		ImGui::ColorEdit3("Specular", glm::value_ptr(light.Ls));
		ImGui::SliderFloat3("Position:", glm::value_ptr(light.pos), -5.0f, 5.0f);
		ImGui::SliderFloat3("Target:", glm::value_ptr(light.target), -3.0f, 3.0f);
		ImGui::SliderFloat("Aperture:", &light.aperture, 0.0f, 90.0f);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Material properties")) {
		if (current_light_model == 0) {
			ImGui::ColorEdit3("Ambient:", glm::value_ptr(matPhong.Ka));
			ImGui::ColorEdit3("Diffuse:", glm::value_ptr(matPhong.Kd));
			ImGui::ColorEdit3("Specular", glm::value_ptr(matPhong.Ks));
			ImGui::SliderFloat("Shininess:", &matPhong.alpha, 0.0f, 256.0f);
		} else if (current_light_model == 1) {
			ImGui::ColorEdit3("Ambient:", glm::value_ptr(matCT.Ka));
			ImGui::ColorEdit3("Diffuse:", glm::value_ptr(matCT.Kd));
			ImGui::ColorEdit3("Specular", glm::value_ptr(matCT.Ks));
			ImGui::SliderFloat("m:", &matCT.m, 0.0f, 2.0f);
			ImGui::SliderFloat("etha:", &matCT.eta, 1.0f, 5.0f);
		} else {
		}
		ImGui::TreePop();
	}

	ImGui::Checkbox("Pause rotate", &rotate); ImGui::SameLine();
	ImGui::Checkbox("Wireframe", &wireframe);
	ImGui::SliderFloat3("Translation", glm::value_ptr(cubeTranslation), -10.0f, 10.0f);
	ImGui::SliderFloat("Scale", &cubeScale, 0.001f, 10.0f);

	ImGui::End();

	/* End with this when you want to render GUI */
	ImGui::Render();
}

void create_glut_window() {
	if (NUM_SAMPLES > 1) {
		glutSetOption(GLUT_MULTISAMPLE, NUM_SAMPLES);
		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	} else {
		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	}
	glutInitWindowPosition(5, 5);
	glutInitWindowSize(800, 600);
	window = glutCreateWindow("Jorge Garcia's Homework 3 Raycast");
}

void init_program() {
	using glm::vec3;
	/* Initialize global variables for program control */
	rotate = false;
	wireframe = false;
	seconds_elapsed = 0.0f;
	cubeTranslation = vec3(0.0f);
	cubeScale = 1.0f;
	backgroundColor = vec3(0.8f, 0.8f, 0.9f);
	current_light_model = 1;
	current_scene = 0;
	current_option = 0;
	//Set the material and light properties
	matCT.Ka = matPhong.Ka = vec3(0.1f, 0.2f, 0.1f);
	matCT.Kd = matPhong.Kd = vec3(0.5f, 0.7f, 0.5f);
	matCT.Ks = matPhong.Ks = vec3(0.6f, 0.6f, 0.6f);
	matPhong.alpha = 64.0f;
	matCT.eta = 3.2f;
	matCT.m = 0.3f;
	//Light
	light.La = vec3(1.0f, 1.0f, 1.0f);
	light.Ld = vec3(1.0f, 1.0f, 1.0f);
	light.Ls = vec3(1.0f, 1.0f, 1.0f);
	//Set the default position of the camera
	cam.setLookAt(vec3(0.0f, 0.0f, 3.0f), vec3(0.0f));
	cam.setAspectRatio(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	cam.setDepthView(0.1f, 5.0f);
	reset_camera();
	//Create trackball camera
	ball.setWindowSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
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

	if (NUM_SAMPLES > 1) {
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_SAMPLE_SHADING);
		glMinSampleShading(1.0f);
	}
	texture_target = NUM_SAMPLES > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	create_fbo(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	create_cube();
	reload_shaders();
}

void display() {
	using glm::vec3;
	using glm::vec4;
	using glm::vec2;
	using glm::mat4;

	mat4 I(1.0f);
	//Cube
	mat4 Q = glm::translate(I, cubeTranslation);
	Q = glm::scale(Q, cubeScale * vec3(1.0f));
	//Model
	mat4 M = rotate ? glm::rotate(I, TAU / 10.0f * seconds_elapsed, vec3(0.0f, 1.0f, 0.0f)) : I;
	//View
	mat4 V = cam.getViewMatrix() * ball.getRotation();
	//Projection
	mat4 P = cam.getProjectionMatrix();

	//Draw pass 1. Back faces of the cube to texture
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	programFacesPtr->use();

	glEnable(GL_CULL_FACE);
	//Set uniforms
	if (facesLoc.u_Q != -1) {
		glUniformMatrix4fv(facesLoc.u_Q, 1, GL_FALSE, glm::value_ptr(Q));
	}
	if (facesLoc.u_PVM != -1) {
		glUniformMatrix4fv(facesLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M));
	}
	glBindFramebuffer(GL_FRAMEBUFFER, fbo.id);
	glEnable(GL_DEPTH_TEST);
	
	glActiveTexture(GL_TEXTURE0);
	//Unbind texture so we can write to it (remember, no read-write access)	
	glBindTexture(texture_target, 0);
	
	//Subsequent drawing should be captured by the framebuffer attached texture
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//clear the color texture and depth texture
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//Draw back faces
	glCullFace(GL_FRONT);
	//Draw cube
	drawCube(1);

	//Pass 2: Raycasting pass
	if (current_light_model == 0) {
		programRayTracePhongPtr->use();
		if (rayTracerPhongLoc.u_Q != -1) {
			glUniformMatrix4fv(rayTracerPhongLoc.u_Q, 1, GL_FALSE, glm::value_ptr(Q));
		}
		if (rayTracerPhongLoc.u_PVM != -1) {
			glUniformMatrix4fv(rayTracerPhongLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M));
		}
		if (rayTracerPhongLoc.u_scene != -1) {
			glUniform1i(rayTracerPhongLoc.u_scene, current_scene);
		}
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(texture_target, fbo.backFacesTex);
		if (rayTracerPhongLoc.a_tex != -1) {
			glUniform1i(rayTracerPhongLoc.a_tex, 0); // we bound our texture to texture unit 0
		}
	} else if (current_light_model == 1) {
		programRayTraceCkTrrncPtr->use();
		if (rayTracerCkTrrncLoc.u_Q != -1) {
			glUniformMatrix4fv(rayTracerCkTrrncLoc.u_Q, 1, GL_FALSE, glm::value_ptr(Q));
		}
		if (rayTracerCkTrrncLoc.u_PVM != -1) {
			glUniformMatrix4fv(rayTracerCkTrrncLoc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M));
		}
		if (rayTracerCkTrrncLoc.u_scene != -1) {
			glUniform1i(rayTracerCkTrrncLoc.u_scene, current_scene);
		}
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(texture_target, fbo.backFacesTex);
		if (rayTracerCkTrrncLoc.a_tex != -1) {
			glUniform1i(rayTracerCkTrrncLoc.a_tex, 0); // we bound our texture to texture unit 0
		}
	}
	
	//Draw front faces to color attachment
	glCullFace(GL_BACK);
	
	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_CULL_FACE);
	}
	
	pass_light_material();

	glDrawBuffer(GL_COLOR_ATTACHMENT1);

	glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(texture_target, fbo.backFacesTex);
	//draw cube front faces to fbo
	drawCube(2);
	//blit color attachment 1 to the screen
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo.id);
	glReadBuffer(GL_COLOR_ATTACHMENT1);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, fbo.width, fbo.height, 0, 0, fbo.width, fbo.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	//Unbind an clean
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);

	/* You need to call this to draw the GUI, After unbinding your program*/
	drawGUI();
	/* But, before flushing the drawinng commands*/

	glutSwapBuffers();
}

void drawCube(int pass) {
	glBindBuffer(GL_ARRAY_BUFFER, cube.vertex_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.index_id);

	int a_pos;
	int a_tex;

	if (current_light_model == 0) {
		a_pos = (pass == 2) ? rayTracerPhongLoc.a_pos : facesLoc.a_pos;
		a_tex = (pass == 2) ? rayTracerPhongLoc.a_tex : facesLoc.a_tex;
	} else if (current_light_model == 1) {
		a_pos = (pass == 2) ? rayTracerCkTrrncLoc.a_pos : facesLoc.a_pos;
		a_tex = (pass == 2) ? rayTracerCkTrrncLoc.a_tex : facesLoc.a_tex;
	}

	glEnableVertexAttribArray(a_pos);
	glEnableVertexAttribArray(a_tex);

	glVertexAttribPointer(a_pos, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glVertexAttribPointer(a_tex, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(cube.num_vertex * 3 * sizeof(float)));

	glDrawElements(GL_TRIANGLES, cube.num_indices, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

	glDisableVertexAttribArray(a_pos);
	glDisableVertexAttribArray(a_tex);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void create_fbo(int width, int height) {
	delete_fbo();
	if (!ogl::framebufferStatus()) {
		std::cerr << "Something worng in frame buffer" << std::endl;
	}
	fbo.width = width;
	fbo.height = height;
	//Create empty textures
	GLuint textures[2];
	glGenTextures(2, textures);
	fbo.backFacesTex = textures[0];
	fbo.renderTex = textures[1];
	//See if we have multisample
	if (NUM_SAMPLES > 1) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbo.backFacesTex);
		glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_SAMPLES, GL_RGBA32F, fbo.width, fbo.height, true);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbo.renderTex);
		glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, NUM_SAMPLES, GL_RGBA32F, fbo.width, fbo.height, true);

		//Create the depth buffer for the framebuffer object
		glGenRenderbuffers(1, &fbo.depthTex);
		glBindRenderbuffer(GL_RENDERBUFFER, fbo.depthTex);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, NUM_SAMPLES, GL_DEPTH_COMPONENT, fbo.width, fbo.height);
	} else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fbo.backFacesTex);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, fbo.width, fbo.height);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, fbo.renderTex);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, fbo.width, fbo.height);

		//Create the depth buffer for the framebuffer object
		glGenRenderbuffers(1, &fbo.depthTex);
		glBindRenderbuffer(GL_RENDERBUFFER, fbo.depthTex);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fbo.width, fbo.height);
	}


	glGenFramebuffers(1, &fbo.id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo.id);

	if (NUM_SAMPLES > 1) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, fbo.backFacesTex, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, fbo.renderTex, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo.depthTex);
	} else {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo.backFacesTex, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fbo.renderTex, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo.depthTex);
	}
	if (!ogl::framebufferStatus()) {
		std::cerr << "Something worng in frame buffer" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void delete_fbo() {
	if (fbo.width != 0 && fbo.height != 0) {
		//Bind the default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//Bind default texture
		glBindTexture(GL_TEXTURE_2D, 0);
		//Delete depth texture
		glDeleteRenderbuffers(1, &fbo.depthTex);
		//Now, that you are sure they are not binded: delete them
		GLuint textures[2] = { fbo.renderTex, fbo.backFacesTex };
		glDeleteTextures(2, textures);
		//Now, that you are sure it is not binded: delete it
		glDeleteFramebuffers(1, &fbo.id);
	}
}

void create_cube() {
	using namespace glm;
	using namespace std;

	cube.num_vertex = 8;

	vector<vec3> positions(cube.num_vertex, vec3(0.0f));
	vector<vec3> textureCoord(cube.num_vertex, vec3(0.0f));
	vector<unsigned short> indices;

	textureCoord[0] = positions[0] = vec3(-1.0f, -1.0f, -1.0f);
	textureCoord[1] = positions[1] = vec3(+1.0f, -1.0f, -1.0f);
	textureCoord[2] = positions[2] = vec3(+1.0f, +1.0f, -1.0f);
	textureCoord[3] = positions[3] = vec3(-1.0f, +1.0f, -1.0f);
	textureCoord[4] = positions[4] = vec3(-1.0f, -1.0f, +1.0f);
	textureCoord[5] = positions[5] = vec3(+1.0f, -1.0f, +1.0f);
	textureCoord[6] = positions[6] = vec3(+1.0f, +1.0f, +1.0f);
	textureCoord[7] = positions[7] = vec3(-1.0f, +1.0f, +1.0f);

	//bottom
	indices.push_back(0); indices.push_back(2); indices.push_back(1);
	indices.push_back(2); indices.push_back(0); indices.push_back(3);
	//front
	indices.push_back(0); indices.push_back(5); indices.push_back(4);
	indices.push_back(5); indices.push_back(0); indices.push_back(1);
	//right
	indices.push_back(1); indices.push_back(6); indices.push_back(5);
	indices.push_back(6); indices.push_back(1); indices.push_back(2);
	//back
	indices.push_back(2); indices.push_back(7); indices.push_back(6);
	indices.push_back(7); indices.push_back(2); indices.push_back(3);
	//left
	indices.push_back(3); indices.push_back(4); indices.push_back(7);
	indices.push_back(4); indices.push_back(3); indices.push_back(0);
	//top
	indices.push_back(4); indices.push_back(5); indices.push_back(6);
	indices.push_back(6); indices.push_back(7); indices.push_back(4);

	cube.num_indices = static_cast<int>(indices.size());

	int sizePositions = static_cast<int>(positions.size() * sizeof(vec3));
	int sizeTextCoord = static_cast<int>(textureCoord.size() * sizeof(vec3));
	//Sent to GPU the vertices
	glGenBuffers(1, &cube.vertex_id);
	glBindBuffer(GL_ARRAY_BUFFER, cube.vertex_id);
	glBufferData(GL_ARRAY_BUFFER, sizePositions + sizeTextCoord, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizePositions, positions.data());
	glBufferSubData(GL_ARRAY_BUFFER, sizePositions, sizeTextCoord, textureCoord.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//Send to GPU the indices
	int sizeIndices = static_cast<int>(indices.size() * sizeof(unsigned short));
	glGenBuffers(1, &cube.index_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.index_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeIndices, indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void reload_shaders() {
	using namespace std;
	/************************************************************************/
	/*                   OpenGL program creation                            */
	/************************************************************************/
	
	if (NUM_SAMPLES > 1) {
		programRayTracePhongPtr = new OGLProgram("shaders/vshader.glsl", "shaders/fPhong_ms.glsl");
		programRayTraceCkTrrncPtr = new OGLProgram("shaders/vshader.glsl", "shaders/fCookTorrance_ms.glsl");
		programFacesPtr = new OGLProgram("shaders/vshader.glsl", "shaders/fshaderFaces_ms.glsl");
	} else {
		programRayTracePhongPtr = new OGLProgram("shaders/vshader.glsl", "shaders/fPhong.glsl");
		programRayTraceCkTrrncPtr = new OGLProgram("shaders/vshader.glsl", "shaders/fCookTorrance.glsl");
		programFacesPtr = new OGLProgram("shaders/vshader.glsl", "shaders/fshaderFaces.glsl");
	}

	//Use background color as a hint to the user that something is wrong in the shader
	if (!programRayTracePhongPtr || !programRayTracePhongPtr->isOK()) {
		cerr << "Something wrong in ray tracer Phong shaders" << endl;
		backgroundColor = glm::vec3(1.0f, 1.0f, 0.0f);
	}
	if (!programRayTraceCkTrrncPtr || !programRayTraceCkTrrncPtr->isOK()) {
		cerr << "Something wrong in ray tracer Cook-Torrance shaders" << endl;
		backgroundColor = glm::vec3(1.0f, 1.0f, 0.0f);
	}
	if (!programFacesPtr || !programFacesPtr->isOK()) {
		cerr << "Something wrong in render faces shaders" << endl;
		backgroundColor = glm::vec3(1.0f, 1.0f, 0.0f);
	}

	if (programRayTracePhongPtr->isOK() && programFacesPtr->isOK() && programRayTraceCkTrrncPtr->isOK()) {
		cout << "Shaders compiled correctlly!" << endl;
		backgroundColor = glm::vec3(0.8f, 0.8f, 0.9f);
	}
	/************************************************************************/
	/*                   Shader variables locations                         */
	/************************************************************************/
	programFacesPtr->use();
	//Atributes
	facesLoc.a_pos = programFacesPtr->attribLoc("pos_attrib");
	facesLoc.a_tex = programFacesPtr->attribLoc("tex_coord_attrib");
	//Uniforms
	facesLoc.u_Q = programFacesPtr->uniformLoc("Q");
	facesLoc.u_PVM = programFacesPtr->uniformLoc("PVM");

	glUseProgram(0);
	programRayTracePhongPtr->use();
	//Atributes
	rayTracerPhongLoc.a_pos = programRayTracePhongPtr->attribLoc("pos_attrib");
	rayTracerPhongLoc.a_tex = programRayTracePhongPtr->attribLoc("tex_coord_attrib");
	//Uniforms
	rayTracerPhongLoc.u_Q = programRayTracePhongPtr->uniformLoc("Q");
	rayTracerPhongLoc.u_PVM = programRayTracePhongPtr->uniformLoc("PVM");
	rayTracerPhongLoc.u_scene = programRayTracePhongPtr->uniformLoc("scene");
	//Material
	rayTracerPhongLoc.u_Ka = programRayTracePhongPtr->uniformLoc("mat.Ka");
	rayTracerPhongLoc.u_Kd = programRayTracePhongPtr->uniformLoc("mat.Kd");
	rayTracerPhongLoc.u_Ks = programRayTracePhongPtr->uniformLoc("mat.Ks");
	rayTracerPhongLoc.u_alpha = programRayTracePhongPtr->uniformLoc("mat.alpha");
	rayTracerPhongLoc.u_eta = programRayTracePhongPtr->uniformLoc("mat.eta");
	rayTracerPhongLoc.u_m = programRayTracePhongPtr->uniformLoc("mat.m");
	//Light
	rayTracerPhongLoc.u_La = programRayTracePhongPtr->uniformLoc("light.La");
	rayTracerPhongLoc.u_Ld = programRayTracePhongPtr->uniformLoc("light.Ld");
	rayTracerPhongLoc.u_Ls = programRayTracePhongPtr->uniformLoc("light.Ls");

	rayTracerPhongLoc.u_Tex = programRayTracePhongPtr->uniformLoc("backfaces");

	glUseProgram(0);
	programRayTraceCkTrrncPtr->use();
	//Atributes
	rayTracerCkTrrncLoc.a_pos = programRayTraceCkTrrncPtr->attribLoc("pos_attrib");
	rayTracerCkTrrncLoc.a_tex = programRayTraceCkTrrncPtr->attribLoc("tex_coord_attrib");
	//Uniforms
	rayTracerCkTrrncLoc.u_Q = programRayTraceCkTrrncPtr->uniformLoc("Q");
	rayTracerCkTrrncLoc.u_PVM = programRayTraceCkTrrncPtr->uniformLoc("PVM");
	rayTracerCkTrrncLoc.u_scene = programRayTraceCkTrrncPtr->uniformLoc("scene");
	rayTracerCkTrrncLoc.u_option = programRayTraceCkTrrncPtr->uniformLoc("option");
	//Material
	rayTracerCkTrrncLoc.u_Ka = programRayTraceCkTrrncPtr->uniformLoc("mat.Ka");
	rayTracerCkTrrncLoc.u_Kd = programRayTraceCkTrrncPtr->uniformLoc("mat.Kd");
	rayTracerCkTrrncLoc.u_Ks = programRayTraceCkTrrncPtr->uniformLoc("mat.Ks");
	rayTracerCkTrrncLoc.u_alpha = programRayTraceCkTrrncPtr->uniformLoc("mat.alpha");
	rayTracerCkTrrncLoc.u_eta = programRayTraceCkTrrncPtr->uniformLoc("mat.eta");
	rayTracerCkTrrncLoc.u_m = programRayTraceCkTrrncPtr->uniformLoc("mat.m");
	//Light
	rayTracerCkTrrncLoc.u_La = programRayTraceCkTrrncPtr->uniformLoc("light.La");
	rayTracerCkTrrncLoc.u_Ld = programRayTraceCkTrrncPtr->uniformLoc("light.Ld");
	rayTracerCkTrrncLoc.u_Ls = programRayTraceCkTrrncPtr->uniformLoc("light.Ls");

	rayTracerCkTrrncLoc.u_Tex = programRayTraceCkTrrncPtr->uniformLoc("backfaces");
	
	glUseProgram(0);
}
void pass_light_material() {
	/*Material*/
	if (current_light_model == 0) {
		if (rayTracerPhongLoc.u_Ka != -1) {
			glUniform3fv(rayTracerPhongLoc.u_Ka, 1, glm::value_ptr(matPhong.Ka));
		}

		if (rayTracerPhongLoc.u_Ks != -1) {
			glUniform3fv(rayTracerPhongLoc.u_Ks, 1, glm::value_ptr(matPhong.Ks));
		}

		if (rayTracerPhongLoc.u_Kd != -1) {
			glUniform3fv(rayTracerPhongLoc.u_Kd, 1, glm::value_ptr(matPhong.Kd));
		}

		if (rayTracerPhongLoc.u_alpha != -1) {
			glUniform1f(rayTracerPhongLoc.u_alpha, matPhong.alpha);
		}

		/*Light*/
		if (rayTracerPhongLoc.u_La != -1) {
			glUniform3fv(rayTracerPhongLoc.u_La, 1, glm::value_ptr(light.La));
		}

		if (rayTracerPhongLoc.u_Ld != -1) {
			glUniform3fv(rayTracerPhongLoc.u_Ld, 1, glm::value_ptr(light.Ld));
		}

		if (rayTracerPhongLoc.u_Ls != -1) {
			glUniform3fv(rayTracerPhongLoc.u_Ls, 1, glm::value_ptr(light.Ls));
		}

	} else if (current_light_model == 1) {
		if (rayTracerCkTrrncLoc.u_Ka != -1) {
			glUniform3fv(rayTracerCkTrrncLoc.u_Ka, 1, glm::value_ptr(matCT.Ka));
		}

		if (rayTracerCkTrrncLoc.u_Ks != -1) {
			glUniform3fv(rayTracerCkTrrncLoc.u_Ks, 1, glm::value_ptr(matCT.Ks));
		}

		if (rayTracerCkTrrncLoc.u_Kd != -1) {
			glUniform3fv(rayTracerCkTrrncLoc.u_Kd, 1, glm::value_ptr(matCT.Kd));
		}

		if (rayTracerCkTrrncLoc.u_eta != -1) {
			glUniform1f(rayTracerCkTrrncLoc.u_eta, matCT.eta);
		}

		if (rayTracerCkTrrncLoc.u_m != -1) {
			glUniform1f(rayTracerCkTrrncLoc.u_m, matCT.m);
		}

		if (rayTracerCkTrrncLoc.u_option != -1) {
			glUniform1i(rayTracerCkTrrncLoc.u_option, current_option);
		}

		/*Light*/
		if (rayTracerCkTrrncLoc.u_La != -1) {
			glUniform3fv(rayTracerCkTrrncLoc.u_La, 1, glm::value_ptr(light.La));
		}

		if (rayTracerCkTrrncLoc.u_Ld != -1) {
			glUniform3fv(rayTracerCkTrrncLoc.u_Ld, 1, glm::value_ptr(light.Ld));
		}

		if (rayTracerCkTrrncLoc.u_Ls != -1) {
			glUniform3fv(rayTracerCkTrrncLoc.u_Ls, 1, glm::value_ptr(light.Ls));
		}
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

void exit_glut() {
	delete programFacesPtr;
	delete programRayTracePhongPtr;
	delete programRayTraceCkTrrncPtr;
	/* Shut down the gui */
	ImGui_ImplGLUT_Shutdown();
	/* Delete window (freeglut) */
	glutDestroyWindow(window);
	exit(EXIT_SUCCESS);
}

void reset_camera() {
	cam.setFovY(PI / 8.0f);
	ball.resetRotation();
}

void reshape(int new_window_width, int new_window_height) {
	glViewport(0, 0, new_window_width, new_window_height);
	cam.setAspectRatio(new_window_width, new_window_height);
	ball.setWindowSize(new_window_width, new_window_height);
	if (fbo.width != new_window_width || fbo.height != new_window_height) {
		create_fbo(new_window_width, new_window_height);
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

	/* Now, the app */
	switch (key) {
		case '1':
			current_scene = 0;
		break;

		case '2':
			current_scene = 1;
		break;

		case '3':
			current_scene = 2;
		break;

		case 'R':
		case 'r':
			rotate = !rotate;
		break;

		case 'W':
		case 'w':
			wireframe = !wireframe;
		break;

		case 27:
			exit_glut();
		break;

		case 'c':
		case 'C':
			reset_camera();
		break;

		case ' ':
			reload_shaders();
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
		} else {
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
		case GLUT_KEY_UP:
			cubeTranslation.y -= 0.1f * cubeScale;
		break;

		case GLUT_KEY_DOWN:
			cubeTranslation.y += 0.1f * cubeScale;
		break;

		case GLUT_KEY_LEFT:
			cubeTranslation.x += 0.1f * cubeScale;
		break;

		case GLUT_KEY_RIGHT:
			cubeTranslation.x -= 0.1f * cubeScale;
		break;

		case GLUT_KEY_PAGE_UP:
			cubeScale += 0.1f;
		break;

		case GLUT_KEY_PAGE_DOWN:
			cubeScale -= 0.1f;
		break;

		case GLUT_KEY_F1:
			current_light_model = 0;
		break;

		case GLUT_KEY_F2:
			current_light_model = 1;
		break;

		case GLUT_KEY_F3:
			current_light_model = 2;
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
	} else if (dir < 0) {
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

