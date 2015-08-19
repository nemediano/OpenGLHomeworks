#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>

//This is a bug on GLM in 32 bits I hope they solve it soon
#define GLM_FORCE_PURE
#define GLM_MESSAGES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include <IL/il.h>
#include <IL/ilu.h>

#include <iostream>
#include <cstdlib>
#include <random>

#include "TextureHandler.h"
#include "OpenGLProgram.h"

// Define a helpful macro for handling offsets into buffer objects
#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

using namespace shaders;
using namespace texture;
using namespace std;
static int win = 0;

//Opengl program object
OpenGLProgram* program_ptr = nullptr;
TextureHandler* particle_texture_ptr = nullptr;

//shader uniform locations
int M_loc = -1;
int PV_loc = -1;
int Time_loc = -1;
int tex_loc = -1;

//shader attrib locations
int pos_loc = -1;
int color_loc = -1;
int size_loc = -1;

bool pauseRotation = true;
float rev_per_min = 0.1f;
float time = 0;
int mode = 0;

GLuint TextureID = 0;
GLuint VBO1;
GLuint IndexBuffer;
const int n = 1200;

struct particle_attribs
{
   glm::vec3 pos;
   glm::vec4 color;
   float size;
};


struct other_particle_state
{
   glm::vec3 vel;
   float age;
};

particle_attribs particle_vbo[n];
other_particle_state particle_cpu[n];


void BufferUnindexedVertsUsingBufferData() {
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, n * sizeof(particle_attribs), particle_vbo, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	gl_error("At buffer using");
}

std::random_device rd;
std::mt19937 rng(rd());
std::normal_distribution<float> normal_dist(0.0f, 1.0f);
std::uniform_real_distribution<float> uniform_dist(0.0f, 1.0f);

void InitParticle(int i) {

	particle_vbo[i].pos = glm::vec3(uniform_dist(rng) * 0.1 - 0.1/2.0, -1.5, 0.0);
	particle_vbo[i].color = glm::vec4(1.0f, 0.0f, 0.0f, 0.35f * uniform_dist(rng));
	particle_vbo[i].size = normal_dist(rng) * 0.25 + 3.0;

	particle_cpu[i].vel = glm::vec3(normal_dist(rng) * 0.02, 0.5, normal_dist(rng) * 0.02);
	particle_cpu[i].age = 5.0f * uniform_dist(rng);
}
 
void init_particles() {
   for(int i = 0; i < n; ++i) {
	   InitParticle(i);		
	}
   BufferUnindexedVertsUsingBufferData();
}

void UpdateParticles(float dt) {
	const float max_age = 9.0f;
	for(int i = 0; i < n; ++i) {
			particle_vbo[i].pos += dt * particle_cpu[i].vel;
			particle_vbo[i].color += dt * glm::vec4(0.0f, 0.1f, 0.02f, -0.01f);
			particle_vbo[i].size += 2.0f * dt;

			particle_cpu[i].vel = 0.999999f * particle_cpu[i].vel;
			particle_cpu[i].age += dt;

			if(particle_cpu[i].age > max_age) {
				InitParticle(i);
			}
	}
	BufferUnindexedVertsUsingBufferData();
}

void DrawUnindexedVerts() {
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);

	if (pos_loc != -1) {
		glEnableVertexAttribArray(pos_loc);
	}
	if (color_loc != -1) {
		glEnableVertexAttribArray(color_loc);
	}
	if (size_loc != -1) {
		glEnableVertexAttribArray(size_loc);
	}
	
	assert(sizeof(particle_attribs) == sizeof(float) * 8);
	int stride = sizeof(particle_attribs);
	
	if (pos_loc != -1) {
		glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, stride, BUFFER_OFFSET(0));
	}
	if (color_loc != -1) {
		glVertexAttribPointer(color_loc, 4, GL_FLOAT, false, stride, BUFFER_OFFSET(3 * sizeof(float)));
	}
	if (size_loc != -1) {
		glVertexAttribPointer(size_loc, 1, GL_FLOAT, false, stride, BUFFER_OFFSET(7 * sizeof(float)));
	}
	glDrawArrays(GL_POINTS, 0, n);

	if (pos_loc != -1) {
		glDisableVertexAttribArray(pos_loc);
	}
	if (color_loc != -1) {
		glDisableVertexAttribArray(color_loc);
	}
	if (size_loc != -1) {
		glDisableVertexAttribArray(size_loc);
	}


	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 I(1.0f);
	glm::mat4 M = glm::rotate(I, rev_per_min, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 P = glm::perspective(120.0f, 1.0f, 0.1f, 100.0f);

	program_ptr->use_program();

	glUniformMatrix4fv(M_loc, 1, GL_FALSE, glm::value_ptr(M));
	glUniformMatrix4fv(PV_loc, 1, GL_FALSE, glm::value_ptr(P * V));

	
	
	glDepthMask(GL_FALSE); //we're not going to depth sort these particles

	if (tex_loc != -1) {
		glActiveTexture(GL_TEXTURE0);
		particle_texture_ptr->bind();
	}
	gl_error("After texturing but before drawing texturing");
	DrawUnindexedVerts();
	gl_error("After drawing");
	glDepthMask(GL_TRUE);

	if (tex_loc != -1) {
		particle_texture_ptr->unbind();
	}
	glutSwapBuffers();
	//gl_error("At display");
}

void load_shader() {
	if(program_ptr != nullptr) {
		delete program_ptr;
	}

	//create and load shaders
	program_ptr = new OpenGLProgram("vshader.glsl", "fshader.glsl");
	if (!program_ptr->is_ok()) {
		gl_error("At the shader compilation");
		exit(EXIT_FAILURE);
	}
	program_ptr->use_program();

	pos_loc = program_ptr->get_attrib_location("pos_attrib");
	color_loc = program_ptr->get_attrib_location("color_attrib");
	size_loc = program_ptr->get_attrib_location("size_attrib");

	M_loc = program_ptr->get_uniform_location("M");
	PV_loc = program_ptr->get_uniform_location("PV");
	Time_loc = program_ptr->get_uniform_location("time");

	tex_loc = program_ptr->get_uniform_location("colortex");

	if(tex_loc != -1) {
		glUniform1i(tex_loc, 0); // we will bind our texture to texture unit 0
	}

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
	
	glClearColor(0.15f, 0.15f, 0.15f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POINT_SPRITE); // allows textured points
	glEnable(GL_PROGRAM_POINT_SIZE); //allows us to set point size in vertex shader

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	
	glGenBuffers(1, &VBO1);
	init_particles();
	load_shader();

	particle_texture_ptr = new TextureHandler();
	particle_texture_ptr->load_texture(L"img/particle.png");

	gl_error("At OpenGL initialization");
}

void idle() {
	static int last_time = 0;
	static float seconds = 0.0f;

	int time = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = time - last_time;
	last_time = time;
	float delta_seconds = 0.001f * elapsed;

	UpdateParticles(delta_seconds);
	
	if(!pauseRotation) {
		seconds += delta_seconds;
		rev_per_min = 0.25f * seconds;
	}

	//set shader time uniform variable
	program_ptr->use_program();

	if(Time_loc != -1) {
		glUniform1f(Time_loc, seconds);
	}

	glutPostRedisplay();
}

void exit_glut() {
	delete program_ptr;
	delete particle_texture_ptr;

	glutDestroyWindow(win);
	exit(EXIT_SUCCESS);
}

void keyboard(unsigned char key, int x, int y) {
   if(key == 27) {//press ESC to exit
	   exit_glut();
   }  

   if(key == 'r' || key == 'R') {
	   load_shader();
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
	win = glutCreateWindow ("Particles");
}

void CreateGlutCallbacks() {
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
}

int main (int argc, char* argv[]) {
	glutInit(&argc, argv); 

	//Init DevIl for texture loading
	ilInit();
	iluInit();

	CreateGlutWindow();
	CreateGlutCallbacks();
	init_OpenGL();

	glutMainLoop();

	exit_glut();
	return EXIT_SUCCESS;
}

