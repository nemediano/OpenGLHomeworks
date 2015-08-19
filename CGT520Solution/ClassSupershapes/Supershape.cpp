#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>

//This is a bug on GLM in 32 bits I hope they solve it soon
#define GLM_MESSAGES
#define GLM_FORCE_PURE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include <iostream>
#include <cstdlib>

#include "OpenGLProgram.h"

using namespace std;
using namespace shaders;

static int win = 0;

// Define a helpful macro for handling offsets into buffer objects
#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

//opengl program object
OpenGLProgram* program_ptr = nullptr;

//shader uniform locations
int M_loc = -1;
int PV_loc = -1;
int Time_loc = -1;
int Color_loc = -1;

//shader attrib locations
int pos_loc = -1;
int tex_coord_loc = -1;
int normal_loc = -1;

bool pauseRotation = false;
float rev_per_min = 0.0f;
int mode = 0;

GLuint VBO1;
GLuint IndexBuffer;
const int n = 250;


float r(float phi, float a, float b, float m, float n1, float n2, float n3) {
   return powf( powf(fabs(cos(m * phi / 4.0f) / a), n2) + powf(fabs(sin(m * phi / 4.0f) / b), n3), -1.0f / n1); 
}

float a = 1.01f;
float b = 1.0f;
float m = 9.0f;

float n1 = 0.2f;
float n2 = 1.7f;
float n3 = 1.9f;


glm::vec3 Surf(int i, int j) {
	const float PI = 3.141592f;
	float theta = j * 2.0f * PI / (n - 1) - PI;
	float r1 = r(theta, a, b, m, n1, n2, n3);
   
	float phi = i * PI / (n - 1) - PI / 2.0f;
	float r2 = r(phi, a, b, m, n1, n2, n3);
   
	float x = r1 * r2 * cos(theta) * cos(phi);
	float y = r1 * r2 * sin(theta) * cos(phi);
	float z = r2 * sin(phi);

	return glm::vec3(x, y, z);
}

glm::vec3 Normal(int i, int j) {
	glm::vec3 dFdu = Surf(i + 1, j) - Surf(i - 1, j);
	glm::vec3 dFdv = Surf(i, j + 1) - Surf(i, j - 1);
	glm::vec3 N = -1.0f * glm::cross(dFdu, dFdv);
	return glm::normalize(N);
}

glm::vec2 TexCoord(int i, int j) {
   return glm::vec2((float)i / (n - 1), (float) j / (n - 1));
}

void BufferIndexedVertsUsingMapBuffer() {
	//Buffer vertices
	int nFloats = n * n * 3 + n * n * 3 + n * n * 2;
	int datasize = nFloats * sizeof(float);
	glGenBuffers(1, &VBO1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, datasize, 0, GL_STATIC_DRAW);

	float* data = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	int index = 0;
	
	for(int i = 0; i < n; ++i) {
		for(int j = 0; j < n; ++j) {
			glm::vec3 p = Surf(i, j);
			data[index++] = p.x;
			data[index++] = p.y;
			data[index++] = p.z;

			glm::vec3 n = Normal(i, j);
			data[index++] = n.x;
			data[index++] = n.y;
			data[index++] = n.z;

			glm::vec2 t = TexCoord(i,j);
			data[index++] = t.x;
			data[index++] = t.y;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Buffer indices
	glGenBuffers(1, &IndexBuffer);
	int nIndices = (n - 1) * (n - 1) * 6;
	int indexsize = nIndices * sizeof(unsigned short);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexsize, 0, GL_STATIC_DRAW);

	unsigned short* indices = (unsigned short*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	index = 0;
	for(int i = 0; i < (n - 1); ++i) {
		for(int j = 0; j < (n - 1); ++j) {
			indices[index++] = n * j + i;
			indices[index++] = n * (j + 1) + i;
			indices[index++] = n * j + (i + 1);

			indices[index++] = n * (j + 1) + i;
			indices[index++] = n * j + (i + 1);
			indices[index++] = n * (j + 1) + (i + 1);
			
		}
	}
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	gl_error("At the creation of the VBOS");
}

void DrawIndexedVerts() {
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
	glEnableVertexAttribArray(pos_loc);
	glEnableVertexAttribArray(normal_loc);
	glEnableVertexAttribArray(tex_coord_loc);
	glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 8 * sizeof(float), BUFFER_OFFSET(0));
	glVertexAttribPointer(normal_loc, 3, GL_FLOAT, false, 8 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
	glVertexAttribPointer(tex_coord_loc, 3, GL_FLOAT, false, 8 * sizeof(float), BUFFER_OFFSET(6 * sizeof(float)));
	glDrawElements(GL_TRIANGLES, (n - 1) * (n - 1) * 6, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
	glDisableVertexAttribArray(pos_loc);
	glDisableVertexAttribArray(normal_loc);
	glDisableVertexAttribArray(tex_coord_loc);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
}


void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 I(1.0f);
	glm::mat4 M = glm::rotate(I, rev_per_min, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 1.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 P = glm::perspective(75.0f * 3.0f, 1.0f, 0.1f, 100.0f);

	program_ptr->use_program();

	if(M_loc != -1)	{
		glUniformMatrix4fv(M_loc, 1, GL_FALSE, glm::value_ptr(M));
	}

	if(PV_loc != -1) {
		glUniformMatrix4fv(PV_loc, 1, GL_FALSE, glm::value_ptr(P * V));
	}

	if (Color_loc != -1) {
		glUniform4f(Color_loc, 1.0f, 0.8f, 0.0f, 1.0f);
	}

	DrawIndexedVerts();
  
	glutSwapBuffers();
	gl_error("At display");
}

void load_shader() {
	if(program_ptr != nullptr) {
		delete program_ptr;
	}

	//create and load shaders
	program_ptr = new OpenGLProgram("vshader.glsl", "fshader.glsl");
	if (!program_ptr->is_ok()) {
		cerr << "At shader compilation" << endl;
		gl_error();
		exit(EXIT_FAILURE);
	}
	program_ptr->use_program();

	pos_loc = program_ptr->get_attrib_location("pos_attrib");
	tex_coord_loc = program_ptr->get_attrib_location("tex_coord_attrib");
	normal_loc = program_ptr->get_attrib_location("normal_attrib");

	M_loc = program_ptr->get_uniform_location("M");
	PV_loc = program_ptr->get_uniform_location("PV");
	Time_loc = program_ptr->get_uniform_location("time");
	Color_loc = program_ptr->get_uniform_location("uColor");

	gl_error("At shader creation");
}


void init_OpenGL() {
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
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

	glClearColor(0.0f, 0.2f, 0.4f, 0.0f);
	glEnable(GL_DEPTH_TEST);

	BufferIndexedVertsUsingMapBuffer();
	load_shader();

	gl_error("At OpenGL initialization");
}

void idle() {
	static int last_time = 0;
	static float seconds = 0.0f;

	int time = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = time - last_time;
	last_time = time;
	float delta_seconds = 0.001f * elapsed;
	
	if(pauseRotation == false) {
		seconds += delta_seconds;
		rev_per_min = 3.0f * seconds;
	}

	//set shader time uniform variable
	program_ptr->use_program();

	if(Time_loc != -1) {
		glUniform1f(Time_loc, seconds);
	}

	glutPostRedisplay();
}

void exit_glut() {
	glutDestroyWindow(win);
	exit(EXIT_SUCCESS);
}

void keyboard(unsigned char key, int x, int y) {
	if(key == 27) { //press ESC to exit
		exit_glut();
	}  

	if(key == 'r' || key == 'R') {
		load_shader();
	}

	if (key == 'q' || key == 'Q') {
		m += 0.5f;
		std::wcout << "m = " << m << L"\n";
		std::wcout << "a = " << a << L"\n";
		std::wcout << "b = " << b << L"\n";
		BufferIndexedVertsUsingMapBuffer();
	}

	if (key == 'a' || key == 'A') {
		m -= 0.5f;
		std::wcout << "m = " << m << L"\n";
		std::wcout << "a = " << a << L"\n";
		std::wcout << "b = " << b << L"\n";
		BufferIndexedVertsUsingMapBuffer();
	}
	
	if (key == 'w' || key == 'W') {
		a += 0.05f;
		std::wcout << "m = " << m << L"\n";
		std::wcout << "a = " << a << L"\n";
		std::wcout << "b = " << b << L"\n";
		BufferIndexedVertsUsingMapBuffer();
	}
	
	if (key == 's' || key == 'S') {
		a -= 0.05f;
		std::wcout << "m = " << m << L"\n";
		std::wcout << "a = " << a << L"\n";
		std::wcout << "b = " << b << L"\n";
		BufferIndexedVertsUsingMapBuffer();
	}

	if (key == 'e' || key == 'E') {
		b += 0.01f;
		std::wcout << "m = " << m << L"\n";
		std::wcout << "a = " << a << L"\n";
		std::wcout << "b = " << b << L"\n";
		BufferIndexedVertsUsingMapBuffer();
	}

	if (key == 'd' || key == 'D') {
		b -= 0.01f;
		std::wcout << "m = " << m << L"\n";
		std::wcout << "a = " << a << L"\n";
		std::wcout << "b = " << b << L"\n";
		BufferIndexedVertsUsingMapBuffer();
	}
}


void special(int key, int x, int y) {

   if(key == GLUT_KEY_F1) {
	   pauseRotation = !pauseRotation;
   }

}

void CreateGlutWindow() {
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition (5, 5);
	glutInitWindowSize (640, 640);
	win = glutCreateWindow ("Supershape");
}

void CreateGlutCallbacks() {
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv); 
	CreateGlutWindow();
	CreateGlutCallbacks();
	init_OpenGL();

	glutMainLoop();

	exit_glut();
	return EXIT_SUCCESS;
}
