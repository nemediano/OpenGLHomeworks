#version 150                                                                                                               

uniform vec3 light;    //light direction
uniform sampler2D texture_map; //texture map
uniform int texture_option; //Use texture or color

in vec3 vnormal;
in vec3 vview;
in vec2 vtextcoord;
out vec4 fragcolor;                         //output fragment color

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
	vec4 text_color = texture(texture_map, vtextcoord);
	vec3 ambient_color = mix(0.15 * text_color.rgb, ka, texture_option);
	vec3 diffuse_color = mix(0.85 * text_color.rgb, kd, texture_option);
    vec3 n = normalize(vnormal);                      
	fragcolor = vec4(ambient_term(ambient_color, La) + diffuse_term(diffuse_color, Ld, n) + specular_term(ks, Ls, n), 1.0);  
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
	vec3 v = normalize(vview);		//eye space view
	vec3 l = normalize(light);
	//Remember that GLSL uses the incident vector, so we need to invert l
	vec3 r = normalize(reflect(-l, n));
	//clamp dot product to avoid negative reflectance
	float rdotv = max(0.0, dot(r, v));
	return k_s * L_s * pow(rdotv, alpha);
}