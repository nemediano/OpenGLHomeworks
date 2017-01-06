#pragma once

namespace ogl {
	class OGLProgram {
	public:

		OGLProgram(std::string vertex_shader_file_name, std::string fragment_shader_file_name);
		~OGLProgram();

		void use();

		GLint uniformLoc(std::string variable);
		GLint attribLoc(std::string variable);
		GLint subroutineUniformLoc(GLenum shadertype, std::string variable);
		GLuint subroutineIndexLoc(GLenum shadertype, std::string variable);
		GLuint getID();

		bool isOK();

	private:
		std::string m_vertex_shader_source;
		std::string m_fragment_shader_source;

		bool m_program_ok;

		OGLProgram();

		GLuint m_vertex_shader;
		GLuint m_fragment_shader;
		GLuint m_program;

		bool validate_shader_type(const GLenum& shadertype);
	};
}