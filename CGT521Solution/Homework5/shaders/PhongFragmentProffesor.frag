#version 150
uniform float time;
uniform sampler2D texture;
const vec3 light = vec3(0.0, 0.707, -0.707); //light direction
const vec3 view = vec3(0.0, 0.0, -1.0);      //view direction
const vec3 ka = vec3(1.0, 1.0, 1.0);         //ambient material color
const vec3 kd = vec3(1.0, 1.0, 1.0);         //diffuse material color
const vec3 ks = vec3(1.0, 1.0, 1.0);         //specular material color
const vec3 La = vec3(0.2, 0.2, 0.2);         //ambient light color
const vec3 Ld = vec3(0.5, 0.5, 0.5);         //diffuse light color
const vec3 Ls = vec3(0.3, 0.3, 0.3);         //specular light color
in vec3 fnormal;                             //varying normal vector
out vec4 fragcolor;                          //output color
void main(void){ 
   vec3 ambient_term = ka * La;
   vec3 diffuse_term = kd * Ld * max(0.0, dot(fnormal, light));
   vec3 r = reflect(-light, fnormal);
   vec3 specular_term = ks * Ls * pow(max(0.0, dot(r, view)), 8.0);
   vec4 fcolor = vec4(ambient_term + diffuse_term + specular_term, 1.0);
   fragcolor = fcolor;
}
