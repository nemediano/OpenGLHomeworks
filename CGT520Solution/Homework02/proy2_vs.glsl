#version 150
in vec4 aPosition; 
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec4 Color; 
out vec4 vColor;

void main(void)
{ 
  gl_Position = P * V * M *aPosition; 
  vColor = Color; 
}
