#ifndef OGL_HELPERS_H_
#define OGL_HELPERS_H_

namespace ogl {
	void printShaderInfoLog(GLuint object);
	void printProgramInfoLog(GLuint object);
	void getErrorLog();
	void glError(std::string text);
	void glError();
	std::string getOpenGLInfo();
	bool framebufferStatus();

	//Debug functions
	void printMatrix(glm::mat4 A);
	void printMatrix(glm::mat3 A);
	void printVector(glm::vec4 u);
	void printVector(glm::vec3 u);
}

#endif