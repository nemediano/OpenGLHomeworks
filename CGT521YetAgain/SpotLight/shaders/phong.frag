#version 430
in vec3 fNormal;
in vec2 fTextCoord;
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

out vec4 fragcolor;

vec3 applyGamma(vec3 color);
vec3 phongShading();

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
	//Eliminate light space backprojection
	if (light_space_normal.z <= 0.0) {
		color = vec3(0.5);
	} else if (light_space_pos.z <= 0.0) {
		// clipped in z, behind light
		color = vec3(1.0, 0.0, 0.0);
	} else if (light_space_pos.x > 1.0 || light_space_pos.x < -1.0 || light_space_pos.y > 1.0 || light_space_pos.y < -1.0) {
		// outside of projection frustrum (out black)
		color = vec3(1.0, 1.0, 0.0);
	} else {
	  //Extra condition in the shape of light
	  vec2 uv = light_space_pos.xy;
	  float dis = light_space_pos.z;
	  color = vec3(0.0);
	  //Shape of the spotlight in the form of a circle
	  if (length(uv) < 1.0) {
		//We fullfill all condition to light this fragment with this light
		color += phongShading();
	  }
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