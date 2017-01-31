#version 140
in vec3 Position;
in vec3 Normal;
in vec2 TextCoord;
in vec3 Color;

uniform mat4 PVM;

out vec3 fNormal;
out vec2 fTextCoord;
flat out vec3 fColor;

void main(void) {

	const int COLS = 3;
	const float STRIDE = 1.0f;
	vec2 deltaPos = vec2(-STRIDE);
	int index_row = gl_InstanceID / COLS;
	int index_col = gl_InstanceID % COLS;
	deltaPos += float(index_row) * vec2(0.0, 1.0) + float(index_col) * vec2(1.0, 0.0);
	const float SPACE = 120.0;
	vec3 modifiedPos = Position;
	modifiedPos += SPACE * vec3(deltaPos, 0.0f);
	gl_Position = PVM * vec4(modifiedPos, 1.0f);
	fNormal = Normal;
	fTextCoord = TextCoord;
	fColor = Color;
}
