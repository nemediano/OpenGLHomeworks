#include <iostream>
#include "FreeImage.h"
#include "Texture.h"

namespace image {
	Texture::Texture() : m_height(0), m_width(0), m_texture_id(0), m_data(nullptr) {

	}

	Texture::Texture(const std::string& input_file_name) : Texture() {
		load_texture(input_file_name);
		ask_locations();
		send_to_gpu();
	}

	Texture::~Texture() {
		delete m_data;
		release_location();
	}
	
	bool Texture::load_texture(const std::string& input_file_name) {
		
		FIBITMAP* tempImg = FreeImage_Load(FreeImage_GetFileType(input_file_name.c_str(), 0), input_file_name.c_str());
		if (!tempImg) {
			std::cerr << "Could not load image: " << input_file_name << std::endl;
			return false;
		}
		FIBITMAP* img = FreeImage_ConvertTo32Bits(tempImg);
		if (!img) {
			std::cerr << "Image: " << input_file_name << " is damaged" << std::endl;
			return false;
		}

		FreeImage_Unload(tempImg);

		m_width = FreeImage_GetWidth(img);
		m_height = FreeImage_GetHeight(img);
		GLuint scanW = FreeImage_GetPitch(img);

		m_data = new GLubyte[m_height * scanW];
		FreeImage_ConvertToRawBits(m_data, img, scanW, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, FALSE);
		FreeImage_Unload(img);

		return true;
	}

	void Texture::bind() const {
		glBindTexture(GL_TEXTURE_2D, m_texture_id);
	}

	int Texture::get_width() const {
		return static_cast<int>(m_width);
	}

	int Texture::get_height() const {
		return static_cast<int>(m_height);
	}

	void Texture::send_to_gpu() {

		if (m_texture_id == 0) {
			ask_locations();
		}

		glBindTexture(GL_TEXTURE_2D, m_texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, m_data);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture::release_location() {
		glDeleteTextures(1, &m_texture_id);
	}

	void Texture::ask_locations() {
		glGenTextures(1, &m_texture_id);
	}
}