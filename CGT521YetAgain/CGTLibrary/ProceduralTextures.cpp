#include "stdafx.h"
#include "ProceduralTextures.h"

namespace image {



	Texture defaultStencil(unsigned int size) {

		using glm::vec2;
		using glm::vec3;
		using glm::vec4;

		Texture t;
		t.m_height = t.m_width = size;
		const int bytesPerPixel = 4;
		t.m_data.resize(t.m_height * t.m_width * bytesPerPixel);

		for (int i = 0; i < t.m_height; i++) {
			for (int j = 0; j < t.m_width; j++) {
				vec2 coordinates = (1.0f / size) * vec2(i, j) + (1 / (2.0f * size));
				float greyLevel = 1.0f - glm::min(2.0f * glm::distance(coordinates, glm::vec2(0.5f)), 1.0f);
				vec4 pixel = vec4(vec3(greyLevel), 1.0f);
				unsigned char red = static_cast<unsigned char>(255.0f * pixel.r);
				unsigned char green = static_cast<unsigned char>(255.0f * pixel.g);
				unsigned char blue = static_cast<unsigned char>(255.0f * pixel.b);
				unsigned char alpha = static_cast<unsigned char>(255.0f * pixel.a);
				t.m_data[(i * t.m_width + j) * bytesPerPixel + 0] = red;
				t.m_data[(i * t.m_width + j) * bytesPerPixel + 1] = green;
				t.m_data[(i * t.m_width + j) * bytesPerPixel + 2] = blue;
				t.m_data[(i * t.m_width + j) * bytesPerPixel + 3] = alpha;
			}
		}


		return t;
	}

	Texture chessBoard(unsigned int size, unsigned int cells) {

		return Texture();
	}
}