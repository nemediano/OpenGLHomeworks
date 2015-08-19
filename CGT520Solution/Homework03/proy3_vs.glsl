#version 330

in vec4 aPosition;
in vec4 aNormal;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec4 Color;
uniform vec4 lightDirection;

out vec4 vColor;
out vec4 vNormal;

vec4 ambient_term();
vec4 diffuse_term();
vec4 specular_term();

void main(void) { 
	gl_Position = P * V * M * aPosition; 
	vColor = ambient_term() + diffuse_term();
	vNormal = aNormal;
}

vec4 ambient_term () {
	vec4 k_a = 0.75 * Color;
	vec4 l_a = vec4(0.45, 0.45, 0.45, 1.0);
	
	return l_a * k_a; 
}

vec4 diffuse_term () {
	vec4 k_d = 0.95 * Color;
	vec4 l_d = vec4(0.85, 0.85, 0.85, 1.0);

	//vec4 n = normalize(transpose(inverse(M)) * aNormal);
	vec4 n = normalize(aNormal);

	return l_d * k_d * max(0.0, dot(n, normalize(lightDirection)));
}
