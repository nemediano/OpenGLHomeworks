#version 420            

// See the layout rules in the redbook. There are complicated rules for sizes and padding in uniform buffer objects.
// A good safe way to get started: make matrices mat4, make vectors vec4, and sort the uniform block members from
// largest to smallest.
layout(std140, binding = 2) uniform LightingUniforms
{
   mat4 M;	//modeling matrix
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

in vec3 pos_attrib;
in vec2 tex_coord_attrib;
in vec3 normal_attrib; 

out vec2 tex_coord; 
out vec3 normal;
out vec3 light;

void main(void)
{
   if(pass==2)
   {

      //lots of work in the vertex shader (just an example)
      vec3 offset = vec3(0.0);
      for(int i=0; i<1000; i++)
      {
         offset += 0.00004*sin(10.0*pos_attrib.z)*normal_attrib;
      }

      gl_Position = PV*M*vec4(pos_attrib+offset, 1.0);
   }
   else
   {
	   gl_Position = PV*M*vec4(pos_attrib, 1.0);
   }
	tex_coord = tex_coord_attrib;
   normal = vec3(M*vec4(normal_attrib, 0.0));
   light = vec3(light_w - M*vec4(pos_attrib, 1.0));
}