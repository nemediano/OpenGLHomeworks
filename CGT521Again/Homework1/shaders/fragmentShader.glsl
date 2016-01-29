#version 330

//Inputs from vertex shader
in vec3 fNormal;
//Position of the fragment in view space
in vec3 fPosition;
flat in int instance_id;

out vec4 fragcolor;

//Light source properties
uniform vec3 La;
uniform vec3 Ld;
uniform vec3 Ls;
uniform vec3 lightPosition;

//Material properties
uniform vec3 Ks;
uniform float shininess;

const vec3 colors[9] = vec3[9](
	vec3(1.0, 1.0, 1.0), vec3(0.0, 1.0, 1.0), vec3(1.0, 0.0, 1.0),
	vec3(1.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0), vec3(0.0, 0.0, 0.0), vec3(0.5, 0.5, 0.5)
);

void main(void) {
	vec3 Ka = colors[instance_id % 9];
	vec3 Kd = Ka;
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

	fragcolor = vec4(ambient_term + diffuse_term + specular_term, 1.0);
}
