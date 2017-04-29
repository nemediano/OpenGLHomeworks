#include "stdafx.h"
#include "FrameBufferCapture.h"

namespace image {
	
	void captureColor(GLuint texture_id, const char* fileName, GLenum format) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, texture_id);
		int width;
		int height;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
		
		const int bytesPerPixel = 3;
		GLubyte* buffer = new GLubyte[(width * height * bytesPerPixel)];
		glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, buffer);

		FIBITMAP* image = FreeImage_ConvertFromRawBits(buffer, width, height, (3 * width), 24, 0x0000FF, 0xFF0000, 0x00FF00, false);
		FreeImage_Save(FIF_PNG, image, fileName, 0);

		FreeImage_Unload(image);
		delete buffer;
	}

	void captureDepth(GLuint depth_buffer_id, const char* fileName, GLenum precision) {
		//Unbound any framebuffer so we can read form it (just in case)
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, depth_buffer_id);
		int width;
		int height;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

		if (precision == GL_DEPTH_COMPONENT16) {
			GLushort* pixels = new GLushort[width * height];
			//glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, static_cast<GLvoid*>(pixels));
			glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, static_cast<GLvoid*>(pixels));
			FIBITMAP* image = FreeImage_ConvertFromRawBits((unsigned char*)(pixels), width, height, (4 * width), 16, 0xFFFF, 0x0000, 0x0000, false);
			FreeImage_Save(FIF_PNG, image, fileName, 0);
			FreeImage_Unload(image);
			delete[] pixels;
		} else if (precision == GL_DEPTH_COMPONENT24) {
			GLuint* pixels = new GLuint[width * height];    //There is no 24 bit variable, so we'll have to settle for 32 bit
			//glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT_24_8, static_cast<GLvoid*>(pixels));  //No upconversion.
			glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT_24_8, static_cast<GLvoid*>(pixels));
			FIBITMAP* image = FreeImage_ConvertFromRawBits((unsigned char*)(pixels), width, height, (4 * width), 24, 0xFFFFFF, 0x000000, 0x000000, false);
			FreeImage_Save(FIF_PNG, image, fileName, 0);
			FreeImage_Unload(image);
			delete[] pixels;
		} else if (precision == GL_DEPTH_COMPONENT32 || precision == GL_DEPTH_COMPONENT32F)	{
			GLuint* pixels = new GLuint[width * height];
			glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, static_cast<GLvoid*>(pixels));
			//glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, static_cast<GLvoid*>(pixels));

			GLuint max = 0;
			GLuint min = 0;
			double sum = 0.0;
			min = max = pixels[0];
			for (size_t i = 1; i < width * height; ++i) {
				GLuint pixel = pixels[i];
				if (pixel < min) {
					min = pixel;
				}
				if (pixel > max) {
					max = pixel;
				}
				sum += pixel;
			}
			double avg = sum / (width * height);

			FIBITMAP* image = FreeImage_ConvertFromRawBits((unsigned char*)(pixels), width, height, width * sizeof(GLuint), 32, 0xFFFFFFFF, 0x00000000, 0x00000000, false);
			FreeImage_Save(FIF_PNG, image, fileName, 0);
			FreeImage_Unload(image);
			delete[] pixels;
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}
}