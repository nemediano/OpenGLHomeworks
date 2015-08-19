#version 150
uniform int texture_map_flag;
uniform int light_mode;
uniform vec4 light_position_in_vs;
uniform vec4 light_direction_in_vs;
uniform vec4 view;      //view direction
uniform sampler2D texture_map;

//From vertex shader
in vec2 text_coord;
in vec3 fragment_normal;
in vec3 position_viewspace;

//Mathematical constants
const float M_PI = 3.1415926535f;
const float epsilon = 1e-7;              //Small bias for the division

//Lighting model settings
vec3 kd = vec3( 1.0, 0.84, 0.0);         //diffuse material color
vec3 ks = vec3( 0.9, 0.9,  0.9);         //specular material color

const vec3 Ld = vec3(0.5, 0.5, 0.5);   //diffuse light color
const vec3 Ls = vec3(0.7, 0.7, 0.7);   //specular light color

//Parameters of the lighting model
const float index_of_refraction = 6.16; // I believe is [0, 1]
const float average_slope = 0.03;
const float metalic = 4.0; // 1.0 or 4.0
//Output to render
out vec4 frag_color;

float fresnel_term(vec3 h, vec3 v, float eta);
float roughness_term(vec3 n, vec3 h, float m);
float geometrical_attenuation(vec3 n, vec3 h, vec3 v, vec3 l);

void main(void) {
   vec4 material_color;
   if (texture_map_flag != 0) {
      material_color = texture(texture_map, text_coord);
	  kd = vec3(material_color.r, material_color.g, material_color.b);
   }

  //Make sure all are unit vectors
   vec3 n = normalize(fragment_normal);
   vec3 l;
   vec3 v;

   if (light_mode == 1) {//Assuming directional light
      //We want a vector point to light source so that is -light_direction
      l = normalize(-light_direction_in_vs.xyz);
	  //Since we have directional light we the camera is also directional
	  v = normalize(-view.xyz);
   } else { //Assuming punctual positional light
      l = normalize(light_position_in_vs.xyz - position_viewspace);
	  //Since we are in view space the eye is at the origin so v = eye - p = -p
	  v = normalize(-position_viewspace);
   }

   //Calculate half vector
   //vec3 h = normalize(0.5 * (light + view));
   //Since 1/2 is just scalar we can optimize
   vec3 h = normalize(l + v);

   //Fresnel Term
   float F = fresnel_term(h, v, index_of_refraction);
   F = 1.0f;
   //Roughness term
   float D = roughness_term(n, h, average_slope);
   //D = 1.0f;
   //Geometric term
   float G = geometrical_attenuation(n, h, v, l);
   G = 1.0f;
   float numerator = F * D * G;
   float denominator = M_PI * max(0.0, dot(n, l)) * max(0.0, dot(n, v)) + epsilon; //Add small bias to avoid zero division
   
   //Evaluate the model
   vec3 specular_term = ks * Ls * numerator / denominator;
   vec3 diffuse_term = kd * Ld * max(0.0, dot(n, l));
   
   frag_color = vec4(diffuse_term + specular_term, 1.0);;
}

float fresnel_term(vec3 h, vec3 v, float eta) {
   float c = max(0.0, dot(v, h));
   float g = sqrt(eta * eta + c * c - 1);
   float g_minus_c = g - c;
   float g_plus_c = g + c;
   float left = (g_minus_c * g_minus_c) / (2.0 * g_plus_c * g_plus_c + epsilon);
   float right = 1 + (c * g_plus_c - 1) *  (c * g_plus_c - 1) / ((c * g_minus_c + 1) * (c * g_minus_c + 1) + epsilon);
   float F = left * right;
   return F;
}

float roughness_term(vec3 n, vec3 h, float m) {
   float n_dot_h = max(0.0, dot(n, h));
   float exponent_argument = ((1 - n_dot_h * n_dot_h) / (m * n_dot_h * n_dot_h + epsilon)) * ((1 - n_dot_h * n_dot_h) / (m * n_dot_h * n_dot_h + epsilon));
   float D = exp(- exponent_argument) / (metalic * m * m * n_dot_h * n_dot_h * n_dot_h * n_dot_h + epsilon);
   return D;
}

float geometrical_attenuation(vec3 n, vec3 h, vec3 v, vec3 l) {
   float n_dot_h = max(0.0, dot(n, h));
   float n_dot_v = max(0.0, dot(n, v));
   float v_dot_h = max(0.0, dot(v, h));
   float n_dot_l = max(0.0, dot(n, l));

   float masking = (2.0 * n_dot_h * n_dot_v) / (v_dot_h + epsilon);
   float shadowing = (2.0 * n_dot_h * n_dot_l) / (v_dot_h + epsilon);
   return min(1.0, min(masking, shadowing));
}

