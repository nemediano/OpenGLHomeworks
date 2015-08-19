#version 150
uniform float time;
uniform vec4 view;
uniform int fish_animation;
uniform mat4 VM;
uniform mat4 P;

in vec3 pos_attrib;
in vec2 tex_coord_attrib;
in vec3 normal_attrib;

out vec2 text_coord;
out vec3 fragment_normal;
out vec3 position_viewspace;

void main(void){
   vec4 position;
   position = vec4(pos_attrib, 1.0);
   
   if (fish_animation != 0) {
	  position.z += sin(3.0 * (time + 2.8 * position.x)) / 32.0;
   }

   mat3 rot_VM = mat3(VM);

   text_coord = tex_coord_attrib;
   //Remember that we need to transform the normal as well. (Only rotations and uniform scaling)
   fragment_normal =  rot_VM * normal_attrib;
   position_viewspace = (VM * position).xyz;
   
   gl_Position = P * VM * position;
}
