#version 440

uniform float time;
uniform mat4 Q;
uniform sampler2DMS backfaces;
uniform sampler2DMS frontfaces;
uniform int pass;

sample in vec3 vpos;

out vec4 fragcolor;   

//function declarations
vec4 raytracedcolor(vec3 rayStart, vec3 rayStop);
vec4 lighting(vec3 pos, vec3 rayDir);
float distToShape(vec3 pos);
vec3 normal(vec3 pos);

void main(void)
{   
	//backfaces get position rendered to texture
	if(pass == 1 || pass == 2)
	{
		fragcolor = vec4(vpos, 1.0);
		return;	
	}

	//uncomment to show backface texture
	//fragcolor = texelFetch(backfaces, ivec2(gl_FragCoord.xy), 0);
	//return;

	vec3 rayStart = vpos.xyz;

	vec3 rayStop = texelFetch(backfaces, ivec2(gl_FragCoord.xy), gl_SampleID).xyz;
	fragcolor = raytracedcolor(rayStart, rayStop);

   if(fragcolor.a==0.0) discard;
   
   fragcolor = pow(fragcolor, vec4(0.45, 0.45, 0.45, 1.0));
}

// trace rays until they intersect the surface
vec4 raytracedcolor(vec3 rayStart, vec3 rayStop)
{
	vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
	float scale = 1.0/Q[0].x;
	int MaxSamples = 1000;

	vec3 rayDir = normalize(rayStop-rayStart);
	float travel = distance(rayStop, rayStart);
	float stepSize = travel/MaxSamples;
	vec3 pos = rayStart;
	vec3 step = rayDir*stepSize;
	
	for (int i=0; i < MaxSamples && travel > 0.0; ++i, pos += step, travel -= stepSize)
	{
		float dist = distToShape(pos);

		stepSize = dist;
		step = rayDir*stepSize;
		
		if(dist<=0.001/scale)
		{
			color = lighting(pos, rayDir);
			return color;
		}	
	}
	return color;
}

//compute lighting on the intersected surface
vec4 lighting(vec3 pos, vec3 rayDir)
{
	const vec3 light = vec3(1.0/1.7, 1.0/1.7, 1.0/1.7);
	const vec4 ambient_color = vec4(0.1, 0.2, 0.1, 1.0);
	const vec4 diffuse_color = vec4(0.5, 0.7, 0.5, 1.0);
	const vec4 spec_color = vec4(0.6, 0.6, 0.6, 1.0);
	vec3 n = normal(pos);
	vec3 v = -rayDir;
	vec3 r = reflect(-light, n);

	return ambient_color + diffuse_color*max(0.0, dot(n, light)) + spec_color*pow(max(0.0, dot(r, v)), 50.0);
}


//shape function declarations
float sdSphere( vec3 p, float s );

// For more distance functions see
// http://iquilezles.org/www/articles/distfunctions/distfunctions.htm

// Soft shadows
// http://www.iquilezles.org/www/articles/rmshadows/rmshadows.htm

// WebGL example and a simple ambient occlusion approximation
// https://www.shadertoy.com/view/Xds3zN


//distance to the shape we are drawing
float distToShape(vec3 pos)
{
	float radius = 0.4;
	return sdSphere(pos, radius);

}

//normal vector of the shape we are drawing
vec3 normal(vec3 pos)
{
	const float h = 0.001;
	const vec3 Xh = vec3(h, 0.0, 0.0);	
	const vec3 Yh = vec3(0.0, h, 0.0);	
	const vec3 Zh = vec3(0.0, 0.0, h);	

	return normalize(vec3(distToShape(pos+Xh)-distToShape(pos-Xh), distToShape(pos+Yh)-distToShape(pos-Yh), distToShape(pos+Zh)-distToShape(pos-Zh)));
}
  
  
// shape function definitions
                
float sdSphere( vec3 p, float s )
{
	return length(p)-s;
}

















