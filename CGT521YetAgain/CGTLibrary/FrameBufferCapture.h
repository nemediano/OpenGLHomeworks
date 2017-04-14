#ifndef FRAMEBUFFER_CAPTURE_H_
#define FRAMEBUFFER_CAPTURE_H_

#include <string>
#include <GL/glew.h>
#include <GL/gl.h>

namespace image {
	
	void captureColor(GLuint texture_id, const char* fileName = nullptr, GLenum format = GL_RGBA);
	void captureDepth(GLuint depth_buffer_id, const char* fileName = nullptr, GLenum precision = GL_DEPTH_COMPONENT32);
	
}
#endif