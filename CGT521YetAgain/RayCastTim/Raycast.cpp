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
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const float TO_RADIANS = 0.0174533f;

#include "InitShader.h"

/*Headers needed for imgui */
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glut.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;
static int win = 0;

//opengl shader object
GLuint program = -1;

//shader uniform locations
int M_loc = -1;
int Q_loc = -1;
int PV_loc = -1;
int Time_loc = -1;
int Pass_loc = -1;

//shader attrib locations
int pos_loc = -1;
int tex_coord_loc = -1;

bool pauseRotation = true;
float rev_per_min = 0.0f;
float seconds = 0.0f;
float Qscale = 1.0f;
glm::vec3 Qtrans(0.0f, 0.0f, 0.0f);

GLuint VBO1;
GLuint IndexBuffer;
GLuint TexBackFaces = 0;
GLuint TexFrontFaces = 0;
GLuint TexRender = 0;
GLuint FBOID = 0;       // framebuffer object,

int WindowWidth = 800;
int WindowHeight = 800;

const int num_samples = 8;

void create_glut_window();
void reload_shaders();
void init_program();
void init_OpenGL();
void bufferIndexedVertsUsingMapBuffer();
void drawIndexedVerts();
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
	init_program();

	/*You need to call this once at the begining of your program for ImGui to work*/
	ImGui_ImplGLUT_Init();

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

	ImGui::Begin("My options");
		if (ImGui::Button("Reload shader"))	{
			reload_shaders();
		}
		ImGui::Checkbox("Pause rotate", &pauseRotation);
		ImGui::SliderFloat3("Trans", &Qtrans.x, -10.0f, 10.0f);
		ImGui::SliderFloat("Scale", &Qscale, 0.001f, 10.0f);

		static bool wireframe = false;
		ImGui::Checkbox("Wireframe", &wireframe);
		if (wireframe == true) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		} else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	ImGui::End();

	/* End with this when you want to render GUI */
	ImGui::Render();
}

void exit_glut() {
	glutDestroyWindow(win);
	exit(EXIT_SUCCESS);
}

void create_glut_window() {
	//Set number of samples per pixel
	if (num_samples <= 1) {
		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	} else {
		glutSetOption(GLUT_MULTISAMPLE, num_samples);
		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	}
	
	glutInitWindowPosition(5, 5);
	glutInitWindowSize(WindowWidth, WindowHeight);
	win = glutCreateWindow("Ray casting (Tim's version)");
}

void reload_shaders() {
	if (program != -1) {
		glDeleteProgram(program);
	}

	//create and load shaders

	if (num_samples <= 1) {
		program = InitShader("vshader.glsl", "fshader.glsl");
	} else {
		program = InitShader("vshader.glsl", "fshader_ms.glsl"); //multisample version of fragment shader
	}

	glUseProgram(program);

	pos_loc = glGetAttribLocation(program, "pos_attrib");
	tex_coord_loc = glGetAttribLocation(program, "tex_coord_attrib");

	Q_loc = glGetUniformLocation(program, "Q");
	M_loc = glGetUniformLocation(program, "M");
	PV_loc = glGetUniformLocation(program, "PV");
	Time_loc = glGetUniformLocation(program, "time");
	Pass_loc = glGetUniformLocation(program, "pass");

	int tex_loc = glGetUniformLocation(program, "backfaces");
	if (tex_loc != -1) {
		glUniform1i(tex_loc, 0);
	}
}

void init_program() {
	
}

void bufferIndexedVertsUsingMapBuffer() {
	using namespace glm;
	vec3 pos[8] = { vec3(-1.0f, -1.0f, -1.0f), vec3(+1.0f, -1.0f, -1.0f), vec3(+1.0f, +1.0f, -1.0f), vec3(-1.0f, +1.0f, -1.0f),
		vec3(-1.0f, -1.0f, +1.0f), vec3(+1.0f, -1.0f, +1.0f), vec3(+1.0f, +1.0f, +1.0f), vec3(-1.0f, +1.0f, +1.0f) };

	vec3 tex[8] = { vec3(-1.0f, -1.0f, -1.0f), vec3(+1.0f, -1.0f, -1.0f), vec3(+1.0f, +1.0f, -1.0f), vec3(-1.0f, +1.0f, -1.0f),
		vec3(-1.0f, -1.0f, +1.0f), vec3(+1.0f, -1.0f, +1.0f), vec3(+1.0f, +1.0f, +1.0f), vec3(-1.0f, +1.0f, +1.0f) };

	unsigned short idx[36] = { 0,2,1, 2,0,3, //bottom
		0,5,4, 5,0,1, //front
		1,6,5, 6,1,2, //right 
		2,7,6, 7,2,3, //back
		3,4,7, 4,3,0, //left
		4,5,6, 6,7,4 };//top
					   //Buffer vertices
	int datasize = sizeof(pos) + sizeof(tex);
	glGenBuffers(1, &VBO1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, datasize, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pos), pos);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(pos), sizeof(tex), tex);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Buffer indices
	glGenBuffers(1, &IndexBuffer);
	int nIndices = 12;
	int indexsize = sizeof(idx);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexsize, idx, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void drawIndexedVerts() {
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);

	glEnableVertexAttribArray(pos_loc);
	glEnableVertexAttribArray(tex_coord_loc);

	glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(0));
	glVertexAttribPointer(tex_coord_loc, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(8 * 3 * sizeof(float)));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

	glDisableVertexAttribArray(pos_loc);
	glDisableVertexAttribArray(tex_coord_loc);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
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

	cout << "Hardware specification: " << endl;
	cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Software specification: " << endl;
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;
	cout << "Using OpenGL " << glGetString(GL_VERSION) << endl;
	cout << "Using GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	int ver = glutGet(GLUT_VERSION);
	cout << "Using freeglut version: " << ver / 10000 << "." << (ver / 100) % 100 << "." << ver % 100 << endl;

	if (num_samples > 1) {
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_SAMPLE_SHADING);
		glMinSampleShading(1.0f);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	//create empty textures
	GLuint tex_target = GL_TEXTURE_2D_MULTISAMPLE;
	if (num_samples <= 1) {
		tex_target = GL_TEXTURE_2D;
	}

	glGenTextures(1, &TexBackFaces);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(tex_target, TexBackFaces);
	if (tex_target == GL_TEXTURE_2D) {
		glTexStorage2D(tex_target, 1, GL_RGBA32F, WindowWidth, WindowHeight);
	} else {
		glTexStorage2DMultisample(tex_target, num_samples, GL_RGBA32F, WindowWidth, WindowHeight, true);
	}

	glGenTextures(1, &TexFrontFaces);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(tex_target, TexFrontFaces);
	if (tex_target == GL_TEXTURE_2D) {
		glTexStorage2D(tex_target, 1, GL_RGBA32F, WindowWidth, WindowHeight);
	} else {
		glTexStorage2DMultisample(tex_target, num_samples, GL_RGBA32F, WindowWidth, WindowHeight, true);
	}

	glGenTextures(1, &TexRender);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(tex_target, TexRender);
	if (tex_target == GL_TEXTURE_2D) {
		glTexStorage2D(tex_target, 1, GL_RGBA32F, WindowWidth, WindowHeight);
	} else {
		glTexStorage2DMultisample(tex_target, num_samples, GL_RGBA32F, WindowWidth, WindowHeight, true);
	}


	//create the depth buffer for the framebuffer object
	GLuint RBOID = 0;
	glGenRenderbuffers(1, &RBOID);
	glBindRenderbuffer(GL_RENDERBUFFER, RBOID);
	if (tex_target == GL_TEXTURE_2D) {
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WindowWidth, WindowHeight);
	} else {
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, num_samples, GL_DEPTH_COMPONENT, WindowWidth, WindowHeight);
	}


	//create the framebuffer object and attach the cubemap faces and the depth buffer
	glGenFramebuffers(1, &FBOID);
	glBindFramebuffer(GL_FRAMEBUFFER, FBOID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_target, TexBackFaces, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, tex_target, TexFrontFaces, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, tex_target, TexRender, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBOID);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	bufferIndexedVertsUsingMapBuffer();
	reload_shaders();
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
	WindowHeight = new_window_height;
	WindowWidth = new_window_width;
}

void display() {

	using namespace glm;
	mat4 M = rotate(6.0f * rev_per_min * TO_RADIANS, vec3(0.0f, 0.0f, 1.0f));
	mat4 V = lookAt(vec3(3.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
	mat4 P = perspective(70.0f * TO_RADIANS, 1.0f, 0.1f, 100.0f);

	mat4 Q = translate(Qtrans) * scale(vec3(Qscale));
	vec4 campos = inverse(V * M) * vec4(0.0f, 0.0f, 0.0f, 1.0f);
	
	//draw pass 1: back faces of cube to texture
	glUseProgram(program);
	glEnable(GL_CULL_FACE);
	//set uniforms
	if (M_loc != -1) {
		glUniformMatrix4fv(M_loc, 1, false, glm::value_ptr(M));
	}

	if (Q_loc != -1) {
		glUniformMatrix4fv(Q_loc, 1, false, glm::value_ptr(Q));
	}

	if (PV_loc != -1) {
		glUniformMatrix4fv(PV_loc, 1, false, glm::value_ptr(P * V));
	}

	glBindFramebuffer(GL_FRAMEBUFFER, FBOID);
	glEnable(GL_DEPTH_TEST);

	if (Pass_loc != -1) {
		glUniform1i(Pass_loc, 1);
	}

	GLuint tex_target = GL_TEXTURE_2D_MULTISAMPLE;
	if (num_samples <= 1) {
		tex_target = GL_TEXTURE_2D;
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(tex_target, 0); //unbind texture so we can write to it (remember, no read-write access)
									//Subsequent drawing should be captured by the framebuffer attached texture
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//clear the color texture and depth texture
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glCullFace(GL_FRONT); //draw back faces
	drawIndexedVerts(); //draw cube


	if (Pass_loc != -1)	{
		glUniform1i(Pass_loc, 2);
	}
	//pass 2: draw front faces to attachment 1
	glCullFace(GL_BACK);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(tex_target, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT1);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//draw cube front faces to fbo
	drawIndexedVerts();

	//raycasting pass
	if (Pass_loc != -1) {
		glUniform1i(Pass_loc, 3);
	}
	//pass 2: draw front faces to attachment 1
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(tex_target, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT2);

	glClearColor(0.8f, 0.8f, 0.9f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(tex_target, TexBackFaces);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(tex_target, TexFrontFaces);

	//draw cube front faces to fbo
	drawIndexedVerts();

	//blit color attachment 2 to the screen
	glBindFramebuffer(GL_READ_FRAMEBUFFER, FBOID);
	glReadBuffer(GL_COLOR_ATTACHMENT2);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, 800, 800, 0, 0, 800, 800, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	
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

void idle() {
	static int last_time = 0;

	int time = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = time - last_time;
	last_time = time;
	float delta_seconds = 0.001f * elapsed;
	seconds += delta_seconds;

	if (pauseRotation == false)	{
		rev_per_min = 6.0f * seconds;
	}

	//set shader time uniform variable
	glUseProgram(program);

	if (Time_loc != -1)	{
		glUniform1f(Time_loc, seconds);
	}

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

		case 'T':
		case 't':
			seconds = 0.0f;
		break;

		case 'W':
		case 'w':
			Qtrans.x += 0.1f * Qscale;
		break;

		case 'A':
		case 'a':
			Qtrans.y -= 0.1f * Qscale;
		break;

		case 'S':
		case 's':
			Qtrans.x -= 0.1f * Qscale;
		break;

		case 'D':
		case 'd':
			Qtrans.y += 0.1f * Qscale;
		break;

		case 27:
			exit_glut();
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

	/* Now, the app */



	glutPostRedisplay();
}

void special(int key, int mouse_x, int mouse_y) {
	/* See if ImGui handles it*/
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharacter(key);

	switch (key) {
		case GLUT_KEY_F1:
			pauseRotation = !pauseRotation;
		break;

		case GLUT_KEY_UP:
			Qtrans.z += 0.1f * Qscale;
		break;

		case GLUT_KEY_DOWN:
			Qtrans.z -= 0.1f * Qscale;
		break;

		case GLUT_KEY_LEFT:
			Qscale = Qscale * 0.99f;
		break;

		case GLUT_KEY_RIGHT:
			Qscale = Qscale * 1.01f;
		break;
	}

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

	/* Now, the app*/


	glutPostRedisplay();
}

void mouseDrag(int mouse_x, int mouse_y) {
	/* See if ImGui handles it*/
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2(float(mouse_x), float(mouse_y));

	/*Now, the app*/

	glutPostRedisplay();
}

void mousePasiveMotion(int mouse_x, int mouse_y) {
	/* See if ImGui handles it*/
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2(float(mouse_x), float(mouse_y));

	/*Now, the app*/

	glutPostRedisplay();
}
