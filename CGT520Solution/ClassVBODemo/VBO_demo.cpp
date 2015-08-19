#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>

//This is a bug on GLM in 32 bits I hope they solve it soon
#define GLM_FORCE_PURE
#define GLM_MESSAGES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

// Define a helpful macro for handling offsets into buffer objects
#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))


#include <iostream>
#include "OpenGLProgram.h"


using namespace std;
using namespace shaders;

static int win = 0;

OpenGLProgram* program_ptr = nullptr;

GLint uM_loc = -1;
GLint uColor_loc = -1;
GLint aPos_loc = -1;

float angle = 0.0f;

bool perspectiveProj = true;
bool rotateCam = true;
float aspectRatio = 1.0;

GLuint VBO1;
GLuint IndexBuffer;

GLuint VBO2;
const int n = 150; //number of vertices in each direction

enum groupId {GROUP_1, GROUP_2, GROUP_3, GROUP_4};

//GROUP_1 : use glBufferData and unindexed primitives
//GROUP_2 : use glBufferData and indexed primitives
//GROUP_3 : use glMapBuffer and unindexed primitives
//GROUP_4 : use glMapBuffer and indexed primitives

groupId group = GROUP_4; //TODO set your group id here

bool useIndexedPrimitives()
{
	if((group == GROUP_2) || (group == GROUP_4))
	{
		return true;
	}
	return false;
}

bool useMapBuffer()
{
	if((group == GROUP_3) || (group == GROUP_4))
	{
		return true;
	}
	return false;
}

glm::vec3 Surf(int i, int j)
{
	const float center = 0.5f*n;
	const float xy_scale = 20.0f/n;
	const float z_scale = 10.0f;

	float x = xy_scale*(i-center);
	float y = xy_scale*(j-center);

	float r = sqrt(x*x + y*y);
	float z = 1.0f;

	if(r != 0.0f)
	{
		z = sin(r)/r;
	}
	z = z*z_scale;

	return glm::vec3(x,y,z);
}


void BufferIndexedVertsUsingMapBuffer()
{
	//Buffer vertices
	int nFloats = n*n*3;
	int datasize = nFloats*sizeof(float);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, datasize, 0, GL_STATIC_DRAW);

	float* data = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	int index = 0;
	
	for(int i=0; i<n; ++i)
	{
		for(int j=0; j<n; ++j)
		{
			glm::vec3 p1 = Surf(i, j);
			data[index++] = p1.x;
			data[index++] = p1.y;
			data[index++] = p1.z;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Buffer indices
	int nIndices = (n-1)*(n-1)*6;
	int indexsize = nIndices*sizeof(unsigned short);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexsize, 0, GL_STATIC_DRAW);

	unsigned short* indices = (unsigned short*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	index = 0;
	for(int i=0; i<(n-1); ++i)
	{
		for(int j=0; j<(n-1); ++j)
		{
			indices[index++] = n*j+i;
			indices[index++] = n*(j+1)+i;
			indices[index++] = n*j+(i+1);

			indices[index++] = n*(j+1)+i;
			indices[index++] = n*j+(i+1);
			indices[index++] = n*(j+1)+(i+1);
			
		}
	}
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void BufferIndexedVertsUsingBufferData()
{
	//Buffer vertices
	int nFloats = n*n*3;
	float* data = new float[nFloats];
	int index = 0;
	
	for(int i=0; i<n; ++i)
	{
		for(int j=0; j<n; ++j)
		{
			glm::vec3 p1 = Surf(i,j);
			data[index++] = p1.x;
			data[index++] = p1.y;
			data[index++] = p1.z;
		}
	}

	int datasize = nFloats*sizeof(float);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, datasize, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete [] data;

	//Buffer indices
	int nIndices = (n-1)*(n-1)*6;
	unsigned short* indices = new unsigned short[nIndices];
	index = 0;
	for(int i=0; i<(n-1); ++i)
	{
		for(int j=0; j<(n-1); ++j)
		{
			indices[index++] = n*j+i;
			indices[index++] = n*(j+1)+i;
			indices[index++] = n*j+(i+1);

			indices[index++] = n*(j+1)+i;
			indices[index++] = n*j+(i+1);
			indices[index++] = n*(j+1)+(i+1);
			
		}
	}

	int indexsize = nIndices*sizeof(unsigned short);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexsize, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void BufferUnindexedVertsUsingBufferData()
{
	int nFloats = (n-1)*(n-1)*6*3;
	float* data = new float[nFloats];
	int index = 0;
	
	for(int i=0; i<(n-1); ++i)
	{
		for(int j=0; j<(n-1); ++j)
		{
			//triangle 1
			glm::vec3 p1 = Surf(i,j);
			data[index++] = p1.x;
			data[index++] = p1.y;
			data[index++] = p1.z;

			glm::vec3 p2 = Surf(i,j+1);
			data[index++] = p2.x;
			data[index++] = p2.y;
			data[index++] = p2.z;

			glm::vec3 p3 = Surf(i+1,j);
			data[index++] = p3.x;
			data[index++] = p3.y;
			data[index++] = p3.z;

			//triangle 2
			data[index++] = p2.x;
			data[index++] = p2.y;
			data[index++] = p2.z;

			data[index++] = p3.x;
			data[index++] = p3.y;
			data[index++] = p3.z;

			glm::vec3 p4 = Surf(i+1,j+1);
			data[index++] = p4.x;
			data[index++] = p4.y;
			data[index++] = p4.z;
		}
	}

	int datasize = nFloats*sizeof(float);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, datasize, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete [] data;
}

void BufferUnindexedVertsUsingMapBuffer()
{
	int nFloats = (n-1)*(n-1)*6*3;
	int datasize = nFloats*sizeof(float);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, datasize, 0, GL_STATIC_DRAW);

	float* data = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	int index = 0;

	for(int i=0; i<(n-1); ++i)
	{
		for(int j=0; j<(n-1); ++j)
		{
			//triangle 1
			glm::vec3 p1 = Surf(i,j);
			data[index++] = p1.x;
			data[index++] = p1.y;
			data[index++] = p1.z;

			glm::vec3 p2 = Surf(i,j+1);
			data[index++] = p2.x;
			data[index++] = p2.y;
			data[index++] = p2.z;

			glm::vec3 p3 = Surf(i+1,j);
			data[index++] = p3.x;
			data[index++] = p3.y;
			data[index++] = p3.z;

			//triangle 2
			data[index++] = p2.x;
			data[index++] = p2.y;
			data[index++] = p2.z;

			data[index++] = p3.x;
			data[index++] = p3.y;
			data[index++] = p3.z;

			glm::vec3 p4 = Surf(i+1,j+1);
			data[index++] = p4.x;
			data[index++] = p4.y;
			data[index++] = p4.z;
		}
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void DrawUnindexedVerts()
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glEnableVertexAttribArray(aPos_loc);
	glVertexAttribPointer(aPos_loc, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(0));
	glDrawArrays(GL_TRIANGLES, 0, (n-1)*(n-1)*6);
	glDisableVertexAttribArray(aPos_loc);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
}

void DrawIndexedVerts()
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
	glEnableVertexAttribArray(aPos_loc);
	glVertexAttribPointer(aPos_loc, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(0));
	glDrawElements(GL_TRIANGLES, (n-1)*(n-1)*6, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
	glDisableVertexAttribArray(aPos_loc);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
}

void BufferUnindexedVerts()
{
	if(useMapBuffer())
	{
		BufferUnindexedVertsUsingMapBuffer();
	}
	else
	{
		BufferUnindexedVertsUsingBufferData();
	}
}

void BufferIndexedVerts()
{
	if(useMapBuffer())
	{
		BufferIndexedVertsUsingMapBuffer();
	}
	else
	{
		BufferIndexedVertsUsingBufferData();
	}
}


void exitGlut()
{
   glutDestroyWindow(win);
   exit(0);
}

void initOpenGL()
{
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

	program_ptr = new OpenGLProgram("vbodemo_vs.glsl", "vbodemo_fs.glsl");

	uM_loc = program_ptr->get_uniform_location("uM");
	uColor_loc = program_ptr->get_uniform_location("uColor");
	aPos_loc = program_ptr->get_attrib_location("aPosition");

	//initialize some basic rendering state
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

	glGenBuffers(1, &VBO1);
	glGenBuffers(1, &IndexBuffer);
	glGenBuffers(1, &VBO2);

	if(useIndexedPrimitives())
	{
		BufferIndexedVerts();
	}
	else
	{
		BufferUnindexedVerts();
	}
		
	glEnable(GL_DEPTH_TEST);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	program_ptr->use_program();
	//set color to draw with
	glUniform4f(uColor_loc, 1.0f, 0.0f, 0.0f, 1.0f);

	
	glm::mat4 I(1.0f); //Identity matrix
	glm::mat4 V = glm::lookAt(glm::vec3(1.5f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 P = I;
	glm::mat4 Rcam = I;

	if(perspectiveProj == true)
	{
		P = glm::perspective(90.0f, aspectRatio, 0.1f, 2.5f);
	}
	else
	{
		P = glm::ortho(-aspectRatio, +aspectRatio, -1.0f, +1.0f, -1.0f, +1.0f);
	}

	if(rotateCam == true)
	{
		Rcam = glm::rotate(I, 0.5f * angle, glm::vec3(0.0f, 0.0f, 1.0));
	}

	const float step = 2.0f;
	glm::mat4 S = glm::scale(I, glm::vec3(step * 0.05f, step * 0.05f, step * 0.05f));

	glFinish(); //blocks until previous gl calls have completed
	GLint64 start;
	glGetInteger64v(GL_TIMESTAMP, &start);

	//draw meshes
	for(float x = -1.0; x<= +1.0; x+= step)
	{
		for(float y = -1.0; y<= +1.0; y+= step)
		{
			glm::mat4 M = glm::translate(I, glm::vec3(x, y, 0.0f)) * S;
			glUniform4f(uColor_loc, (x + 1.0f) / 2.0f, (y + 1.0f) / 2.0f, 0.5f * sin(0.01f * angle) + 0.5f, 1.0f);
			glUniformMatrix4fv(uM_loc, 1, GL_FALSE, glm::value_ptr(P * V * Rcam * M));

			if(useIndexedPrimitives())
			{
				DrawIndexedVerts();
			}
			else
			{
				DrawUnindexedVerts();
			}
		}
	}

	glFinish(); //blocks until previous gl calls have completed
	GLint64 finish;
	glGetInteger64v(GL_TIMESTAMP, &finish);

	const int nmeshes = static_cast<int>((2.0f/step)*(2.0/step));
	const int ntris = nmeshes*(n-1)*(n-1)*2;
	cout<<"Draw calls took " << (finish-start)/1000000.0 << "ms, " <<"drew "<<ntris<<" triangles "<<endl;
	

	gl_error("At display function");
	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
   if(key == 27) //press ESC to exit
   {
      exitGlut();
   }  
}

void special(int key, int x, int y)
{
   if(key == GLUT_KEY_F3)
   {
      rotateCam = !rotateCam;
   }
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	aspectRatio = static_cast<float>(w)/static_cast<float>(h);
}


void idle()
{  
   //timers for time-based animation
   static int last_time = 0;
   int time = glutGet(GLUT_ELAPSED_TIME);
   int elapsed = time-last_time;
   float delta_seconds = 0.001f * elapsed;
   last_time = time;

   angle = angle + 180.0f * delta_seconds;

   gl_error("At idle function"); //check for errors and print error strings
   glutPostRedisplay();
}

void createGlutWindow()
{
	glutInitDisplayMode (GLUT_RGB|GLUT_DEPTH|GLUT_DOUBLE);
	glutInitWindowSize (512, 512);
	win = glutCreateWindow ("VBO demo");
}

void createGlutCallbacks()
{
	glutDisplayFunc		(display);
	glutIdleFunc		(idle);
	glutKeyboardFunc	(keyboard);
	glutSpecialFunc		(special);
	glutReshapeFunc		(reshape);
}

int main (int argc, char **argv)
{
	glutInit(&argc, argv); 
	createGlutWindow();
	createGlutCallbacks();
	initOpenGL();
	glutMainLoop();
	glutDestroyWindow(win);
	return 0;
}