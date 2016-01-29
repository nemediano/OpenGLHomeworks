#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

// glut display callback function.
// This function gets called every time the scene gets redisplayed 
void display()
{
   glClear(GL_COLOR_BUFFER_BIT);

   //draw something
   glFinish();
}

// glut keyboard callback function.
// This function gets called when an ASCII key is pressed
void keyboard(unsigned char key, int x, int y)
{
   std::cout << "key : " << key << ", x: " << x << ", y: " << y << std::endl;
}

// glut keyboard callback function.
// This function gets called when a mouse button is pressed
void mouse(int button, int state, int x, int y)
{
   std::cout << "button : "<< button << ", state: " << state << ", x: " << x << ", y: " << y << std::endl;
}

	
void printGlInfo()
{
   std::cout << "Vendor: "       << glGetString(GL_VENDOR)                    << std::endl;
   std::cout << "Renderer: "     << glGetString(GL_RENDERER)                  << std::endl;
   std::cout << "Version: "      << glGetString(GL_VERSION)                   << std::endl;
   std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION)  << std::endl;
}

int main (int argc, char **argv)
{
   //Configure initial window state
   glutInit(&argc, argv); 
   glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
   glutInitWindowPosition (5, 5);
   glutInitWindowSize (512, 512);
   int win = glutCreateWindow ("CGT 520 System Info");

   printGlInfo();

   //Register callback functions with glut. 
   glutDisplayFunc(display); 
   glutMouseFunc(mouse);
   glutKeyboardFunc(keyboard);

   //Set the color the screen gets cleared to.
   glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

   //Enter the glut event loop.
   glutMainLoop();
   glutDestroyWindow(win);
   return 0;		
}

