#version 150
uniform float time;
uniform mat4 VM;
uniform mat4 P;
in vec3 pos_attrib;
in vec3 normal_attrib;
in vec2 tex_coord_attrib;
out vec3 fnormal;
void main(void){
   fnormal = normal_attrib;
   gl_Position = vec4(pos_attrib, 1.0) * vec4(3.0, 3.0, 1.0, 1.0);
}
