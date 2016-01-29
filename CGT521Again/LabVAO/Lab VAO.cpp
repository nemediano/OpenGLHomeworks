#include <iostream>
#include <vector>


#include <GL/glew.h>
#include <GL/freeglut.h>


#include "InitShader.h"
#include "UseVbo.h"
#include "UseVao.h"

static const std::vector<std::string> vertex_shader = {"vao0_vs.glsl", "vao1_vs.glsl"};
static const std::vector<std::string> fragment_shader = {"vao0_fs.glsl", "vao1_fs.glsl"};

GLuint shader_program[2] = {-1, -1};
GLuint vao[2] = {-1, -1};
GLuint vbo[2] = {-1, -1};

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

// glut display callback function.
// This function gets called every time the scene gets redisplayed 
void display()
{
   glClear(GL_COLOR_BUFFER_BIT);

   //this doesn't work
   glUseProgram(shader_program[0]); //subsequent draw calls will use shader_program[0]
   glDrawArrays(GL_TRIANGLES, 0, 3);// draw 3 vertices as triangles
   glUseProgram(shader_program[1]);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);// draw 4 vertices as a triangle strip

   glutSwapBuffers(); 
}

void idle()
{
	glutPostRedisplay(); 
}

void reload_shaders()
{
   for(int i=0; i<2; i++)
   {
      GLuint new_shader = InitShader(vertex_shader[i].c_str(), fragment_shader[i].c_str());

      if(new_shader == -1) // loading failed
      {
         glClearColor(1.0f, 0.0f, 1.0f, 0.0f); // magenta clear color is a clue that shader compilation or linking failed
         return;
      }
      else
      {
         glClearColor(0.35f, 0.35f, 0.35f, 0.0f);

         if(shader_program[i] != -1)
         {
            glDeleteProgram(shader_program[i]);
         }
         shader_program[i] = new_shader;
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
         reload_shaders();     
      break;

      case '1':
         init_vao();
         glutDisplayFunc(display);
      break;

      case '2':
         glutDisplayFunc(draw_from_vbo);
      break;

      case '3':
         init_vao();
         glutDisplayFunc(draw_from_vao);
      break;
   }
}

void print_gl_info()
{
   std::cout << "Vendor: "       << glGetString(GL_VENDOR)                    << std::endl;
   std::cout << "Renderer: "     << glGetString(GL_RENDERER)                  << std::endl;
   std::cout << "Version: "      << glGetString(GL_VERSION)                   << std::endl;
   std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION)  << std::endl;
}

void init_vbo()
{
   float triangle_verts[] = 
      {  0.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 0.0f,
      0.5f, 1.0f, 0.0f};

   float square_verts[] = 
      {  -1.0f, -1.0f, 0.0f,
         -1.0f, 0.0f, 0.0f,
         0.0f, -1.0f, 0.0f, 
         0.0f, 0.0f, 0.0f};
   

   glGenBuffers(2, vbo); // generate vbos to hold vertex attributes for triangle and square
   glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //binding vbo[0] means that subsequent bufferdata calls will load data into this object
   glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_verts), &triangle_verts[0], GL_STATIC_DRAW);  //upload from main memory to gpu memory

   glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
   glBufferData(GL_ARRAY_BUFFER, sizeof(square_verts), &square_verts[0], GL_STATIC_DRAW); 

   glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind vbo
}

void init_opengl()
{
   glewInit();
   reload_shaders();
   init_vbo();
}

int main (int argc, char **argv)
{
   //Configure initial window state
   glutInit(&argc, argv); 
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA);
   glutInitWindowPosition (5, 5);
   glutInitWindowSize (512, 512);
   int win = glutCreateWindow ("Lab VAO");

   print_gl_info();

   //Register callback functions with glut. 
   glutDisplayFunc(display); 
   glutKeyboardFunc(keyboard);
   glutIdleFunc(idle);

   init_opengl();

   //Enter the glut event loop.
   glutMainLoop();
   glutDestroyWindow(win);
   return 0;		
}

