#include <GL/glew.h>
#include <GL/freeglut.h>

#define GLM_FORCE_PURE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Two math constants (New glm uses radians as default)
const float TAU = 6.28318f;
const float PI = 3.14159f;

#include <iostream>
#include "InitShader.h"
#include "LoadMesh.h"

// JisunKang: define for Window management.
#define WINDOW_WIDTH	512
#define WINDOW_HEIGHT	512
#define WINDOW_TITLE	"Dynamic Cubemapping Demo"

static int win = 0;
float theta = 0.0f;     //rotation angle for rotating objects

static const std::string vertex_shader("cubemap_vs.glsl");
static const std::string fragment_shader("cubemap_fs.glsl");
GLuint shader_program = -1;

static const std::string cow_mesh_name = "cow.ply";
MeshData cow_mesh_data;

static const std::string fish_mesh_name = "Amago0.obj";
MeshData fish_mesh_data;

static const std::string sphere_mesh_name = "sphere.obj";
MeshData sphere_mesh_data;

int cubemapsize = 128;
GLuint CubemapID = 0;   //IDs for cube map texture,  
GLuint FBOID = 0;       //framebuffer object,
GLuint RBOID = 0;       //and renderbuffer (for depth buffering)

//Set look and up vectors for the view matrix of each cubemap face
//see table 3.19 on page 170 of GL spec
float look[6][3] = {{1.0f, 0.0f, 0.0f},     //positive X face
                    {-1.0f, 0.0f, 0.0f},    //negative X face
                    {0.0f, 1.0f, 0.0f},     //positive Y face
                    {0.0f, -1.0f, 0.0f},    //negative Y face
                    {0.0f, 0.0f, 1.0f},     //positive Z face
                    {0.0f, 0.0f, -1.0f}};   //negative Z face

float up[6][3] = {  {0.0f, -1.0f, 0.0f},
                    {0.0f, -1.0f, 0.0f},
                    {0.0f, 0.0f, 1.0f},
                    {0.0f, 0.0f, -1.0f},
                    {0.0f, -1.0f, 0.0f},
                    {0.0f, -1.0f, 0.0f}};

bool CheckFramebufferStatus();
void DrawReflectedObjects();
void RenderToCubemap();
void DrawReflectedObjects();
void DrawCubemappedObject();
void ExitGlut();

void sendUniformValue(const char* uniname, glm::mat4& val)
{
	int loc = glGetUniformLocation(shader_program, uniname);
	if (loc != -1)
	{
		glUniformMatrix4fv(loc, 1, false, glm::value_ptr(val));
	}
}

void sendUniformValue(const char* uniname, glm::vec4& v)
{
	int loc = glGetUniformLocation(shader_program, uniname);
	if (loc != -1)
	{
		glUniform4fv(loc, 1, glm::value_ptr(v));
	}
}

void sendUniformValue(const char* uniname, int val)
{
	int loc = glGetUniformLocation(shader_program, uniname);
	if (loc != -1)
	{
		glUniform1i(loc, val);
	}
}

void InitOpenGL()
{
	glewInit();

	//////////////////////////////////////////////////////
	//initialize and load shader
	GLuint new_shader = InitShader(vertex_shader.c_str(), fragment_shader.c_str());
	if (new_shader == -1)
	{
		glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
	}
	else
	{
		glClearColor(0.35f, 0.35f, 0.35f, 0.0f);
		if (shader_program != -1)
		{
			glDeleteProgram(shader_program);
		}
		shader_program = new_shader;
	}

	//////////////////////////////////////////////////////
	//build reflected and reflect objects
	//load objects - cow, fish, sphere
	cow_mesh_data = LoadMesh(cow_mesh_name);
	fish_mesh_data = LoadMesh(fish_mesh_name);
	sphere_mesh_data = LoadMesh(sphere_mesh_name);

	//////////////////////////////////////////////////////
	//create a cubemap texture object and set empty texture images
	glGenTextures(1, &CubemapID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, CubemapID);
	for (int i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, cubemapsize, cubemapsize, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	//////////////////////////////////////////////////////
	//create the depth buffer for the framebuffer object
	glGenRenderbuffers(1, &RBOID);
	glBindRenderbuffer(GL_RENDERBUFFER, RBOID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, cubemapsize, cubemapsize);

	//////////////////////////////////////////////////////
	//create the framebuffer object and attach the cubemap faces and the depth buffer
	glGenFramebuffers(1, &FBOID);
	glBindFramebuffer(GL_FRAMEBUFFER, FBOID);
	for (int i = 0; i < 6; i++)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, CubemapID, 0);
	}
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBOID);

	if (!CheckFramebufferStatus())
	{
		ExitGlut();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}

void display()
{
   //pass 1: render to cubemap
	RenderToCubemap();


   //pass 2: render scene
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.5f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); //eye, look, up
	glm::mat4 P = glm::perspective(TAU / 5.5f, 1.0f, 0.1f, 100.0f);
	sendUniformValue("P", P);
	sendUniformValue("V", V);

	DrawCubemappedObject();
	DrawReflectedObjects();

	glutSwapBuffers();
}

void DrawReflectedObjects()
{
	sendUniformValue("useCubemap", 0);

	//draw rotating cow
   sendUniformValue("color", glm::vec4(0.5f, 1.0f, 0.5f, 1.0f));
	glm::mat4 M2 = glm::rotate(theta, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, 0.7f)) * glm::scale(glm::vec3(cow_mesh_data.mScaleFactor*0.4f));
	sendUniformValue("M", M2);
	glBindVertexArray(cow_mesh_data.mVao);
	glDrawElements(GL_TRIANGLES, cow_mesh_data.mNumIndices, GL_UNSIGNED_INT, 0);

   M2 = glm::rotate(theta+PI, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::translate(glm::vec3(0.0f, 0.0f, 0.7f)) * glm::scale(glm::vec3(cow_mesh_data.mScaleFactor*0.4f));
	sendUniformValue("M", M2);
   sendUniformValue("color", glm::vec4(0.75f, 0.75f, 0.75f, 1.0f));
   glDrawElements(GL_TRIANGLES, cow_mesh_data.mNumIndices, GL_UNSIGNED_INT, 0);

	//draw rotating sphere
   sendUniformValue("color", glm::vec4(0.5f, 0.5f, 1.0f, 1.0f));
	glm::mat4 M3 = glm::rotate(theta+PI, glm::vec3(0.0f, 0.0f, 1.0f)) * glm::translate(glm::vec3(0.0f, 0.75f, 0.0f)) * glm::scale(glm::vec3(0.08f));
	sendUniformValue("M", M3);
	glBindVertexArray(sphere_mesh_data.mVao);
	glDrawElements(GL_TRIANGLES, sphere_mesh_data.mNumIndices, GL_UNSIGNED_INT, 0);


   M3 = glm::rotate(theta , glm::vec3(0.0f, 0.0f, 1.0f)) * glm::translate(glm::vec3(0.0f, 0.75f, 0.0f)) * glm::scale(glm::vec3(0.08f));
   sendUniformValue("M", M3);
   sendUniformValue("color", glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
   glDrawElements(GL_TRIANGLES, sphere_mesh_data.mNumIndices, GL_UNSIGNED_INT, 0);
	
}

void DrawCubemappedObject()
{
	sendUniformValue("useCubemap", 1);			
   sendUniformValue("color", glm::vec4(0.95f, 0.95f, 0.95f, 1.0f));
	glBindTexture(GL_TEXTURE_CUBE_MAP, CubemapID);

	// draw main fish
	glm::mat4 M = glm::scale(glm::vec3(3.0));
	sendUniformValue("M", M);
	glBindVertexArray(fish_mesh_data.mVao);
	glDrawElements(GL_TRIANGLES, fish_mesh_data.mNumIndices, GL_UNSIGNED_INT, 0);

}

void RenderToCubemap()
{
	//render to the 6 faces of the cube map
	glViewport(0, 0, cubemapsize, cubemapsize);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);		//unbind cube textures so we can write to them (remember, no read-write access)
	glBindFramebuffer(GL_FRAMEBUFFER, FBOID);

	glUseProgram(shader_program);
	glm::mat4 P = glm::perspective(TAU / 4.0f, 1.0f, 0.1f, 100.0f);
	sendUniformValue("P", P);

	//render the 6 faces
	for (int i = 0; i < 6; i++)
	{
		glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(look[i][0], look[i][1], look[i][2]), glm::vec3(up[i][0], up[i][1], up[i][2]));
		sendUniformValue("V", V);

		//set the texture image to render to
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		DrawReflectedObjects();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);		//unbind framebuffer object
	glDrawBuffer(GL_BACK);						//resume rendering to the back buffer (we are using double buffering)
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

void keyboard(unsigned char key, int x, int y)
{
	if (key == 27) //press ESC to exit
	{
		ExitGlut();
	}
}

void idle()
{
	//timers for time-based animation
	static int last_time = 0;
	int time = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = time - last_time;
	float delta_seconds = 0.001f*elapsed;
	last_time = time;

	//objects will rotate at 60 degrees per second, regardless of framerate
	theta = theta + TAU / 6.0f * delta_seconds;

	glutPostRedisplay();
}

void CreateGlutWindow()
{
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	win = glutCreateWindow(WINDOW_TITLE);

	//print system info
	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));
	printf("Extensions: %s\n", glGetString(GL_EXTENSIONS));
}

void CreateGlutCallbacks()
{
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
}

bool CheckFramebufferStatus()
{
	GLenum status;
	status = (GLenum)glCheckFramebufferStatus(GL_FRAMEBUFFER);
	switch (status) {
	case GL_FRAMEBUFFER_COMPLETE:
		return true;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		printf("Framebuffer incomplete, incomplete attachment\n");
		return false;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		printf("Unsupported framebuffer format\n");
		return false;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		printf("Framebuffer incomplete, missing attachment\n");
		return false;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		printf("Framebuffer incomplete, missing draw buffer\n");
		return false;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		printf("Framebuffer incomplete, missing read buffer\n");
		return false;
	}
	return false;
}

void ExitGlut()
{
	glutDestroyWindow(win);
	exit(0);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	CreateGlutWindow();
	CreateGlutCallbacks();
	InitOpenGL();
	glutMainLoop();
	ExitGlut();
	return 0;
}