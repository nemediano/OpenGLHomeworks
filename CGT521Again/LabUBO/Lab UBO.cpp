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
#include "LoadTexture.h"

static const std::string vertex_shader("ubo_vs.glsl");
static const std::string fragment_shader("ubo_fs.glsl");

GLuint shader_program = -1;
GLuint texture_id = -1;
GLuint uniform_buffer = -1;

// See the layout rules in the redbook. There are complicated rules for sizes and padding in uniform buffer objects.
// A good safe way to get started: make matrices mat4, make vectors vec4, and sort the uniform block members from
// largest to smallest.

struct LightingUniforms
{
   glm::mat4 M;	//modeling matrix
   glm::mat4 PV;	//camera projection * view matrix

   glm::vec4 La;	//ambient light color
   glm::vec4 Ld;	//diffuse light color
   glm::vec4 Ls;	//specular light color
   glm::vec4 ka;	//ambient material color
   glm::vec4 kd;	//diffuse material color
   glm::vec4 ks;	//specular material color
   glm::vec4 eye_w;	//world-space eye position
   glm::vec4 light_w; //world-space light position

   float shininess;
} LightingUniformData;

static const std::string mesh_name = "Amago0.obj";
static const std::string texture_name = "AmagoT.bmp";
MeshData mesh_data;

float time_sec;

// glut display callback function.
// This function gets called every time the scene gets redisplayed 
void display()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glUseProgram(shader_program);

   //Update the values that have changed.
   LightingUniformData.M = glm::rotate(PI / 36.0f * time_sec, glm::vec3(0.0f, 1.0f, 0.0f))*glm::scale(glm::vec3(mesh_data.mScaleFactor));
   LightingUniformData.eye_w = glm::vec4(0.0f, 0.0f, 2.0f, 1.0f);
   glm::mat4 V = glm::lookAt(glm::vec3(LightingUniformData.eye_w), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
   //New GLM uses radians as default
   glm::mat4 P = glm::perspective(PI / 4.0f, 1.0f, 0.1f, 100.0f);
   LightingUniformData.PV = P*V;

   glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer); //Bind the OpenGL UBO before we update the data.
   glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightingUniformData), &LightingUniformData); //Upload the new uniform values.
   glBindBuffer(GL_UNIFORM_BUFFER, 0); //unbind the ubo
   //Using the UBO saves us from making 11 glUniform* function calls

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, texture_id);
   
   int tex_loc = glGetUniformLocation(shader_program, "texColor");
   if(tex_loc != -1)
   {
      glUniform1i(tex_loc, 0); // we bound our texture to texture unit 0
   }

   glBindVertexArray(mesh_data.mVao);
	glDrawElements(GL_TRIANGLES, mesh_data.mNumIndices, GL_UNSIGNED_INT, 0);

   glutSwapBuffers();
}

void idle()
{
	glutPostRedisplay();

   const int time_ms = glutGet(GLUT_ELAPSED_TIME);
   time_sec = 0.001f*time_ms;
}

void reload_shader()
{
   GLuint new_shader = InitShader(vertex_shader.c_str(), fragment_shader.c_str());

   if(new_shader == -1) // loading failed
   {
      glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
   }
   else
   {
      glClearColor(0.35f, 0.35f, 0.35f, 0.0f);

      if(shader_program != -1)
      {
         glDeleteProgram(shader_program);
      }
      shader_program = new_shader;

      if(mesh_data.mVao != -1)
      {
         BufferIndexedVerts(mesh_data);
      }
   }
}


// glut keyboard callback function.
// This function gets called when an ASCII key is pressed
void keyboard(unsigned char key, int x, int y)
{
   std::cout << "key : " << key << ", x: " << x << ", y: " << y << std::endl;

   switch(key)
   {
      case 'r':
      case 'R':
         reload_shader();     
      break;
   }
}

void printGlInfo()
{
   std::cout << "Vendor: "       << glGetString(GL_VENDOR)                    << std::endl;
   std::cout << "Renderer: "     << glGetString(GL_RENDERER)                  << std::endl;
   std::cout << "Version: "      << glGetString(GL_VERSION)                   << std::endl;
   std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION)  << std::endl;
}

void initOpenGl()
{
   glewInit();

   glEnable(GL_DEPTH_TEST);

   glGenBuffers(1, &uniform_buffer);
   glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
   glBufferData(GL_UNIFORM_BUFFER, sizeof(LightingUniforms), NULL, GL_STREAM_DRAW); //Allocate memory for the buffer, but don't copy (since pointer is null).
   const int ubBindingPoint = 2; //This values comes from the binding value specified in the block layout.
   glBindBufferBase(GL_UNIFORM_BUFFER, ubBindingPoint, uniform_buffer); //Associate this uniform buffer with the uniform block in the shader that has binding = 2.
   glBindBuffer(GL_UNIFORM_BUFFER, 0);


   //Set initial values for UBO variables
   LightingUniformData.La = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
   LightingUniformData.Ld = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
   LightingUniformData.Ls = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

   LightingUniformData.ka = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
   LightingUniformData.kd = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
   LightingUniformData.ks = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
   LightingUniformData.light_w = glm::vec4(-10.0f, 10.0f, 10.0f, 1.0f);

   LightingUniformData.shininess = 8.0f;

   reload_shader();
   mesh_data = LoadMesh(mesh_name);
   texture_id = LoadTexture(texture_name.c_str());

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glGenerateMipmap(GL_TEXTURE_2D);
}

int main (int argc, char **argv)
{
   //Configure initial window state
   glutInit(&argc, argv); 
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
   glutInitWindowPosition (5, 5);
   glutInitWindowSize (512, 512);
   int win = glutCreateWindow ("Uniform Buffer");

   printGlInfo();

   //Register callback functions with glut. 
   glutDisplayFunc(display); 
   glutKeyboardFunc(keyboard);
   glutIdleFunc(idle);

   initOpenGl();

   //Enter the glut event loop.
   glutMainLoop();
   glutDestroyWindow(win);
   return 0;		
}

