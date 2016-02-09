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
#include "Cube.h"

static const std::string vertex_shader("ubo_vs.glsl");
static const std::string fragment_shader("ubo_fs.glsl");

GLuint shader_program = -1;
GLuint texture_id = -1;
GLuint uniform_buffer = -1;

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
   int pass;

} LightingUniformData;

static const std::string mesh_name = "Amago0.obj";
static const std::string texture_name = "AmagoT.bmp";

static const std::string occluder_name = "skyscraper2.obj";

MeshData mesh_data;
MeshData occluder_data;
MeshData cube_data;
const int rows = 50;

GLuint OcclusionQueries[rows*rows];
GLuint TimerQuery;

float time_sec;

glm::mat4 T_box;
glm::mat4 S_mesh;

void draw_occluders();
void draw_bounding_volumes();
void conditional_draw_meshes();
void draw_meshes();

// glut display callback function.
// This function gets called every time the scene gets redisplayed 
void display_conditional()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glUseProgram(shader_program);

   //Update the values that have changed.
   
   LightingUniformData.eye_w = glm::vec4(0.0f, 2.0f, 12.0f, 1.0f);
   glm::mat4 V = glm::lookAt(glm::vec3(LightingUniformData.eye_w), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f))*glm::rotate(10.0f*time_sec, glm::vec3(0.0f, 1.0f, 0.0f));
   glm::mat4 P = glm::perspective(TAU / 9.0f, 1.0f, 0.1f, 100.0f);
   LightingUniformData.PV = P*V;

   glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer); //Bind the OpenGL UBO before we update the data.

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, texture_id);
   draw_occluders();

   //draw bounding volumes
   glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
   glDepthMask(GL_FALSE);

   draw_bounding_volumes();
   glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
   glDepthMask(GL_TRUE);
   

   conditional_draw_meshes();

   glutSwapBuffers();
   glBindBuffer(GL_UNIFORM_BUFFER, 0); 
}

void display()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glUseProgram(shader_program);

   //Update the values that have changed.
   LightingUniformData.eye_w = glm::vec4(0.0f, 2.0f, 12.0f, 1.0f);
   glm::mat4 V = glm::lookAt(glm::vec3(LightingUniformData.eye_w), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f))*glm::rotate(10.0f*time_sec, glm::vec3(0.0f, 1.0f, 0.0f));
   glm::mat4 P = glm::perspective(TAU / 9.0f, 1.0f, 0.1f, 100.0f);
   LightingUniformData.PV = P*V;

   glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer); //Bind the OpenGL UBO before we update the data.

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, texture_id);
   draw_occluders();
   draw_meshes();

   glutSwapBuffers();
   glBindBuffer(GL_UNIFORM_BUFFER, 0); 
}

void display_debug()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glUseProgram(shader_program);

   //Update the values that have changed.
   LightingUniformData.eye_w = glm::vec4(0.0f, 2.0f, 12.0f, 1.0f);
   glm::mat4 V = glm::lookAt(glm::vec3(LightingUniformData.eye_w), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f))*glm::rotate(10.0f*time_sec, glm::vec3(0.0f, 1.0f, 0.0f));
   glm::mat4 P = glm::perspective(TAU / 9.0f, 1.0f, 0.1f, 100.0f);
   LightingUniformData.PV = P*V;

   glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer); //Bind the OpenGL UBO before we update the data.

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, texture_id);
   draw_occluders();
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   draw_bounding_volumes();
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   draw_meshes();

   glutSwapBuffers();
   glBindBuffer(GL_UNIFORM_BUFFER, 0); 
}


void draw_occluders()
{
   LightingUniformData.pass = 0;
   glBindVertexArray(occluder_data.mVao);
   glm::mat4 S_occluder = glm::scale(glm::vec3(occluder_data.mScaleFactor))*glm::scale(glm::vec3(1.5f, 2.5f, 1.5f));
   for(int i=0; i<rows/2; i++)
   {
      float x = 2.0f*(i-rows/4);
      for(int j=0; j<rows/2; j++)
      {
         float y = 2.0f*(j-rows/4) + 0.5f;
         LightingUniformData.M = glm::translate(glm::vec3(x,0.0f,y))*S_occluder;
         glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightingUniformData), &LightingUniformData); //Upload the new uniform values.
   
	      glDrawElements(GL_TRIANGLES, occluder_data.mNumIndices, GL_UNSIGNED_INT, 0);
      }
   }
}

void draw_bounding_volumes()
{
   LightingUniformData.pass = 1;
   glBindVertexArray(cube_data.mVao);
   glBindTexture(GL_TEXTURE_2D, 0);
   for(int i=0; i<rows; i++)
   {
      float x = 1.0f*(i-rows/2);
      for(int j=0; j<rows; j++)
      {
         float y = 1.0f*(j-rows/2);
         LightingUniformData.M = glm::translate(glm::vec3(x,0.0f,y))*glm::scale(glm::vec3(cube_data.mScaleFactor))*T_box*S_mesh;
         glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightingUniformData), &LightingUniformData); //Upload the new uniform values.
         glBeginQuery(GL_ANY_SAMPLES_PASSED, OcclusionQueries[i+rows*j]);
	      glDrawElements(GL_TRIANGLES, cube_data.mNumIndices, GL_UNSIGNED_SHORT, 0);
         glEndQuery(GL_ANY_SAMPLES_PASSED);
      }
   }
}

void conditional_draw_meshes()
{
   LightingUniformData.pass = 2;
   glBindVertexArray(mesh_data.mVao);
   glBindTexture(GL_TEXTURE_2D, texture_id);
   for(int i=0; i<rows; i++)
   {
      float x = 1.0f*(i-rows/2);
      for(int j=0; j<rows; j++)
      {
         float y = 1.0f*(j-rows/2);
         LightingUniformData.M = glm::translate(glm::vec3(x,0.0f,y))*S_mesh;

         glBeginConditionalRender(OcclusionQueries[i+rows*j], GL_QUERY_WAIT);
         glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightingUniformData), &LightingUniformData); //Upload the new uniform values.
	      glDrawElements(GL_TRIANGLES, mesh_data.mNumIndices, GL_UNSIGNED_INT, 0);
         glEndConditionalRender();
      }
   }
}

void draw_meshes()
{
   LightingUniformData.pass = 2;
   glBindVertexArray(mesh_data.mVao);
   glBindTexture(GL_TEXTURE_2D, texture_id);
   for(int i=0; i<rows; i++)
   {
      float x = 1.0f*(i-rows/2);
      for(int j=0; j<rows; j++)
      {
         float y = 1.0f*(j-rows/2);
         LightingUniformData.M = glm::translate(glm::vec3(x,0.0f,y))*S_mesh;
         glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightingUniformData), &LightingUniformData); //Upload the new uniform values.
	      glDrawElements(GL_TRIANGLES, mesh_data.mNumIndices, GL_UNSIGNED_INT, 0);
      }
   }
}


void idle()
{
   glEndQuery(GL_TIME_ELAPSED);
	glutPostRedisplay();

   const int time_ms = glutGet(GLUT_ELAPSED_TIME);
   time_sec = 0.001f*time_ms;

   

   GLint available=0;
   while(!available)
   {
		glGetQueryObjectiv(TimerQuery, GL_QUERY_RESULT_AVAILABLE, &available);
   }

   GLuint64 elapsed;
   glGetQueryObjectui64v(TimerQuery, GL_QUERY_RESULT, &elapsed);
   float delta_seconds = 1e-9f*elapsed;
   glBeginQuery(GL_TIME_ELAPSED_EXT, TimerQuery);

   printf("delta_seconds = %f\n", delta_seconds);
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

      case '1':
         glutDisplayFunc(display); 
      break;

      case '2':
         glutDisplayFunc(display_conditional);      
      break;

      case '3':
         glutDisplayFunc(display_debug);      
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
   LightingUniformData.pass = 0;

   reload_shader();
   mesh_data = LoadMesh(mesh_name);
   occluder_data = LoadMesh(occluder_name);
   cube_data = CreateCube();

   aiVector3D box_width = mesh_data.mBbMax-mesh_data.mBbMin;
   aiVector3D box_center = 0.5f*(mesh_data.mBbMax+mesh_data.mBbMin);
   T_box = glm::translate(glm::vec3(box_center.x, box_center.y, box_center.z))*glm::scale(glm::vec3(box_width.x, box_width.y, box_width.z));
   S_mesh = glm::scale(glm::vec3(mesh_data.mScaleFactor*0.85f));

   texture_id = LoadTexture(texture_name.c_str());
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glGenerateMipmap(GL_TEXTURE_2D);

   int tex_loc = glGetUniformLocation(shader_program, "texColor");
   if(tex_loc != -1)
   {
      glUniform1i(tex_loc, 0); // we bound our texture to texture unit 0
   }

   glGenQueries(1, &TimerQuery);
   glBeginQuery(GL_TIME_ELAPSED, TimerQuery);

   glGenQueries(rows*rows, OcclusionQueries);
}

int main (int argc, char **argv)
{
   //Configure initial window state
   glutInit(&argc, argv); 
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
   glutInitWindowPosition (5, 5);
   glutInitWindowSize (512, 512);
   int win = glutCreateWindow ("Conditional Rendering");

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

