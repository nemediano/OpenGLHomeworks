#version 400            

uniform mat4 PVM;
uniform float time;

in vec3 pos_attrib;
in vec2 tex_coord_attrib;
in vec3 normal_attrib;
out vec3 fPosition;

void main(void){
	const float AMPLITUDE = 0.15f;
	const float FREQUENCY = 0.002f;
	vec3 posPerturbated = pos_attrib;
	posPerturbated += vec3(0.0, AMPLITUDE * sin(FREQUENCY * time + pos_attrib.x), 0.0);
	gl_Position = PVM*vec4(posPerturbated, 1.0);
	fPosition = pos_attrib;
}
