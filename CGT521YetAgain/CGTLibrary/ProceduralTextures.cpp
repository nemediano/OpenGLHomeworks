#include "stdafx.h"
#include "ProceduralTextures.h"

namespace image {



	Texture defaultStencil(float cutoff, unsigned int size) {

		using glm::vec2;
		using glm::vec3;
		using glm::vec4;

		Texture t;
		t.m_height = t.m_width = size;
		const int bytesPerPixel = 4;
		t.m_data.resize(t.m_height * t.m_width * bytesPerPixel);

		for (int i = 0; i < t.m_height; i++) {
			for (int j = 0; j < t.m_width; j++) {
				vec2 coordinates = (1.0f / size) * vec2(i, j) + (0.5f / size);
				float distanceNormalized = 2.0f * glm::distance(coordinates, glm::vec2(0.5f));
				float greyLevel = 1.0f;
				if (distanceNormalized < cutoff) {
					greyLevel = 1.0f;
				}  else if (distanceNormalized < 1.0f) {
					float s = (distanceNormalized - cutoff) / (1.0f - cutoff);
					greyLevel = 1.0f - s;
				} else {					greyLevel = 0.0f;				}
				/*vec4 pixel = vec4(vec3(greyLevel), 1.0f);
				unsigned char red = static_cast<unsigned char>(255.0f * pixel.r);
				unsigned char green = static_cast<unsigned char>(255.0f * pixel.g);
				unsigned char blue = static_cast<unsigned char>(255.0f * pixel.b);
				unsigned char alpha = static_cast<unsigned char>(255.0f * pixel.a);
				t.m_data[(i * t.m_width + j) * bytesPerPixel + 0] = red;
				t.m_data[(i * t.m_width + j) * bytesPerPixel + 1] = green;
				t.m_data[(i * t.m_width + j) * bytesPerPixel + 2] = blue;
				t.m_data[(i * t.m_width + j) * bytesPerPixel + 3] = alpha;*/
				unsigned char grey = static_cast<unsigned char>(greyLevel * 255.0f);
				t.m_data[(i * t.m_width + j) * bytesPerPixel + 0] = grey;
				t.m_data[(i * t.m_width + j) * bytesPerPixel + 1] = grey;
				t.m_data[(i * t.m_width + j) * bytesPerPixel + 2] = grey;
				t.m_data[(i * t.m_width + j) * bytesPerPixel + 3] = 255;
			}
		}


		return t;
	}

	Texture chessBoard(unsigned int size, unsigned int cells, glm::vec3 black, glm::vec3 white) {
		using glm::vec2;
		using glm::vec3;
		using glm::vec4;

		Texture t;
		t.m_height = t.m_width = size;
		const int bytesPerPixel = 4;
		t.m_data.resize(t.m_height * t.m_width * bytesPerPixel);

		for (int i = 0; i < t.m_height; i++) {
			for (int j = 0; j < t.m_width; j++) {
				vec2 coordinates = (1.0f / size) * vec2(i, j) + (0.5f / size);
				bool flag;
				vec3 pixel = flag ? white : black;
				unsigned char red = static_cast<unsigned char>(255.0f * pixel.r);
				unsigned char green = static_cast<unsigned char>(255.0f * pixel.g);
				unsigned char blue = static_cast<unsigned char>(255.0f * pixel.b);
				t.m_data[(i * t.m_width + j) * bytesPerPixel + 0] = red;
				t.m_data[(i * t.m_width + j) * bytesPerPixel + 1] = green;
				t.m_data[(i * t.m_width + j) * bytesPerPixel + 2] = blue;
				t.m_data[(i * t.m_width + j) * bytesPerPixel + 3] = 255;
			}
		}


		return t;
	}
}