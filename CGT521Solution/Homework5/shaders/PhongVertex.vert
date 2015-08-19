#version 150
uniform float time;
uniform int fish_animation;
uniform int texture_map_flag;
uniform int rotate_animation;
uniform mat4 VM;
uniform mat4 P;

in vec3 pos_attrib;
in vec2 tex_coord_attrib;
in vec3 normal_attrib;

out vec2 text_coord;
out vec3 fnormal;
out vec3 position_viewspace;

mat4 rotationMatrix(vec3 rot_axis, float angle);

void main(void){
   vec4 position;
   position = vec4(pos_attrib, 1.0);
   if (fish_animation != 0) {
	  position.z += sin(3.0 * (time + 2.8 * position.x)) / 32.0;
   }
   
   mat3 rot_VM = mat3(VM);

   if (rotate_animation != 0) {
      mat4 Rot_in_y = rotationMatrix(vec3(0.0, 1.0, 0.0), time); 
      position = Rot_in_y * position;
	  rot_VM = rot_VM * mat3(Rot_in_y);
   }
   
   text_coord = tex_coord_attrib;
   //fnormal =  (VM * vec4(normal_attrib, 0.0)).xyz;
   fnormal =  rot_VM * normal_attrib;
   position_viewspace = (VM * position).xyz;
   //fnormal =  normal_attrib;
   gl_Position = P * VM * position;
}

mat4 rotationMatrix(vec3 rot_axis, float angle) {
   vec3 axis = normalize(rot_axis);
   float s = sin(angle);
   float c = cos(angle);
   float oc = 1.0 - c;
   return mat4(oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s, 0.0,
               oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s, 0.0,
               oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c, 0.0,
               0.0, 0.0, 0.0, 1.0);
}