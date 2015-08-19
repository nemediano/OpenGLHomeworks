#ifndef SHADER_HANDLER_H_
#define SHADER_HANDLER_H_

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <string>
using namespace std;

namespace shaders {
class ShaderHandler {
	public:

		ShaderHandler(string vertex_shader_file_name, string fragment_shader_file_name);
		~ShaderHandler();

		void use_program();

		GLint get_uniform_location(string variable);
		GLint get_attrib_location(string variable);

		string vertex_shader_source;
		string fragment_shader_source;

	private:
      
		ShaderHandler();

		GLuint m_vertex_shader;
		GLuint m_fragment_shader;
		GLuint m_program;
};

void printShaderInfoLog(GLuint object);
void printProgramInfoLog(GLuint object);
}

#endif