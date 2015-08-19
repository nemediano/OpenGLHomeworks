#version 150            
uniform mat4 VM; 
uniform mat4 P;  
uniform vec3 light;    //light direction

in vec3 pos_attrib;   
in vec3 normal_attrib;  
in vec2 tex_coord_attrib; 

out vec3 vcolor;
out vec2 vtext_coordinates;

const vec3 ka = vec3(0.1, 0.1, 0.2);		//ambient material color
const vec3 kd = vec3(0.0, 0.3, 0.35);		//diffuse material color
const vec3 ks = vec3(0.5, 0.9, 1.0);        //specular material color
const vec3 La = vec3(0.2, 0.2, 0.2);		//ambient light color
const vec3 Ld = vec3(1.0, 1.0, 1.0);		//diffuse light color
const vec3 Ls = vec3(1.0, 1.0, 1.0);		//specular light color

vec3 ambient_term(vec3 k_a, vec3 L_a);
vec3 diffuse_term(vec3 k_d, vec3 L_d, vec3 n);
vec3 specular_term(vec3 k_a, vec3 L_a, vec3 r);

void main(void) {
	vec3 n = normalize((VM * vec4(normal_attrib, 0.0f)).xyz);		//eye-space normal
	vcolor = ambient_term(ka, La) + diffuse_term(kd, Ld, n) + specular_term(ks, Ls, n);
	vtext_coordinates = tex_coord_attrib;
	gl_Position = P * VM * vec4(pos_attrib, 1.0);
}

vec3 ambient_term(vec3 k_a, vec3 L_a) {
	return k_a * L_a;
}

vec3 diffuse_term(vec3 k_d, vec3 L_d, vec3 n) {
	vec3 l = normalize(light);
	//clamp dot product to avoid negative reflectance
	float ndotl = max(0.0, dot(n,l));
	return k_d * L_d * ndotl;
}

vec3 specular_term(vec3 k_s, vec3 L_s, vec3 n) {
	float alpha = 15.0;
	vec3 v = normalize(-(VM * vec4(pos_attrib, 1.0)).xyz);		//eye space view
	vec3 l = normalize(light);
	//Remember that GLSL uses the incident vector, so we need to invert l
	vec3 r = normalize(reflect(-l, n));
	//clamp dot product to avoid negative reflectance
	float rdotv = max(0.0, dot(r, v));
	return k_s * L_s * pow(rdotv, alpha);
}