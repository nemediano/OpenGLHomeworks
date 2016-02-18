/*
Warning: this example uses some deprecated OpenGL features (glMatrixMode, eetc.). It is meant to illustrate 
the use of transform feedback only.
*/

#include <stdio.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <GL/glew.h>
#include <GL/freeglut.h>


#include "TransformFeedback.h"

static int win = 0;
float angle = 0.0;

//Shaders and Programs
GLuint VShader;
GLuint FShader;
GLuint Program;

const char* vs_code = 
//Vertex Shader (particle animation)
"#version 120\n"
"attribute vec4 v_in;"
"varying vec4 p_out;"
"varying vec4 v_out;"
"vec4 vel(vec3 p);"
"vec3 v(vec3 p);"
"void main(void){"
"  v_out = 0.2*v_in + 0.8*vel(2.0*gl_Vertex.xyz) - vec4(0.0, 0.0, 0.2, 0.0);"	//apply gravity
"  v_out.w = v_in.w + 0.005;"
"  p_out = gl_Vertex + 0.05*vec4(v_out.xyz, 0.0);"
"  if(p_out.z < -1.0) {p_out.z+=1.0; v_out.w = 5.0; v_out.w = 7.0;}"		//basic collision detection
"  if(p_out.z > +1.0) {p_out.z-=1.0; v_out.w = 5.0; v_out.w = 7.0;}"		//basic collision detection

"  if(p_out.y < -1.0) {p_out.y+=1.0; v_out.w = 5.0; v_out.w = 7.0;}"		//basic collision detection
"  if(p_out.y > +1.0) {p_out.y-=1.0; v_out.w = 5.0; v_out.w = 7.0;}"		//basic collision detection

"  if(p_out.x < -1.0) {p_out.x+=1.0; v_out.w = 5.0; v_out.w = 7.0;}"		//basic collision detection
"  if(p_out.x > +1.0) {p_out.x-=1.0; v_out.w = 5.0; v_out.w = 7.0;}"		//basic collision detection
"  gl_Position = gl_ModelViewProjectionMatrix*p_out;"
"  gl_PointSize = v_out.w;"
"  gl_TexCoord[0] = gl_MultiTexCoord0;"

"}"
"vec4 vel(vec3 p)"
"{"
"	const int n = 8;"
"	vec3 octaves = vec3(0.0);"
"	float scale = 1.0;"
"	for(int i=0; i<n; i++)"
"	{"
"		octaves = octaves + v(scale*p)/scale;"
"		scale*= 1.5;"
"	}"
"	return vec4(0.1*octaves, 0.0);"
"}"

"vec3 v0(vec3 p)"
"{"
"	return vec3(sin(p.y*5.0), -sin(p.x*5.0), +cos(1.2*p.z));"
"}"

"vec3 v(vec3 p)"
"{"
"	return v0(p.xyz + vec3(0.0, 2.0*v_in.w, 0.0)) + v0(p.zyx);"
"}"
;

const char* fs_code = 
//Fragment Shader 
"#version 120\n"
"varying vec4 p_out;"
"varying vec4 v_out;"
"void main(void){"
"  float r = length(gl_TexCoord[0].st-vec2(0.5));"
"  float s = smoothstep(1.0, 0.0, r);"
"  gl_FragColor = vec4(0.7, 0.2, 0.1, 0.2*s);"
"}";

//number of particles
const int nvertices = 20000;
GLuint VBO_ID[2];
int Read_Index = 0;  //initially read from VBO_ID[0]
int Write_Index = 1; //initially write to VBO_ID[1]

void InitOpenGL()
{
	glewInit();

   float* lpVBOdata = new float[nvertices*(4+4)]; //4 floats for vertex location, 4 for particle velocity
   float* lpPos = lpVBOdata; //Separate attribs in one VBO: positions first, velocities second
   float* lpVel = lpVBOdata+nvertices*4;

   //put init vertex locations into client-side array
   for(int i=0; i<nvertices; i++)
   {
      lpPos[4*i+0] = 0.5f*(float(rand())/RAND_MAX-0.5f);
      lpPos[4*i+1] = 0.5f*(float(rand())/RAND_MAX-0.5f);
      lpPos[4*i+2] = 0.5f*(float(rand())/RAND_MAX-0.5f);
      lpPos[4*i+3] = 1.0f;
   }

   //put init velocities into client-side array
   for(int i=0; i<nvertices; i++)
   {
      lpVel[4*i+0] = 0.5f*(float(rand())/RAND_MAX-0.5f);
      lpVel[4*i+1] = 0.5f*(float(rand())/RAND_MAX-0.5f);
      lpVel[4*i+2] = 0.5f*(float(rand())/RAND_MAX-0.5f);
      lpVel[4*i+3] = 5.0f;
   }

   //initialize VBOs
   glGenBuffers(2, VBO_ID); // Generate 2: one for reading and one for writing
   glBindBuffer(GL_ARRAY_BUFFER, VBO_ID[Read_Index]);
   glBufferData(GL_ARRAY_BUFFER, nvertices*8*sizeof(float), lpVBOdata, GL_STREAM_COPY); //buffer init values

   glBindBuffer(GL_ARRAY_BUFFER, VBO_ID[Write_Index]);
   glBufferData(GL_ARRAY_BUFFER, nvertices*8*sizeof(float), 0, GL_STREAM_COPY); //allocate, but don't initialize the write buffer
   glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, VBO_ID[Write_Index], 0, nvertices*4*sizeof(float)); //variable 0 is associated with the first half of the VBO.
   glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 1, VBO_ID[Write_Index], nvertices*4*sizeof(float), nvertices*4*sizeof(float)); //variable 1 is associated with the second half of the VBO.

   //create shaders
   int status;

   VShader = glCreateShader(GL_VERTEX_SHADER);
   FShader = glCreateShader(GL_FRAGMENT_SHADER);

   glShaderSource(VShader, 1, &vs_code, NULL);
   glCompileShader(VShader);
   glGetShaderiv(VShader, GL_COMPILE_STATUS, &status);
   if(status == GL_FALSE)
   {
      printShaderInfoLog(VShader);
   }

   glShaderSource(FShader, 1, &fs_code, NULL);
   glCompileShader(FShader);
   glGetShaderiv(FShader, GL_COMPILE_STATUS, &status);
   if(status == GL_FALSE)
   {
      printShaderInfoLog(FShader);
   }

      
   Program = glCreateProgram(); 

   glAttachShader(Program, VShader);
   glAttachShader(Program, FShader);

   //Specify what the glsl variable names are for variable 0 and variable 1.
   const char *vars[] = {"p_out", "v_out"};
   glTransformFeedbackVaryings(Program, 2, vars, GL_SEPARATE_ATTRIBS);
      
   glLinkProgram(Program);
   glGetProgramiv(Program, GL_LINK_STATUS, &status);
   if(status == GL_FALSE)
   {
      printProgramInfoLog(Program);
   }   

   //enable some basic rendering state
   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
   glPointSize(5.0f);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE); //additivve alpha blending
   glColor4f(0.75f, 0.125f, 0.25f, 0.75f);

   glTexEnvf(GL_POINT_SPRITE,GL_COORD_REPLACE,GL_TRUE);

   //set camera viewing volume (fov, aspect ratio, near, far clip dist)
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(70.0, 1.0, 0.01, 10.0);
   glError();
}

void display()
{
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); 

   //set camera position and orientation
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   
   glBindBuffer(GL_ARRAY_BUFFER, VBO_ID[Read_Index]);
   glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, VBO_ID[Write_Index], 0, nvertices*4*sizeof(float));
   glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 1, VBO_ID[Write_Index], nvertices*4*sizeof(float), nvertices*4*sizeof(float));

   glUseProgram(Program);
   GLint user_loc = glGetAttribLocation(Program, "v_in");
   glPushMatrix();
   glTranslatef(0.0f, 0.0f, -5.0f);
   glScalef(2.0f, 2.0f, 2.0f);


   glEnableClientState(GL_VERTEX_ARRAY); 
   glEnableVertexAttribArray(user_loc);
   glVertexPointer(4, GL_FLOAT, 0, BUFFER_OFFSET(0)); 
   glVertexAttribPointer(user_loc, 4, GL_FLOAT, false, 0, BUFFER_OFFSET(nvertices*4*sizeof(float)));

   glEnable(GL_POINT_SPRITE);
   glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

   //Transform feedback is enabled within this begin/end block
   glBeginTransformFeedback(GL_POINTS);
   glDrawArrays(GL_POINTS, 0, nvertices); 
   glEndTransformFeedback();


   glDisableClientState(GL_VERTEX_ARRAY); 
   glDisableVertexAttribArray(user_loc);
   glPopMatrix();
   glUseProgram(0);

   //Ping-pong buffers
   Read_Index = 1-Read_Index;
   Write_Index = 1-Write_Index;

   glutSwapBuffers();

   glError();
}


//check for errors and print error codes.
void glError()
{
   GLenum errCode;
   const GLubyte *errString;
   if((errCode = glGetError()) != GL_NO_ERROR)
   {
      errString = gluErrorString(errCode);
      printf("OpenGL Error: %s\n", errString);
   }
}

void keyboard(unsigned char key, int x, int y)
{
   if(key==27) //press ESC to exit
   {
      exit(0);
   }  
}

void idle()
{  
   //timers for time-based animation
   static int last_time = 0;
   int time = glutGet(GLUT_ELAPSED_TIME);
   int elapsed = time-last_time;
   float delta_seconds = 0.001f*elapsed;
   last_time = time;

   angle += 10.0f*delta_seconds;

   glError(); //check for errors and print error strings
   glutPostRedisplay();
}

void CreateGlutWindow()
{
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
   glutInitContextVersion (3, 1);	//for OpenGL 3.1 context
   glutInitWindowSize (640, 640);
   win = glutCreateWindow ("Transform Feedback Demo");

   //print system info
   printf("Vendor: %s\n", glGetString(GL_VENDOR));
   printf("Renderer: %s\n", glGetString(GL_RENDERER));
   printf("Version: %s\n", glGetString(GL_VERSION));
   //printf("Extensions: %s\n", glGetString(GL_EXTENSIONS));
}

void CreateGlutCallbacks()
{
   glutDisplayFunc   (display);
   glutIdleFunc	   (idle);
   glutKeyboardFunc  (keyboard);
}



void printShaderInfoLog(GLuint obj)
{
   int infologLength = 0;
   int charsWritten  = 0;
   char *infoLog;

   glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

   if (infologLength > 0)
   {
      infoLog = (char *)malloc(infologLength);
      glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog); 
      printf("%s\n",infoLog);
      free(infoLog);
   }
}

void printProgramInfoLog(GLuint obj)
{
   int infologLength = 0;
   int charsWritten  = 0;
   char *infoLog;

   glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

   if (infologLength > 0)
   {
      infoLog = (char *)malloc(infologLength);
      glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
      printf("%s\n",infoLog);
      free(infoLog);
   }
}

void ExitGlut()
{
   glutDestroyWindow(win);
   exit(0);
}

int main (int argc, char **argv)
{
   glutInit(&argc, argv); 
   CreateGlutWindow();
   CreateGlutCallbacks();
   InitOpenGL();
   glutMainLoop();
   ExitGlut();
   return 0;
}