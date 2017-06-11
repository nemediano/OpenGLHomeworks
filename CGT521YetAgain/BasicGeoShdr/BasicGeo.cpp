#include "stdafx.h"

/* Include heders and namespaces from my CGT Library */
#include "OGLHelpers.h"
#include "MathConstants.h"
#include "OGLProgram.h"
#include "Geometries.h"
#include "ScreenGrabber.h"

using namespace ogl;
using namespace math;
using namespace mesh;
using namespace image;

/* CGT Library related*/
OGLProgram* programPtr = nullptr;

GLint window = 0;
// Location for shader variables
struct Locations {
	GLint u_PVM = -1;
	GLint u_M = -1;
	GLint u_Time = -1;
	GLint a_position = -1;
	GLint a_velocity = -1;
	GLint a_mass = -1;
};

struct Particle {
	glm::vec3 position;
	glm::vec3 velocity;
	float mass;
};

struct GPUInterface {
	GLuint vbo = 0;
	GLuint vao = 0;
};

GPUInterface buffers;

std::vector<Particle> particles;

Locations loc;

//Global variables for the program logic
float seconds_elapsed;
float delta_time;
float world_size;
glm::vec4 scene_limits;

ScreenGrabber grabber;

//Imgui related function
void drawGUI();

void create_glut_window();
void init_program();
void init_OpenGL();
void create_glut_callbacks();
void exit_glut();
void reload_shaders();

//Glut callback functions
void display();
void reshape(int new_window_width, int new_window_height);
void idle();
void keyboard(unsigned char key, int mouse_x, int mouse_y);
void special(int key, int mouse_x, int mouse_y);
void mouse(int button, int state, int x, int y);
void mouseWheel(int button, int dir, int mouse_x, int mouse_y);
void mouseDrag(int mouse_x, int mouse_y);
void mousePasiveMotion(int mouse_x, int mouse_y);

void drawGUI() {
	using glm::vec3;

	/* Always start with this call*/
	ImGui_ImplGLUT_NewFrame(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	/* Position of the menu, if no imgui.ini exist */
	ImGui::SetNextWindowSize(ImVec2(50, 50), ImGuiSetCond_FirstUseEver);

	/*Create a new menu for my app*/
	ImGui::Begin("Options");

	if (ImGui::Button("Quit")) {
		exit_glut();
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
	delete programPtr;
	/*Delete Buffers on GPU*/
	glDeleteVertexArrays(1, &buffers.vao);
	glDeleteBuffers(1, &buffers.vbo);
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
	glutInitWindowSize(800, 600);
	window = glutCreateWindow("Geometry shader test");
}

void init_program() {
	using glm::vec3;
	/* Initialize global variables for program control */
	seconds_elapsed = 0.0f;
	delta_time = 0.0f;
	world_size = 3.0f;

	/*Create vertices*/
	int numParticles = 1e3f;

	unsigned int seed = static_cast<unsigned int>(time(nullptr));
	std::cout << "Random number seed: " << seed << std::endl;
	srand(seed);

	Particle p;
	for (int i = 0; i < numParticles; ++i) {
		p.position = vec3(glm::linearRand(glm::vec2(-4.0, -3.0), glm::vec2(4.0, 3.0)), 0.0f);
		p.velocity = glm::linearRand(vec3(0.2f), vec3(1.0f));
		p.mass = glm::linearRand(0.05f, 0.1f);
		particles.push_back(p);
	}

	//Send data to GPU
	glGenBuffers(1, &buffers.vbo);
	glGenVertexArrays(1, &buffers.vao);
	glBindVertexArray(buffers.vao);

	glBindBuffer(GL_ARRAY_BUFFER, buffers.vbo);
	glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(Particle), particles.data(), GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(loc.a_position);
	glVertexAttribPointer(loc.a_position, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), OFFSET_OF(Particle, position));

	glEnableVertexAttribArray(loc.a_velocity);
	glVertexAttribPointer(loc.a_velocity, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), OFFSET_OF(Particle, velocity));

	glEnableVertexAttribArray(loc.a_mass);
	glVertexAttribPointer(loc.a_mass, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), OFFSET_OF(Particle, mass));

	glBindVertexArray(0);
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
	cout << getOpenGLInfo() << endl;
	
	reload_shaders();

	//Activate antialliasing
	glEnable(GL_MULTISAMPLE);

	//Initialize some basic rendering state
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);
}

void reload_shaders() {
	using std::cerr;
	using std::endl;
	/************************************************************************/
	/*                   OpenGL program creation                            */
	/************************************************************************/
	programPtr = new OGLProgram("shaders/particles.vert", "shaders/particles.frag", "shaders/particles.geom");

	if (!programPtr || !programPtr->isOK()) {
		cerr << "Something wrong in shader" << endl;
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	}

	if (programPtr->isOK()) {
		cerr << "Shaders compiled" << endl;
		glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	}

	/************************************************************************/
	/* Allocating variables for shaders                                     */
	/************************************************************************/
	loc.u_PVM = programPtr->uniformLoc("PVM");
	loc.u_M = programPtr->uniformLoc("M");
	loc.u_Time = programPtr->uniformLoc("time");
	loc.a_position = programPtr->attribLoc("Position");
	loc.a_velocity = programPtr->attribLoc("Velocity");
	loc.a_mass = programPtr->attribLoc("Mass");
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
	grabber.resize(new_window_width, new_window_height);

	if (new_window_width <= new_window_height) {
		scene_limits[0] = world_size;
		scene_limits[2] = world_size * new_window_height / new_window_width;
	} else {
		scene_limits[0] = world_size * new_window_width / new_window_height;
		scene_limits[2] = world_size;
	}

	scene_limits[1] = -scene_limits[0];
	scene_limits[3] = -scene_limits[2];
}

void display() {
	using glm::vec3;
	using glm::vec4;
	using glm::vec2;
	using glm::mat4;
	using namespace std;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	programPtr->use();

	mat4 I(1.0f);
	//Model
	mat4 M = I;
	//View
	mat4 V = glm::lookAt(vec3(0.0f, 0.0f, 1.0f), vec3(0.0f), vec3(0.0, 1.0, 0.0));
	//Projection
	/*In ortho as in orthoRH, the far and near coordinate are inverted.
	Either pass it inverted or use orthoLH*/
	mat4 P = glm::ortho(scene_limits[1], scene_limits[0], scene_limits[3], scene_limits[2], 1.0f, -1.0f);

	/************************************************************************/
	/* Send uniform values to shader                                        */
	/************************************************************************/
	glUniformMatrix4fv(loc.u_PVM, 1, GL_FALSE, glm::value_ptr(P * V * M));
	glUniformMatrix4fv(loc.u_M, 1, GL_FALSE, glm::value_ptr(M));
	glUniform1f(loc.u_Time, delta_time);

	glBindVertexArray(buffers.vao);
	glDrawArrays(GL_POINTS, 0, static_cast<int>(particles.size()));

	
	//Unbind an clean
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
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
	delta_time = 0.001f * elapsed;
	seconds_elapsed += delta_time;

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

	

	glutPostRedisplay();
}

void special(int key, int mouse_x, int mouse_y) {
	/* See if ImGui handles it*/
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharacter(key);

	/*switch (key) {
		case GLUT_KEY_F1:
			rotateScene = !rotateScene;
		break;
	}*/

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

	glutPostRedisplay();
}

void mouseDrag(int mouse_x, int mouse_y) {
	/* See if ImGui handles it*/
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2(float(mouse_x), float(mouse_y));


	glutPostRedisplay();
}

void mousePasiveMotion(int mouse_x, int mouse_y) {
	/* See if ImGui handles it*/
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2(float(mouse_x), float(mouse_y));

	/*Now, the app*/
	glutPostRedisplay();
}

