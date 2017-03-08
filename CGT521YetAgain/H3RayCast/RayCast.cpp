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

/*For image loading */
#include <FreeImage.h>
//For obj loading
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"

// Define a helpful macro for handling offsets into buffer objects
#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))
#define OFFSET_OF(type, member) ((GLvoid*)(offsetof(type, member)))
//Math constant equal two PI
const float TAU = 6.28318f;
const float PI = 3.14159f;

struct Vertex {
	glm::vec3 position;
	glm::vec2 textCoord;
	glm::vec3 normal;
};


GLint window = 0;
// Location for shader variables
GLint u_PVM_location = -1;
GLint u_texture_location = -1;
GLint a_position_loc = -1;
GLint a_normal_loc = -1;
GLint a_texture_loc = -1;
// OpenGL program handlers
GLuint vertex_shader;
GLuint fragment_shader;
GLuint program;
GLuint texture_id;

//Global variables for the program logic
int nTriangles;
bool rotate;
float seconds_elapsed;
float angle;

//Manage the Vertex Buffer Object
GLuint vbo;
GLuint indexBuffer;

void create_glut_window();
void init_program();
void init_OpenGL();
void create_primitives();
bool load_texture(const std::string& file_name);
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

//OpenGL debug context
void APIENTRY openglCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);

	create_glut_window();
	create_glut_callbacks();

	init_OpenGL();
	/*You need to call this once at the begining of your program for ImGui to work*/
	ImGui_ImplGLUT_Init();
	init_program();


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

	/*Create a new menu for my app*/
	ImGui::Begin("My options");
	ImGui::Checkbox("Rotation", &rotate);
	if (ImGui::Button("Quit")) {
		exit_glut();
	}
	ImGui::End();

	/* End with this when you want to render GUI */
	ImGui::Render();
}

void exit_glut() {
	/* Delete Texture on GPU */
	glDeleteTextures(1, &texture_id);
	/* Delete OpenGL program */
	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);
	/* Shut down the gui */
	ImGui_ImplGLUT_Shutdown();
	/* Delete window (freeglut) */
	glutDestroyWindow(window);
	exit(EXIT_SUCCESS);
}

void create_glut_window() {
	//glutInitContextVersion(4, 5);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE
#if _DEBUG		
		| GLUT_DEBUG
#endif
	);
	glutSetOption(
		GLUT_ACTION_ON_WINDOW_CLOSE,
		GLUT_ACTION_GLUTMAINLOOP_RETURNS
	);
	//Set number of samples per pixel
	glutSetOption(GLUT_MULTISAMPLE, 8);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	//glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	window = glutCreateWindow("Hello world OpenGL");
}

void init_program() {
	/* Initialize global variables for program control */
	nTriangles = 1;
	rotate = false;
	/* Then, create primitives (load them from mesh) */
	create_primitives();
	load_texture("../models/AmagoTexture.png");
	seconds_elapsed = 0.0f;
	angle = 0.0f;
}

void init_OpenGL() {
	using std::cout;
	using std::cerr;
	using std::endl;
	/************************************************************************/
	/*                    Init OpenGL context   info                        */
	/************************************************************************/
	glewExperimental = true;
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		cerr << "Error: " << glewGetErrorString(err) << endl;
	}

#if _DEBUG
	if (glDebugMessageCallback) {
		cout << "Register OpenGL debug callback " << endl;
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(openglCallbackFunction, nullptr);
		GLuint unusedIds = 0;
		glDebugMessageControl(GL_DONT_CARE,
			GL_DONT_CARE,
			GL_DONT_CARE,
			0,
			&unusedIds,
			true);
	}
	else
		cout << "glDebugMessageCallback not available" << endl;
#endif

	cout << "Hardware specification: " << endl;
	cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Software specification: " << endl;
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;
	cout << "Using OpenGL " << glGetString(GL_VERSION) << endl;
	cout << "Using GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	int ver = glutGet(GLUT_VERSION);
	cout << "Using freeglut version: " << ver / 10000 << "." << (ver / 100) % 100 << "." << ver % 100 << endl;

	/************************************************************************/
	/*                   OpenGL program creation                            */
	/************************************************************************/
	using std::string;

	/* In a normal program the shader should be in separate text files
	I put them here to avoid another layer of complexity */
	string vertex_shader_src =
		"#version 130\n"
		"in vec3 Position;\n"
		"in vec3 Normal;\n"
		"in vec2 TextCoord;\n"
		"\n"
		"uniform mat4 PVM;\n"
		"\n"
		"out vec3 fNormal;\n"
		"out vec2 fTextCoord;\n"
		"\n"
		"void main(void) {\n"
		"\tgl_Position = PVM * vec4(Position, 1.0f);\n"
		"\tfNormal = Normal;\n"
		"\tfTextCoord = TextCoord;\n"
		"}\n";

	string fragment_shader_src =
		"#version 130\n"
		"\n"
		"in vec3 fNormal;\n"
		"in vec2 fTextCoord;\n"
		"\n"
		"uniform sampler2D texture_image;\n"
		"\n"
		"out vec4 fragcolor;\n"
		"\n"
		"void main(void) {\n"
		"\tfragcolor = vec4(texture(texture_image, fTextCoord).rgb, 1.0);\n"
		"}\n";

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	const char* start = &vertex_shader_src[0];
	glShaderSource(vertex_shader, 1, &start, nullptr);

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	start = &fragment_shader_src[0];
	glShaderSource(fragment_shader, 1, &start, nullptr);

	int status;
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		cerr << "Vertex shader was not compiled!!" << endl;
	}
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		cerr << "Fragment shader was not compiled!!" << endl;
	}
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		cerr << "OpenGL program was not linked!!" << endl;
	}

	/************************************************************************/
	/* Allocating variables for shaders                                     */
	/************************************************************************/

	u_PVM_location = glGetUniformLocation(program, "PVM");
	u_texture_location = glGetUniformLocation(program, "texture_image");

	a_position_loc = glGetAttribLocation(program, "Position");
	a_normal_loc = glGetAttribLocation(program, "Normal");
	a_texture_loc = glGetAttribLocation(program, "TextCoord");

	//Activate anti-alias
	glEnable(GL_MULTISAMPLE);

	//Initialize some basic rendering state
	glEnable(GL_DEPTH_TEST);
	//Dark gray background color
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

}

void create_primitives() {
	using std::cout;
	using std::cerr;
	using std::endl;
	nTriangles = 0;
	//For loading the obj
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	std::string filename = "../models/Amago.obj";
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str());
	if (!err.empty()) {
		cerr << "Error at loading obj file:" << filename << endl;
		cerr << err.c_str();
	}

	if (!ret) {
		return;
	}

	bool hasNormals = attrib.normals.size() > 0;
	bool hasTexture = attrib.texcoords.size() > 0;
	vertices.clear();
	vertices = std::vector<Vertex>(attrib.vertices.size() / 3);
	indices.clear();

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces (polygon, hopefully triangle =) )
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			int fv = shapes[s].mesh.num_face_vertices[f];
			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				vertices[idx.vertex_index].position.x = attrib.vertices[3 * idx.vertex_index + 0];
				vertices[idx.vertex_index].position.y = attrib.vertices[3 * idx.vertex_index + 1];
				vertices[idx.vertex_index].position.z = attrib.vertices[3 * idx.vertex_index + 2];

				if (hasNormals) {
					vertices[idx.vertex_index].normal.x = attrib.normals[3 * idx.normal_index + 0];
					vertices[idx.vertex_index].normal.y = attrib.normals[3 * idx.normal_index + 1];
					vertices[idx.vertex_index].normal.z = attrib.normals[3 * idx.normal_index + 2];
				}

				if (hasTexture) {
					vertices[idx.vertex_index].textCoord.s = attrib.texcoords[2 * idx.texcoord_index + 0];
					vertices[idx.vertex_index].textCoord.t = attrib.texcoords[2 * idx.texcoord_index + 1];
				}

				indices.push_back(idx.vertex_index);
			}
			index_offset += fv;
		}
	}
	nTriangles = int(indices.size() / 3);


	//Create the buffers
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &indexBuffer);

	//Send data to GPU
	//First send the vertices
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//Now, the indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/* Load texture from image file usng the FreeImage library. I always load in the same format*/

bool load_texture(const std::string& file_name) {
	unsigned char* data = nullptr; //Temporal CPU buffer to hold the image pixels

	using std::cout;
	using std::cerr;
	using std::endl;

	FIBITMAP* tempImg = FreeImage_Load(FreeImage_GetFileType(file_name.c_str(), 0), file_name.c_str());
	if (!tempImg) {
		//Quick test if the image exist and I can read
		cerr << "Could not load image: " << file_name << endl;
		return false;
	}
	FIBITMAP* img = FreeImage_ConvertTo32Bits(tempImg); //See if I can convert to my format
	if (!img) {
		cerr << "Image: " << file_name << " is damaged" << endl;
		return false;
	}
	// I don't need original data, I work in my own format from here
	FreeImage_Unload(tempImg);
	//Get image metadata (size)
	unsigned int width = FreeImage_GetWidth(img);
	unsigned int height = FreeImage_GetHeight(img);
	GLuint scanW = FreeImage_GetPitch(img);
	//Allocate CPU memory to hold my pixels
	data = new GLubyte[height * scanW];
	//Load image in my own buffer.
	FreeImage_ConvertToRawBits(data, img, scanW, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, FALSE);
	FreeImage_Unload(img); //I don't need FreeImage (or his datastructures) anymore


						   /*OpenGL texture creation */
	texture_id = 0;
	//Ask for a free texture id
	glGenTextures(1, &texture_id);
	//Pass my data to GPU
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	//I don't need my CPU buffer anymore, texture is on GPU now
	delete data;
	//I reach here texture is loaded on GPU and ready to use
	return true;
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
}

void display() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);

	/************************************************************************/
	/* Calculate  Model View Projection Matrices                            */
	/************************************************************************/
	//Identity matrix
	glm::mat4 I(1.0f);
	//Model
	glm::mat4 M = rotate ? glm::rotate(I, TAU / 10.0f * seconds_elapsed, glm::vec3(0.0f, 1.0f, 0.0f)) : I;
	//View
	glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 camera_position = glm::vec3(0.0f, 0.0f, 1.5f);
	glm::vec3 camera_center = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::mat4 V = glm::lookAt(camera_position, camera_center, camera_up);
	//Projection
	GLfloat aspect = float(glutGet(GLUT_WINDOW_WIDTH)) / float(glutGet(GLUT_WINDOW_HEIGHT));
	GLfloat fovy = PI / 8.0f;
	GLfloat zNear = 0.1f;
	GLfloat zFar = 3.0f;
	glm::mat4 P = glm::perspective(fovy, aspect, zNear, zFar);

	/************************************************************************/
	/* Send uniform values to shader                                        */
	/************************************************************************/
	if (u_PVM_location != -1) {
		glUniformMatrix4fv(u_PVM_location, 1, GL_FALSE, glm::value_ptr(P * V * M));
	}
	//Set active texture and bind
	glActiveTexture(GL_TEXTURE0); //Active texture unit 0
	glBindTexture(GL_TEXTURE_2D, texture_id); //The next binded texture will be refered with the active texture unit
	if (u_texture_location != -1) {
		glUniform1i(u_texture_location, 0); // we bound our texture to texture unit 0
	}

	/************************************************************************/
	/* Bind buffer object and their corresponding attributes                */
	/************************************************************************/
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	if (a_position_loc != -1) {
		glEnableVertexAttribArray(a_position_loc);
		glVertexAttribPointer(a_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSET_OF(Vertex, position));
	}
	if (a_texture_loc != -1) {
		glEnableVertexAttribArray(a_texture_loc);
		glVertexAttribPointer(a_texture_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSET_OF(Vertex, textCoord));
	}
	if (a_normal_loc != -1) {
		glEnableVertexAttribArray(a_normal_loc);
		glVertexAttribPointer(a_normal_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSET_OF(Vertex, normal));
	}
	//Bind the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	/* Draw */
	glDrawElements(GL_TRIANGLES, 3 * nTriangles, GL_UNSIGNED_INT, BUFFER_OFFSET(0 * sizeof(unsigned int)));

	/* Unbind and clean */
	if (a_position_loc != -1) {
		glDisableVertexAttribArray(a_position_loc);
	}
	if (a_texture_loc != -1) {
		glDisableVertexAttribArray(a_texture_loc);
	}
	if (a_normal_loc != -1) {
		glDisableVertexAttribArray(a_normal_loc);
	}
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
		rotate = !rotate;
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

void APIENTRY openglCallbackFunction(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam) {
	using namespace std;

	cout << "---------------------opengl-callback-start------------" << endl;
	cout << "message: " << message << endl;
	cout << "type: ";
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		cout << "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		cout << "DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		cout << "UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		cout << "PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		cout << "PERFORMANCE";
		break;
	case GL_DEBUG_TYPE_OTHER:
		cout << "OTHER";
		break;
	}
	cout << endl;

	cout << "id: " << id << endl;
	cout << "severity: ";
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:
		cout << "LOW";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		cout << "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		cout << "HIGH";
		break;
	}
	cout << endl;
	cout << "---------------------opengl-callback-end--------------" << endl;
}
