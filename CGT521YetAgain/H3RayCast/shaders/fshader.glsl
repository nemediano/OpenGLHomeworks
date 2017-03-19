#version 440

layout(location = 1) uniform mat4 Q;
layout(location = 2, binding = 0) uniform sampler2D backfaces;
layout(location = 3) uniform int scene;
//layout(location = 2, binding = 0) uniform sampler2D frontfaces;

in vec3 vpos;

out vec4 fragcolor;   

//function declarations
vec4 raytracedcolor(vec3 rayStart, vec3 rayStop);
vec4 lighting(vec3 pos, vec3 rayDir);
float distToShape(vec3 pos);
vec3 normal(vec3 pos);

void main(void) {
	//uncomment to show backface texture
	//fragcolor = texelFetch(backfaces, ivec2(gl_FragCoord.xy), 0);
	//return;

	vec3 rayStart = vpos;

	vec3 rayStop = texelFetch(backfaces, ivec2(gl_FragCoord.xy), 0).xyz;
	fragcolor = raytracedcolor(rayStart, rayStop);

	if (fragcolor.a == 0.0) {
		discard;
	}
	//Gamma corrected output
	fragcolor = pow(fragcolor, vec4(0.45, 0.45, 0.45, 1.0));
}

// trace rays until they intersect the surface
vec4 raytracedcolor(vec3 rayStart, vec3 rayStop) {
	
	vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
	float scale = 1.0 / Q[0].x;
	int MaxSamples = 1000;

	vec3 rayDir = normalize(rayStop - rayStart);
	float travel = distance(rayStop, rayStart);
	float stepSize = travel / MaxSamples;
	vec3 pos = rayStart;
	vec3 rayStep = rayDir * stepSize;
	
	for (int i=0; i < MaxSamples && travel > 0.0; ++i, pos += rayStep, travel -= stepSize)	{
		float dist = distToShape(pos);

		stepSize = dist;
		rayStep = rayDir * stepSize;
		
		if(dist <= 0.001 / scale) {
			color = lighting(pos, rayDir);
			return color;
		}	
	}
	
	return color;
}

//compute lighting on the intersected surface
vec4 lighting(vec3 pos, vec3 rayDir) {
	const vec3 light = vec3(1.0 / 1.7, 1.0 / 1.7, 1.0 / 1.7);
	const vec4 ambient_color = vec4(0.1, 0.2, 0.1, 1.0);
	const vec4 diffuse_color = vec4(0.5, 0.7, 0.5, 1.0);
	const vec4 spec_color = vec4(0.6, 0.6, 0.6, 1.0);
	vec3 n = normal(pos);
	vec3 v = -rayDir;
	vec3 r = reflect(-light, n);

	return ambient_color + diffuse_color * max(0.0, dot(n, light)) + spec_color * pow(max(0.0, dot(r, v)), 50.0);
}

//shape function declarations
float sdSphere(vec3 position, float radius);
float sdBox(vec3 position, vec3 boxSize);
float udRoundBox(vec3 position, vec3 boxSize, float radius);
float sdRoundBox(vec3 position, vec3 boxSize, float radius);

float sdTorus(vec3 position, vec2 radius);
float sdTorus22(vec3 position, vec2 radius);
float sdTorus88(vec3 position, vec2 radius);
float sdTorus82(vec3 position, vec2 radius);
float figure3(vec3 position);

//Norms different than the Euclidean
float length2(vec3 v);
float length2(vec2 v);
float length8(vec3 v);
float length8(vec2 v);

//CGS Operations
float operationUnion(float shape1, float shape2);
//Substract shaphe2 from shape1, so the operation is shape1 minus shape2
float operationSubstraction(float shape1, float shape2);
float operationIntersection(float shape1, float shape2);

// For more distance functions see
// http://iquilezles.org/www/articles/distfunctions/distfunctions.htm

// Soft shadows
// http://www.iquilezles.org/www/articles/rmshadows/rmshadows.htm

// WebGL example and a simple ambient occlusion approximation
// https://www.shadertoy.com/view/Xds3zN


//distance to the shape we are drawing
float distToShape(vec3 pos) {
	float dis = 0.0;
	if (scene == 0) {
		float r = 0.4;
		dis = sdSphere(pos, r);
	} else if (scene == 1) {
		//vec2 r = vec2(0.3, 0.15);
		//dis = sdTorus(pos, r);
		vec2 r = vec2(0.2, 0.1);
		dis = sdTorus82(pos, r);
	} else {
		dis = figure3(pos);
	}
	
	return dis;
}
//Radius are the inner and outter radious
//radius.x is the outer radius (the donut)
//radius.y is the inner radius (the tube)
float sdTorus(vec3 position, vec2 radius) {
  vec2 q = vec2(length(position.xz) - radius.x, position.y);
  return length(q) - radius.y;
}

//box size is the size of the box
float sdBox(vec3 position, vec3 boxSize) {
  vec3 d = abs(position) - boxSize;
  return min(max(d.x, max(d.y, d.z)), 0.0) + length(max(d, 0.0));
}
//box size are the sizes of the box
//Radius is the radious of the spehere that round the edges
float udRoundBox(vec3 position, vec3 boxSize, float radius) {
	return length(max(abs(position) - boxSize, 0.0)) - radius;
}

//box size are the sizes of the box
//Radius is the radious of the spehere that round the edges
float sdRoundBox(vec3 position, vec3 boxSize, float radius) {
	vec3 q = abs(position) - boxSize;
    vec2 m = vec2( min(q.x,q.y), max(q.x,q.y) );
    float d = (m.x > 0.0) ? length(q) : m.y; 
    return d - radius;
}

//Since 88 is squared both direction
//Radius are the inner and outter radious
//radius.x is the outer radius (the donut)
//radius.y is the inner radius (the tube)
float sdTorus88(vec3 position, vec2 radius) {
  vec2 q = vec2(length8(position.xz) - radius.x, position.y);
  return length8(q) - radius.y;
}

//Since 2, is rounded the donut
//Since 8, the tube is squared (round edges)
//Radius are the inner and outter radious
//radius.x is the outer radius (the donut)
//radius.y is the inner radius (the tube)
float sdTorus82(vec3 position, vec2 radius) {
  vec2 q = vec2(length2(position.xz) - radius.x, position.y);
  return length8(q) - radius.y;
}

//Since 22 it has a squared shape
//Radius are the inner and outter radious
//radius.x is the outer radius (the donut)
//radius.y is the inner radius (the tube)
float sdTorus22(vec3 position, vec2 radius) {
  vec2 q = vec2(length2(position.xz) - radius.x, position.y);
  return length2(q) - radius.y;
}

float figure3(vec3 position) {
	vec3 sizes = vec3(0.3, 0.3, 0.3);
	float radius = 0.35;
	float box = sdBox(position, sizes);
	//float box = udRoundBox(position, sizes, 0.05);
	float sphere = sdSphere(position, radius);
	//return operationUnion(box, sphere);
	//return operationIntersection(box, sphere);
	return operationSubstraction(box, sphere);
}

//normal vector of the shape we are drawing
vec3 normal(vec3 pos) {
	const float h = 0.001;
	const vec3 Xh = vec3(h, 0.0, 0.0);	
	const vec3 Yh = vec3(0.0, h, 0.0);	
	const vec3 Zh = vec3(0.0, 0.0, h);	

	return normalize(vec3(
		distToShape(pos + Xh) - distToShape(pos - Xh),
		distToShape(pos + Yh) - distToShape(pos - Yh),
		distToShape(pos + Zh) - distToShape(pos - Zh)
	));
}

// shape function definitions
float sdSphere(vec3 position, float radius) {
	return length(position) - radius;
}

float length2(vec3 v) {
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

float length2(vec2 v) {
	return v.x * v.x + v.y * v.y;
}

float length8(vec3 v) {
	v = v * v;
	v = v * v;
	v = v * v;
	return pow(v.x + v.y + v.z, 1.0 / 8.0);
}

float length8(vec2 v) {
	v = v * v;
	v = v * v;
	v = v * v;
	return pow(v.x + v.y, 1.0 / 8.0);
}

float operationUnion(float shape1, float shape2) {
	return min(shape1, shape2);
}

//Substract shaphe2 from shape1, so the operation is shape1 minus shape2
float operationSubstraction(float shape1, float shape2) {
	return max(shape1, -shape2);
}

float operationIntersection(float shape1, float shape2) {
	return max(shape1, shape2);
}
