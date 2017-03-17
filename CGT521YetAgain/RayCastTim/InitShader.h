#ifndef __INITSHADER_H__
#define __INITSHADER_H__

GLuint InitShader( const char* computeShaderFile);
GLuint InitShader( const char* vertexShaderFile, const char* fragmentShaderFile );
GLuint InitShader( const char* vertexShaderFile, const char* geometryShader, const char* fragmentShaderFile );

#endif