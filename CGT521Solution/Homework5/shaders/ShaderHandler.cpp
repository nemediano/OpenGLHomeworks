#include "ShaderHandler.h"
#include "../GLExt.h"
#include <iostream>
#include <fstream>
namespace shaders {

	ShaderHandler::ShaderHandler(string vertex_shader_file_name, string fragment_shader_file_name) {
		/* Cool preprocesing */
		ifstream input_vertex_shader_file(vertex_shader_file_name, ios::in | ios::binary);
		if (input_vertex_shader_file) {
			input_vertex_shader_file.seekg(0, ios::end);
			vertex_shader_source.resize(input_vertex_shader_file.tellg());
			input_vertex_shader_file.seekg(0, ios::beg);
			input_vertex_shader_file.read(&vertex_shader_source[0], vertex_shader_source.size());
			input_vertex_shader_file.close();
		} else {
			cerr << "Could not open vertex shader file!!" << endl;
		}

		ifstream input_fragment_shader_file(fragment_shader_file_name, ios::in | ios::binary);
		if (input_fragment_shader_file) {
			input_fragment_shader_file.seekg(0, ios::end);
			fragment_shader_source.resize(input_fragment_shader_file.tellg());
			input_fragment_shader_file.seekg(0, ios::beg);
			input_fragment_shader_file.read(&fragment_shader_source[0], fragment_shader_source.size());
			input_fragment_shader_file.close();
		} else {
			cerr << "Could not open fragment shader file!!" << endl;
		}

		/* or not */
		m_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		const char* start = &vertex_shader_source[0];
		glShaderSource(m_vertex_shader, 1, &start, NULL);

		m_fragment_shader= glCreateShader(GL_FRAGMENT_SHADER);
		start = &fragment_shader_source[0];
		glShaderSource(m_fragment_shader, 1, &start, NULL);

		try {
			int status;
			glCompileShader(m_vertex_shader);
			glGetShaderiv(m_vertex_shader, GL_COMPILE_STATUS, &status); 

			if(status == GL_FALSE) {
				throw m_vertex_shader;	
			}

			glCompileShader(m_fragment_shader);
			glGetShaderiv(m_fragment_shader, GL_COMPILE_STATUS, &status); 
	
			if(status == GL_FALSE) {
				throw m_fragment_shader;	
			}
		} catch(GLuint bad_shader) {
			printShaderInfoLog(bad_shader);
		}

		m_program = glCreateProgram();
		try {
			int status;
			glAttachShader(m_program, m_vertex_shader);
			glAttachShader(m_program, m_fragment_shader);
			glLinkProgram(m_program);

			glGetProgramiv(m_program, GL_LINK_STATUS, &status);

			if(status == GL_FALSE) {
				throw m_program;	
			}

		} catch(GLuint bad_program) {
			printProgramInfoLog(bad_program);
		}

	}
	
	ShaderHandler::~ShaderHandler() {
		glDetachShader(m_program, m_vertex_shader); 
		glDetachShader(m_program, m_fragment_shader);

		glDeleteShader(m_vertex_shader); 
		glDeleteShader(m_fragment_shader); 
		glDeleteProgram(m_program); 
	}

	void ShaderHandler::use_program() {
		glUseProgram(m_program);
	}

	GLint ShaderHandler::get_uniform_location(string variable) {
		return glGetUniformLocation(m_program, variable.c_str());
	}

	GLint ShaderHandler::get_attrib_location(string variable) {
		return glGetAttribLocation(m_program, variable.c_str());
	}
    
	void printShaderInfoLog(GLuint obj) {
		int infologLength = 0;
		int charsWritten  = 0;
		char *infoLog;

		glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

		if (infologLength > 0) {
			infoLog = new char[infologLength];
			glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog); 
			cout << infoLog;
			delete [] infoLog;
		}
	}

    void printProgramInfoLog(GLuint obj) {
		int infologLength = 0;
		int charsWritten  = 0;
		char *infoLog;

		glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

		if (infologLength > 0) {
			infoLog = new char[infologLength];
			glGetProgramInfoLog(obj, infologLength, &charsWritten, &infoLog[0]);
			cout << infoLog;
			delete [] infoLog;
		}
	}
	
};
