#version 330

in vec4 Position;
in vec3 Normal;

uniform mat4 PVM;
uniform mat4 NormalMatrix;
uniform mat4 VM;

//Outputs for fragment shader
out vec3 fNormal;
out vec3 fPosition;
flat out int instance_id;

//Move according the instance
const vec2 deltas = 150.0 * vec2(1.0, 1.0);
const vec4 translation = -170.0 * vec4(1.0, 1.0, 0.0, 0.0);

void main(void) { 
	int row = gl_InstanceID % 3;
	int col = gl_InstanceID / 3;
	vec4 newPosition = Position + vec4(deltas, 0.0, 0.0) * vec4(col, row, 0.0, 0.0) + translation;
	gl_Position = PVM * newPosition;
	fNormal = normalize(vec3(NormalMatrix * vec4(Normal, 1.0)));
	//Position needs to be in view space for lighting
	fPosition = vec3(VM * newPosition);
	instance_id = gl_InstanceID;
}
