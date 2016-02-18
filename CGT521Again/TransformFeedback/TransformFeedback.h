/*
 *		Written by Tim McGraw, 2010
 *		www.csee.wvu.edu/~tmcgraw
 */

#ifndef __TRANSFORMFEEDBACK_H__
#define __TRANSFORMFEEDBACK_H__

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

void glError();
void printShaderInfoLog(GLuint obj);
void printProgramInfoLog(GLuint obj);
void ExitGlut();

#endif