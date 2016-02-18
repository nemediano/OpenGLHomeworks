#ifndef OPENGL_PROGRAM_H_
#define OPENGL_PROGRAM_H_

#include <GL/gl.h>
#include <string>
using namespace std;

namespace opengl {
class OpenGLProgram {
	public:

		OpenGLProgram(string vertex_shader_file_name, string fragment_shader_file_name);
		~OpenGLProgram();

		void use_program();

		GLint get_uniform_location(string variable);
		GLint get_attrib_location(string variable);
		GLint get_subroutine_uniform_location(GLenum shadertype, string variable);
		GLuint get_subroutine_index_location(GLenum shadertype, string variable);
		GLuint get_program_id();

		bool is_ok();

	private:
		string m_vertex_shader_source;
		string m_fragment_shader_source;

		bool m_program_ok;

		OpenGLProgram();

		GLuint m_vertex_shader;
		GLuint m_fragment_shader;
		GLuint m_program;

		bool validate_shader_type(const GLenum& shadertype);
};

}

#endif