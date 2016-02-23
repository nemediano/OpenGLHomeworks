#version 430

//Inputs from vertex shader
in vec3 fNormal;
//Position of the fragment in view space
in vec3 fPosition;
//Texture coordinates of this fragment
in vec2 fTextCoord;

layout (location = 0) out vec4 fragshading;

//Light source properties
uniform vec3 La;
uniform vec3 Ld;
uniform vec3 Ls;
uniform vec3 lightPosition;
//Material properties
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float shininess;

uniform int texture_option;
uniform float time;

//Texture map
uniform sampler2D texture_map;

subroutine vec3 shadingModel(vec3 fKa, vec3 fKd, vec3 fKs, float fshininnes);

vec3 phong_shading(vec3 fKa, vec3 fKd, vec3 fKs, float fshininnes);

subroutine uniform shadingModel selectedModel;

void main(void) {
	//Material properties taken from the texture
	vec3 fKa = mix(Ka, vec3(texture2D(texture_map, fTextCoord)), texture_option);
	vec3 fKd = mix(Kd, fKa, texture_option);
	const vec3 fKs = mix(Ks, vec3(1.0f), texture_option);
	const float fshine = mix(shininess, 32.0f, texture_option);
	
	fragshading = vec4(selectedModel(fKa, fKd, fKs, fshine), 1.0);
}

subroutine (shadingModel) vec3 phong_shading(vec3 fKa, vec3 fKd, vec3 fKs, float fshininnes) {
	
	vec3 n = fNormal;
	//Light position must be in Viewspace (Same as fPosition)
	vec3 l = normalize(lightPosition - fPosition);
	//Since we are in view space the eye is at the origin so v = eye - p = -p
	vec3 v = normalize(-fPosition);
	//Since in GLSL the first argument of the reflect function 
	//is "incident" vector. I'm going to use the negative of the
	//vector that usually use on the books
	vec3 r = normalize(reflect(-l, fNormal));

	vec3 ambient_term = fKa * La;
	vec3 diffuse_term = fKd * Ld * max(0.0, dot(n, l));
	vec3 specular_term = fKs * Ls * pow(max(0.0, dot(r, v)), fshininnes);
	
	return ambient_term + diffuse_term + specular_term;
}
