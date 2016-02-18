#version 430

in vec4 Position;
in vec3 Normal;
in vec2 TextureCoordinate;

uniform mat4 PVM;
uniform mat4 NormalMatrix;
uniform mat4 VM;

//Outputs for fragment shader
out vec3 fNormal;
out vec3 fPosition;
out vec2 fTextCoord;
out flat int fInstanceId;

const int TOTAL_COLS = 2;
const int TOTAL_ROWS = 3;

const vec4 i = vec4(1.0f, 0.0f, 0.0f, 0.0f);
const vec4 j = vec4(0.0f, 1.0f, 0.0f, 0.0f);

void main(void) {
	const vec4 start = vec4(-0.9f, -1.2f, 0.0f, 0.0f);
	const float deltaRow = 1.2f;
	const float deltaCol = 1.9f;
	int row = gl_InstanceID % TOTAL_ROWS;
	int col = gl_InstanceID % TOTAL_COLS;
	vec4 shift = (deltaRow * row) * j + (deltaCol * col) * i + start;
	gl_Position = (PVM * Position) + shift;
	fNormal = normalize(vec3(NormalMatrix * vec4(Normal, 1.0)));
	//Position needs to be in view space for lighting
	fPosition = vec3((VM * Position) + shift);
	fTextCoord = TextureCoordinate;
	fInstanceId = gl_InstanceID;
}
