#version 400
out vec4 fragcolor;      

//const vec4 color = vec4(1.0, 0.0, 0.0, 1.0);
const vec4 colors[9] = vec4[9](
	vec4(1.0, 1.0, 1.0, 1.0), vec4(0.0, 1.0, 1.0, 1.0), vec4(1.0, 0.0, 1.0, 1.0),
	vec4(1.0, 1.0, 0.0, 1.0), vec4(1.0, 0.0, 0.0, 1.0), vec4(0.0, 1.0, 0.0, 1.0),
	vec4(0.0, 0.0, 1.0, 1.0), vec4(0.0, 0.0, 0.0, 1.0), vec4(0.5, 0.5, 0.5, 1.0)
);
 
flat in int instance_id;
 
void main(void)
{   
	//fragcolor = color;
	fragcolor = colors[instance_id % 9];
}




















