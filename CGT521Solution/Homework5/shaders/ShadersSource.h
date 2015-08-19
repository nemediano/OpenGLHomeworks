#ifndef SHADERS_SOURCE_H_
#define SHADERS_SOURCE_H_
#include <string>


const std::string vertex_shader_src = 
	  //Vertex Shader 
	  "#version 150 \n"
      "in vec3 position_attribute; \n"
	  "in vec2 texture_coordinates_attribute; \n"
	  "out vec2 text_coord; \n"
      "void main(void){ \n"
	  "  vec4 position; \n"
	  "  position = vec4(position_attribute, 1.0);"
	  "  text_coord = texture_coordinates_attribute; \n"
      "  gl_Position = position; \n"
      "} \n";


const std::string fragment_shader_src = 		//Fragment Shader 
	  "#version 150 \n"
	  "in vec2 text_coord; \n"
	  "uniform sampler2D texture_map; \n"
      "out vec4 fragcolor; \n"
      "void main(void){ \n"
	  "   fragcolor = texture(texture_map, text_coord); \n"
      "}\n ";

#endif
