#version 420

// See the layout rules in the redbook. There are complicated rules for sizes and padding in uniform buffer objects.
// A good safe way to get started: make matrices mat4, make vectors vec4, and sort the uniform block members from
// largest to smallest.
layout(std140, binding = 2) uniform LightingUniforms
{
   mat4 M;		//modeling matrix
   mat4 PV;	//camera projection * view matrix

   vec4 La;	//ambient light color
   vec4 Ld;	//diffuse light color
   vec4 Ls;	//specular light color
   vec4 ka;	//ambient material color
   vec4 kd;	//diffuse material color
   vec4 ks;	//specular material color
   vec4 eye_w;	//world-space eye position
   vec4 light_w; //world-space light position

   float shininess;
   int pass;
};

uniform sampler2D texColor; //samplers cannot go into a uniform buffer object

out vec4 fragcolor;           
in vec2 tex_coord; 
in vec3 normal; 
in vec3 light; 

void main(void)
{ 
   if(pass==0) //occluders
   {
      vec3 n = normalize(normal);
      vec3 l = normalize(light);
      fragcolor = kd*abs(dot(n, l)) + ka;
      return;
   }

   if(pass==1) // bounding volumes
   {
      fragcolor = vec4(1.0, 1.0, 1.0, 1.0);
      return;
   }

   //meshes
   vec3 n = normalize(normal);
   vec3 l = normalize(light);

   fragcolor = texture(texColor, tex_coord)*(kd*abs(dot(n, l)) + ka);
   
}




















