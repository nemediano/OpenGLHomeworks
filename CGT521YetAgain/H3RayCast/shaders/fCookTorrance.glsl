#version 440

layout(location = 1) uniform mat4 Q;
layout(location = 2, binding = 0) uniform sampler2D backfaces;
layout(location = 3) uniform int scene;
//layout(location = 2, binding = 0) uniform sampler2D frontfaces;

struct Material {
	vec3 Ka;
	vec3 Ks;
	vec3 Kd;
	float m;
	float eta;
};

struct Light {
	vec3 La;
	vec3 Ls;
	vec3 Ld;
};

uniform Material mat;
uniform Light light;
uniform int option;

in vec3 vpos;

out vec4 fragcolor;   

//function declarations
vec4 raytracedcolor(vec3 rayStart, vec3 rayStop);
vec4 lighting(vec3 pos, vec3 rayDir);
float distToShape(vec3 pos);
vec3 calculateNormal(vec3 pos);

//Helpers for Cook Torrance
float geometric_attenuation(vec3 n, vec3 h, vec3 v, vec3 l);
float roughness_term(vec3 n, vec3 h, float m);
float fresnel_term(vec3 h, vec3 v, float eta);
float fresnel_term_fast(vec3 n, vec3 v, float eta);
float fresnel_term_2(vec3 n, vec3 v, float eta);

const float EPSILON = 0.0000001;

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
	const vec3 lightPosition = vec3(1.0 / 1.7, 1.0 / 1.7, 1.0 / 1.7);
		
	vec3 n = calculateNormal(pos);
	vec3 v = -rayDir;
	vec3 l = normalize(lightPosition);
	vec3 r = normalize(reflect(-l, n));
	vec3 h = normalize(l + v);
	
	vec3 ambient_color = mat.Ka * light.La;
	vec3 diffuse_color = mat.Kd * light.Ld * max(0.0, dot(n, lightPosition));
	
	float m = mat.m;
	float eta = mat.eta;
	
	float F = fresnel_term(h, v, eta);
	//float F = fresnel_term_fast(n, v, eta);
	//float F = fresnel_term_2(n, v, eta);
	float D = roughness_term(n, h, m);
	float G = geometric_attenuation(n, h, v, l);
	
	float n_dot_l = dot(n, l);
	float n_dot_v = max(0.0, dot(n, v));
	vec3 speculr_color;

	if (option == 0) {
		speculr_color = mat.Ks * light.Ls * max(0.0, (F * D * G) / (4.0 * n_dot_l * n_dot_v));
	} else if (option == 1) {
		speculr_color = vec3(1.0) * max(0.0, F / (4.0 * n_dot_l * n_dot_v));
		ambient_color = vec3(0.0);
		diffuse_color = vec3(0.0);
	} else if (option == 2) {
		speculr_color = vec3(1.0) * max(0.0, D / (4.0 * n_dot_l * n_dot_v));
		ambient_color = vec3(0.0);
		diffuse_color = vec3(0.0);
	} else {
		speculr_color = vec3(1.0) * max(0.0, G / (4.0 * n_dot_l * n_dot_v));
		ambient_color = vec3(0.0);
		diffuse_color = vec3(0.0);
	}
	
	
	return vec4(ambient_color + diffuse_color + speculr_color, 1.0);
}

float geometric_attenuation(vec3 n, vec3 h, vec3 v, vec3 l) {
	
	float n_dot_h = max(0.0, dot(n, h));
	float v_dot_h = max(EPSILON, dot(v, h));
	float n_dot_v = max(0.0, dot(n, v));
	float n_dot_l = dot(n, l);
	
	float masking = 2.0f * n_dot_h * n_dot_v / v_dot_h;
	float shadowing = 2.0f * n_dot_h * n_dot_l / v_dot_h;
	
	return min(1.0f, min(masking, shadowing));
}

float roughness_term(vec3 n, vec3 h, float m) {
	float n_dot_h_sq = dot(n, h) * dot(n, h);
	float tan_sq = (1.0f - n_dot_h_sq) / (n_dot_h_sq);
	float m_sq = m * m;
	
	return exp(-1.0f * tan_sq / (m_sq)) / (3.1416f * m_sq * n_dot_h_sq * n_dot_h_sq);
}

float fresnel_term_fast(vec3 n, vec3 v, float eta) {
	float one_minus_n_dot_v_5th = pow(1.0f - dot(n, v), 5.0);
	float f_lambda = ((1.0f - eta) / (1.0f + eta)) * ((1.0f - eta) / (1.0f + eta));
	
	return f_lambda + (1.0f - f_lambda) * one_minus_n_dot_v_5th;
}

float fresnel_term(vec3 h, vec3 v, float eta) {
	float c = dot(v, h);
	float g = sqrt(eta * eta + c * c - 1.0);
	
	float g_plus_c = g + c;
	float g_minus_c = g - c;
	
	float left_factor  =  (g_minus_c * g_minus_c) / (2.0 * g_plus_c * g_plus_c);
	float right_factor = (1.0 + pow(c * g_plus_c - 1.0, 2.0) / pow(c * g_minus_c - 1.0, 2.0));
	
	return left_factor * right_factor;
}

float fresnel_term_2(vec3 n, vec3 v, float eta) {
	return pow(1.0 + dot(n, v), eta);
}

//normal vector of the shape we are drawing
vec3 calculateNormal(vec3 pos) {
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

const float TAU = 6.28318;

//shape function declarations
float sdSphere(vec3 position, float radius);
float sdBox(vec3 position, vec3 boxSize);
float sdEllipsoid(vec3 position, vec3 axys);
float udRoundBox(vec3 position, vec3 boxSize, float radius);

float sdTorus(vec3 position, vec2 radius);
float sdTorus22(vec3 position, vec2 radius);
float sdTorus88(vec3 position, vec2 radius);
float sdTorus82(vec3 position, vec2 radius);
float figure2(vec3 position);
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
		//dis = sdEllipsoid(pos, vec3(0.4, 0.2, 0.1));
	} else if (scene == 1) {
		dis = figure2(pos);
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

//Axys are the lenght of the principal axys of the elipsoid
float sdEllipsoid(vec3 position, vec3 axys) {
    return (length(position / axys) - 1.0) * min(min(axys.x, axys.y), axys.z);
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

float figure2(vec3 position) {
	//Apply optional twist
	float c = cos(4.0 * position.y);
    float s = sin(5.0 * position.y);
    mat2  m = mat2(c, -s, s, c);
    vec3  q = vec3(m * position.xz, position.y);
	float scaleFactor = 0.6;
	vec2 r = vec2(0.2, 0.1);
	return sdTorus82(q / scaleFactor, r) * scaleFactor;
}

// shape function definitions
float sdSphere(vec3 position, float radius) {
	return length(position) - radius;
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