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

float getShadowFactor();

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
	if (all(greaterThan(infront, vec2(0.0))) && all(lessThan(inside, vec2(1.0)))) {
	  vec2 uv = 0.5 * light_space_pos.xy + 0.5;
	  vec3 stencil =  texture(lightStencil, uv, 0).rgb;
	  color += stencil * getShadowFactor() * phongShading();
	}

	fragcolor = vec4(applyGamma(color), 1.0);
}

float getShadowFactor() {
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	vec3 shadowCoord = fShadowCoord.xyz / fShadowCoord.w;
	vec3 l = normalize(light.position - fPosition);
	vec3 n = normalize(fNormal);
	float bias = max(0.0005 * (1.0 - dot(n, l)), 0.0);
	shadowCoord.z -= bias;
	
	const int size = 2;
	for(int x = -size; x <= size; ++x) {
		for(int y = -size; y <= size; ++y) {
			shadow += texture(shadowMap, shadowCoord + vec3(vec2(x, y) * texelSize, 0.0));
		}
	}
	shadow /= (2 * size + 1) * (2 * size + 1);
	
	return shadow;
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