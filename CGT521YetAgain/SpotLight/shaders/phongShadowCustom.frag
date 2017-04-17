#version 430
in vec3 fNormal;
in vec2 fTextCoord;
in vec4 fShadowCoord;
in vec3 fPosition;

struct Light {
	vec3 La;
	vec3 Ls;
	vec3 Ld;
	vec3 position;
	mat4 PM;
	mat4 M;
};

struct Material {
	vec3 Ka;
	vec3 Ks;
	vec3 Kd;
	float alpha;
};

uniform Light light;
uniform Material mat;
uniform float gamma = 1.0;
uniform vec3 cameraPosition;
uniform sampler2D lightStencil;
uniform sampler2DShadow shadowMap;

out vec4 fragcolor;

vec3 applyGamma(vec3 color);
vec3 phongShading();
const float EPSILON = 1e-2;

void main(void) {
	vec3 Ka = mat.Ka;
	vec3 La = light.La;
	//Accumulate the ambient part of the light
	vec3 color = Ka * La;
	
	//Convert to light space to eliminate all the fragments that the light will not touch
	vec4 light_space_pos = light.PM * vec4(fPosition, 1.0);
	vec4 light_space_normal = light.M * vec4(fNormal, 0.0);
	//Perspective division
	light_space_pos = light_space_pos / light_space_pos.w;
	
	//See if we are inside of the light cone
	vec2 infront = vec2(light_space_normal.z, light_space_pos.z);
	vec2 inside = abs(light_space_pos.xy);
	if (all(greaterThanEqual(infront, vec2(0.0))) && all(lessThanEqual(inside, vec2(1.0)))) {
	  vec2 uv = clamp(0.5 * light_space_pos.xy + 0.5, vec2(EPSILON), vec2(1.0 - EPSILON));
	  vec3 stencil =  texture(lightStencil, uv, 0).rgb;
	  float shadowFactor = textureProj(shadowMap, fShadowCoord, 0);
	  color += phongShading() * stencil * shadowFactor;
	}

	fragcolor = vec4(applyGamma(color), 1.0);
}

vec3 applyGamma(vec3 color) {
	return pow(color, vec3(1.0 / gamma));
}

vec3 phongShading() {
	vec3 l = normalize(light.position - fPosition);
	vec3 n = normalize(fNormal);
	vec3 v = normalize(cameraPosition - fPosition);
	vec3 r = normalize(reflect(-l, n));
	
	vec3 Kd = mat.Kd;
	vec3 Ks = mat.Ks;
	float alpha = mat.alpha;
	
	vec3 Ld = light.Ld;
	vec3 Ls = light.Ls;
	
	vec3 diffuse_color = Kd * Ld * max(0.0, dot(n, l));
	vec3 speculr_color = Ks * Ls * pow(max(0.0, dot(r, v)), alpha);
	
	return diffuse_color + speculr_color;
}