#version 430

//Light container
struct Light {
	vec3 direction;
	vec3 color;
	float ratio;
	float intensity;
};

//Material container
struct Material {
	float metalicity;
	float roughness;
	vec3 base_color;
	vec3 F0;
};

#ifndef PI
#define PI 3.14159
#endif

#ifndef EPSILON
#define EPSILON 0.001
#endif

layout (location = 0) out vec4 fragcolor;

uniform Material mat;
uniform Light dir;
uniform vec3 cameraPos;

//Inputs from vertex shader is in world space
in vec3 fNormal;
in vec3 fPosition;


//Actual lighting calculation similars to Cook Torrance
vec3 fresnelTerm(float VdH, vec3 F0);
float geometryTerm(float roughness, vec3 V, vec3 N, vec3 H);
float distributionTerm(float NdH, float alpha2);

//The real shading model
vec3 shade(vec3 L, float lightRadius, vec3 V, vec3 N, Material material);
//Abreviated version of the model
vec3 shade(vec3 L, vec3 V, vec3 N, Material material);

//Helper in calculation
float safeDot(vec3 u, vec3 v);

void main(void) {
	vec3 position = fPosition;
	vec3 normal   = normalize(fNormal);
	
	vec3 accumulated_light = 0.2 * mat.base_color;	  
	   
	vec3 N = normalize(normal);
	//View vector, V = eye - position
	vec3 V = normalize(cameraPos - position);
	//Directional light
	vec3 L = normalize(-dir.direction);
	  
	accumulated_light += dir.intensity * (shade(L, dir.ratio, V, N, mat) * dir.color);
	
	fragcolor = vec4(min(accumulated_light, vec3(1.0)), 1.0);
}

//Disney's lighting shader
vec3 shade(vec3 L, float lightRadius, vec3 V, vec3 N, Material material) {
  float alpha2 = material.roughness * material.roughness;
  //If no lightRadious alpha2 becomes alpha^3 (Is this correct?)
  alpha2 *= alpha2 + lightRadius;

  vec3 H = normalize(V+L);

  float VdH = safeDot(V, H);
  float NdH = safeDot(N, H);
  float NdL = safeDot(N, L);
  float NdV = safeDot(N, V);
  //Microfacet distributions term
  float D = distributionTerm(NdH, alpha2);
  //Geometric attenuation is split in two equal calculations
  float Gl = geometryTerm(material.roughness, L, N, H);
  float Gv = geometryTerm(material.roughness, V, N, H);
  vec3 F = fresnelTerm(VdH, material.F0);
  //Use Cook Trrance for specular contribution
  vec3 specular_contribution = F * (Gl * Gv * D / (4.0 * NdL * NdV + 0.000001));

  // metals don't have a diffuse contribution, so turn off the diffuse color
  // when the material is metallic
  vec3 diffuse_color = material.base_color * (1.0 - material.metalicity) / PI;

  // use reflectance to calculate energy conservation
  // diffuse_color *= vec3(1.0, 1.0, 1.0) - F0;

  return (specular_contribution + diffuse_color) * NdL;
}

//No light radious (Directional light?)
vec3 shade(vec3 L, vec3 V, vec3 N, Material material) {
  return shade(L, 0.0, V, N, material);
}

// Schlick approximation for the Fresnell term using a 
// spherical gaussian approximation. 
// F0 is the specular reflectance at normal incidence.
vec3 fresnelTerm(float VdH, vec3 F0) {
  return F0 + (vec3(1.0, 1.0, 1.0) - F0) * pow(2.0, (-5.55473 * VdH - 6.98316) * VdH);
}

float geometryTerm(float roughness, vec3 V, vec3 N, vec3 H) {
  // remapped roughness, to be used for the geometry term calculations,
  // per Disney [16], Unreal [3]. N.B. don't do this in IBL
  float roughness_remapped = 0.5 + roughness / 2.0;

  float NdV = max(0.0, dot(N,V));
  
  float k = pow(roughness_remapped + 1.0, 2.0) / 8.0;
  return NdV / ((NdV) * (1.0 - k) + k);
}

float distributionTerm(float NdH, float alpha2) {
  float D_denominator = ((NdH * NdH)*(alpha2 - 1.0) + 1.0);
  return alpha2 / (PI * D_denominator * D_denominator + 0.0000001);
}

float safeDot(vec3 u, vec3 v) {
  return max(dot(u,v), 0.0);
}
