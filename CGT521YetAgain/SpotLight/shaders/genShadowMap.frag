#version 430

/*This is an empthy shader, for shadow map generation you dont need to output any color 
Actually you just need to execute the shader so the depth test is actually made*/
out vec4 fragcolor;
void main(void) {
	fragcolor = vec4(1.0);
}
