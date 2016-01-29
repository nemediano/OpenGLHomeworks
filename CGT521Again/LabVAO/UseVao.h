#ifndef __USEVAO_H__
#define __USEVAO_H__

extern GLuint shader_program[2];
extern GLuint vbo[2];
extern GLuint vao[2];

void init_vao()
{
   glGenVertexArrays(2, vao); //generate vaos to hold the mapping from attrib variables in shader to memory locations in vbo

   glBindVertexArray(vao[0]); //binding vao[0] means that bindbuffer, enablevertexattribarray and vertexattribpointer state will be remembered by vao[0]
   glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //specify the buffer where vertex attribute data is stored
   GLint pos_loc0 = glGetAttribLocation(shader_program[0], "pos_attrib"); //get a reference to an attrib variable name in a shader
	glEnableVertexAttribArray(pos_loc0); //enable this attribute
	glVertexAttribPointer(pos_loc0, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(0)); //tell opengl how to get the attribute values out of the vbo (stride and offset)

   glBindVertexArray(vao[1]);
   glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
   GLint pos_loc1 = glGetAttribLocation(shader_program[1], "pos_attrib");
	glEnableVertexAttribArray(pos_loc1);
	glVertexAttribPointer(pos_loc1, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(0));

   glBindVertexArray(0); //unbind vao
   glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind vbo
}

void draw_from_vao()
{
   glClear(GL_COLOR_BUFFER_BIT);

   glUseProgram(shader_program[0]);
   glBindVertexArray(vao[0]);
   glDrawArrays(GL_TRIANGLES, 0, 3);

   glUseProgram(shader_program[1]);
   glBindVertexArray(vao[1]);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   glBindVertexArray(0);

   glutSwapBuffers();
}


#endif