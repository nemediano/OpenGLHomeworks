#version 430

//Inputs from vertex shader
in vec3 fNormal;
//Position of the fragment in view space
in vec3 fPosition;
//Texture coordinates of this fragment
in vec2 fTextCoord;


layout (location = 0) out vec4 fragcolor;
layout (location = 1) out vec4 fragshading;
layout (location = 2) out vec4 fragnormal;

//Light source properties
uniform vec3 La;
uniform vec3 Ld;
uniform vec3 Ls;
uniform vec3 lightPosition;
uniform int selected_id;
uniform float time;

//Texture map
uniform sampler2D texture_map;

vec3 phong_shading();

void main(void) {
	fragcolor = texture2D(texture_map, fTextCoord);
	fragshading = vec4(phong_shading(), 1.0);
	fragnormal = vec4(fNormal, 1.0);
}

vec3 phong_shading() {
	//Material properties taken from the texture
	vec3 Ka = vec3(texture2D(texture_map, fTextCoord));
	vec3 Kd = Ka;
	const vec3 Ks = vec3(1.0f);
	const float shininess = 32.0f;

	vec3 n = fNormal;
	//Light position must be in Viewspace (Same as fPosition)
	vec3 l = normalize(lightPosition - fPosition);
	//Since we are in view space the eye is at the origin so v = eye - p = -p
	vec3 v = normalize(-fPosition);
	//Since in GLSL the first argument of the reflect function 
	//is "incident" vector. I'm going to use the negative of the
	//vector that usually use on the books
	vec3 r = normalize(reflect(-l, fNormal));

	vec3 ambient_term = Ka * La;
	vec3 diffuse_term = Kd * Ld * max(0.0, dot(n, l));
	vec3 specular_term = Ks * Ls * pow(max(0.0, dot(r, v)), shininess);
	
	return ambient_term + diffuse_term + specular_term;
}
