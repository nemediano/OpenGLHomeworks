#version 150
in vec3 aPosition; 
uniform mat4 uM;
uniform vec4 uColor; 
out vec4 vColor; //varying variable
out vec3 vPos;
void main(void)
{ 
  gl_Position = uM*vec4(aPosition, 1.0); 
  vPos = aPosition;
  vColor = uColor; 
}
