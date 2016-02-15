#version 400
uniform int useCubemap;
uniform samplerCube cubetex;
uniform vec4 color;

in vec3 normal_v;
in vec3 view_v;

out vec4 fragcolor;

void main()
{
	if ( useCubemap == 1 )
	{
		vec3 r = reflect(-view_v, normal_v);
      vec4 lighting = color*vec4(abs(normal_v.y))/10.0 - vec4(0.06);
		fragcolor = color*texture(cubetex, r) + lighting;
	}
   else
	{
		fragcolor = color;
	}

}