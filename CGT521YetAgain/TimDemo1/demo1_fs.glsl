#version 400

uniform float time;

in vec3 fPosition;
out vec4 fragcolor;      
                
void main(void)
{   
	
	const float SPEED = 0.001f;
	const float REPETITIONS = 5.0f;
	float f = 0.5 * sin(SPEED * time + REPETITIONS * fPosition.x) + 0.5;
	float g = 0.5 * cos(SPEED * time + REPETITIONS * fPosition.y) + 0.5;
	
	fragcolor = max(f, g) * vec4(1.0);
}




















