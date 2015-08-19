#version 150
in vec4 aPosition; 
uniform mat4 uM;
uniform vec4 uColor; 
out vec4 vColor;

void main(void)
{ 
  gl_Position = uM*aPosition; 
  vColor = uColor; 
}
