#include "stdafx.h"
#include "Cubemap.h"

namespace image {
	void CubeMap::release_location() {
		glDeleteTextures(1, &m_cubemap_id);
	}

	void CubeMap::ask_locations() {
		glGenTextures(1, &m_cubemap_id);
	}

	CubeMap::CubeMap() : m_width(0), m_height(0), m_cubemap_id(0) {

	}

	CubeMap::CubeMap(const std::vector<std::string>& input_file_names) : m_width(0), m_height(0), m_cubemap_id(0) {
		load_texture(input_file_names);
	}

	CubeMap::~CubeMap() {
		release_location();
	}

	bool CubeMap::load_texture(const std::vector<std::string>& input_file_names) {
		if (m_cubemap_id == 0) {
			ask_locations();
		}

		if (input_file_names.size() < 6) {
			return false;
		}
		
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap_id);
		for (int i = 0; i < 6; ++i) {
			FIBITMAP* tempImg = FreeImage_Load(FreeImage_GetFileType(input_file_names[i].c_str(), 0), input_file_names[i].c_str());
			if (!tempImg) {
				std::cerr << "Could not load image: " << input_file_names[i] << std::endl;
				return false;
			}
			FIBITMAP* img = FreeImage_ConvertTo32Bits(tempImg);
			if (!img) {
				std::cerr << "Image: " << input_file_names[i] << " is damaged" << std::endl;
				return false;
			}

			FreeImage_Unload(tempImg);

			m_width = FreeImage_GetWidth(img);
			m_height = FreeImage_GetHeight(img);
			GLuint scanW = FreeImage_GetPitch(img);

			std::vector<unsigned char> pixels;
			pixels.resize(m_height * scanW);
			FreeImage_ConvertToRawBits(pixels.data(), img, scanW, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, FALSE);
			FreeImage_Unload(img);

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGBA, m_width, m_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels.data());
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		return true;
	}

	void CubeMap::bind() const {
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap_id);
	}

	/*void CubeMap::send_to_gpu() {

	}*/

	int CubeMap::get_width() const {
		return m_width;
	}

	int CubeMap::get_height() const {
		return m_height;
	}

	GLuint CubeMap::get_id() const {
		return m_cubemap_id;
	}
}