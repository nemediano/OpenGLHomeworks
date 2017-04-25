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
uniform float spread = 700.0;

out vec4 fragcolor;

vec3 applyGamma(vec3 color);
vec3 phongShading();

vec2 poissonDisk[32] = vec2[](
	vec2(-0.436676, -0.16539f),
	vec2(-0.1457429, 0.2641475),
	vec2(-0.8007998, -0.5128862),
	vec2(-0.8259562, -0.09363466),
	vec2(-0.1242217, -0.009824245),
	vec2(-0.5341815, -0.6150366),
	vec2(-0.0916272, -0.3877475),
	vec2(-0.5404648, 0.3534201),
	vec2(-0.4193692, -0.8930702),
	vec2(-0.06114891, -0.8964248),
	vec2(-0.1000517, 0.6349613),
	vec2(0.1739175, 0.136365),
	vec2(0.1245896, 0.4622511),
	vec2(-0.4278838, 0.6153105),
	vec2(0.2686949, -0.1171129),
	vec2(0.4432742, 0.1803804),
	vec2(0.3864222, 0.590681),
	vec2(0.5442576, -0.1798256),
	vec2(0.7076544, 0.09673079),
	vec2(-0.8854074, 0.248059),
	vec2(0.7231508, 0.5907731),
	vec2(0.9244204, 0.3066544),
	vec2(-0.6209617, 0.09009076),
	vec2(0.9113371, -0.1602885),
	vec2(0.3532589, 0.8606513),
	vec2(0.7084755, -0.5066093),
	vec2(0.006417691, 0.9157037),
	vec2(0.265075, -0.6932341),
	vec2(0.239396, -0.408051),
	vec2(-0.849546, 0.5209261),
	vec2(0.5228971, -0.7789191),
	vec2(-0.3783323, 0.9108117)
);


float getShadowFactor();
float random(vec4 seed);

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
	
	vec3 shadowCoord = fShadowCoord.xyz / fShadowCoord.w;
	vec3 l = normalize(light.position - fPosition);
	vec3 n = normalize(fNormal);
	float bias = max(0.0005 * (1.0 - dot(n, l)), 0.0);
	shadowCoord.z -= bias;
	
	const int numSamples = 16; //Depends on the speed of the GPU
	for(int i = 0; i < numSamples; ++i) {
		int randomIndex = int(32.0 * random(vec4(fPosition, i))) % 32;
		shadow += texture(shadowMap, shadowCoord + vec3(poissonDisk[randomIndex] / spread, 0.0));
	}
	shadow /= numSamples;
	
	return shadow;
}

float random (vec4 seed) {
	float dot_product = dot(seed, vec4(12.9898, 78.233, 45.164, 94.673));
    return fract(sin(dot_product) * 43758.5453);
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