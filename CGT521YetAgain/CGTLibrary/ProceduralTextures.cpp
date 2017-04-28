#include "stdafx.h"
#include "ProceduralTextures.h"

namespace image {
	Texture defaultStencil(unsigned int size) {
		Texture t;
		t.m_height = t.m_width = size;
		const int bytesPerPixel = 4;
		t.m_data.resize(t.m_height * t.m_width * bytesPerPixel);

		for (int j = 0; j < t.m_height; j++) {
			for (int i = 0; i < t.m_width; i++) {
				glm::vec4 pixel = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
				unsigned char red = pixel.r > 0 ? static_cast<unsigned char>(255.0f / pixel.r) : 0;
				unsigned char green = pixel.g > 0 ? static_cast<unsigned char>(255.0f / pixel.g) : 0;
				unsigned char blue = pixel.b > 0 ? static_cast<unsigned char>(255.0f / pixel.b) : 0;
				unsigned char alpha = pixel.a > 0 ? static_cast<unsigned char>(255.0f / pixel.a) : 0;
				t.m_data[j * t.m_width + i * bytesPerPixel + 0] = red;
				t.m_data[j * t.m_width + i * bytesPerPixel + 1] = green;
				t.m_data[j * t.m_width + i * bytesPerPixel + 2] = blue;
				t.m_data[j * t.m_width + i * bytesPerPixel + 3] = alpha;
			}
		}


		return t;
	}

	Texture chessBoard(unsigned int size, unsigned int cells) {

		return Texture();
	}
}