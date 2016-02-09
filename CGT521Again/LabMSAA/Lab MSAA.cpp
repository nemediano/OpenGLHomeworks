#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "InitShader.h"

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

   glUseProgram(shader_program[1]);
   glBindVertexArray(vao[1]);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   glUseProgram(shader_program[0]);
   glBindVertexArray(vao[0]);
   glDrawArrays(GL_LINES, 0, 22);

   glBindVertexArray(0);

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

      //Toggle MSAA on/off
      case '1':
         if(glIsEnabled(GL_MULTISAMPLE))
         {
            glDisable(GL_MULTISAMPLE);
         }
         else
         {
            glEnable(GL_MULTISAMPLE);
         }

      break;
   }
}

void print_gl_info()
{
   std::cout << "Vendor: "       << glGetString(GL_VENDOR)                    << std::endl;
   std::cout << "Renderer: "     << glGetString(GL_RENDERER)                  << std::endl;
   std::cout << "Version: "      << glGetString(GL_VERSION)                   << std::endl;
   std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION)  << std::endl;

   //Query MSAA support
   int spp = 0;
   glGetIntegerv(GL_SAMPLES, &spp);
   std::cout << "MSAA support: " << spp  << " samples per pixel" << std::endl;

   std::cout << "Sample locations: "<< std::endl;
   for(int i=0; i<spp; i++)
   {
      float pos[2];
      glGetMultisamplefv(GL_SAMPLE_POSITION, i, pos);
      std::cout << "Sample " << i  << " is at " << pos[0] <<", "<< pos[1] << std::endl;
   }

}

void init_vbo()
{
   float lines_verts[] = 
      {  -1.0f, -1.0f, 1.0f, 1.0f,              
         -0.9f, -1.0f, 0.9f, 1.0f, 
         -0.8f, -1.0f, 0.8f, 1.0f, 
         -0.7f, -1.0f, 0.7f, 1.0f, 
         -0.6f, -1.0f, 0.6f, 1.0f, 
         -0.5f, -1.0f, 0.5f, 1.0f, 
         -0.4f, -1.0f, 0.4f, 1.0f, 
         -0.3f, -1.0f, 0.3f, 1.0f, 
         -0.2f, -1.0f, 0.2f, 1.0f, 
         -0.1f, -1.0f, 0.1f, 1.0f, 
         -0.0f, -1.0f, 0.0f, 1.0f
      };

   float square_verts[] = 
      {  -0.5f, -0.5f, 0.0f,
         -0.5f, 0.5f, 0.0f,
         0.5f, -0.5f, 0.0f, 
         0.5f, 0.5f, 0.0f};
   

   glGenBuffers(2, vbo); // generate vbos to hold vertex attributes for triangle and square
   glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //binding vbo[0] means that subsequent bufferdata calls will load data into this object
   glBufferData(GL_ARRAY_BUFFER, sizeof(lines_verts), &lines_verts[0], GL_STATIC_DRAW);  //upload from main memory to gpu memory

   glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
   glBufferData(GL_ARRAY_BUFFER, sizeof(square_verts), &square_verts[0], GL_STATIC_DRAW); 

   glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind vbo
}

void init_vao()
{
   glGenVertexArrays(2, vao); //generate vaos to hold the mapping from attrib variables in shader to memory locations in vbo

   glBindVertexArray(vao[0]); //binding vao[0] means that bindbuffer, enablevertexattribarray and vertexattribpointer state will be remembered by vao[0]
   glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //specify the buffer where vertex attribute data is stored
   GLint pos_loc0 = glGetAttribLocation(shader_program[0], "pos_attrib"); //get a reference to an attrib variable name in a shader
	glEnableVertexAttribArray(pos_loc0); //enable this attribute
	glVertexAttribPointer(pos_loc0, 2, GL_FLOAT, false, 0, BUFFER_OFFSET(0)); //tell opengl how to get the attribute values out of the vbo (stride and offset)

   glBindVertexArray(vao[1]);
   glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
   const GLint pos_loc1 = 5; //glGetAttribLocation(shader_program[1], "pos_attrib");
	glEnableVertexAttribArray(pos_loc1);
	glVertexAttribPointer(pos_loc1, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(0));

   glBindVertexArray(0); //unbind vao
   glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind vbo
}


void init_opengl()
{
   glewInit();
   reload_shaders();
   init_vbo();
   init_vao();
   glLineWidth(1.5f);
}

int main (int argc, char **argv)
{
   //Configure initial window state
   glutInit(&argc, argv); 

   //Request MSAA support
   glutSetOption(GLUT_MULTISAMPLE, 4); //Set number of samples per pixel
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);

   glutInitWindowPosition (5, 5);
   glutInitWindowSize (512, 512);
   int win = glutCreateWindow ("Lab MSAA");

   //Register callback functions with glut. 
   glutDisplayFunc(display); 
   glutKeyboardFunc(keyboard);
   glutIdleFunc(idle);

   init_opengl();

   print_gl_info();

   //Enter the glut event loop.
   glutMainLoop();
   glutDestroyWindow(win);
   return 0;		
}

