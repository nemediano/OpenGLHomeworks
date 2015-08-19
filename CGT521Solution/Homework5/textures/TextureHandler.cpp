#include <cassert>
#include <iostream>
#include <GL/glew.h>
#include "TextureHandler.h"

using namespace std;

namespace texture {
	TextureHandler::TextureHandler(int width, int height) {
		assert(width > 0);
		assert(height > 0);
		m_width = width;
		m_height = height;
		//Create data buffer to hole the data
		m_texture_data = new unsigned char[m_width * m_height];
		//Create texture
		glGenTextures(1, &m_texture_id);
		//Temporary bind texture to set some parameters
        glBindTexture(GL_TEXTURE_2D, m_texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//Unbind texture
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	TextureHandler::~TextureHandler() {
		glDeleteTextures(1, &m_texture_id);
		if (m_texture_data) {
			delete [] m_texture_data;
			m_texture_data = nullptr;
		}
		m_width = m_height = 0;
	}
	
	int TextureHandler::get_width() {
		return m_width;
	}
	
	GLuint TextureHandler::get_id() {
		return m_texture_id;
	}
	
	int TextureHandler::get_height() {
		return m_height;
	}
	
	unsigned char* TextureHandler::get_texture() {
		return m_texture_data;
	}
}