#ifndef __USEVBO_H__
#define __USEVBO_H__

extern GLuint shader_program[2];
extern GLuint vbo[2];

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

void draw_from_vbo()
{
   glClear(GL_COLOR_BUFFER_BIT);

   glUseProgram(shader_program[0]); //subsequent draw calls will use shader_program[0]
   glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //specify the buffer where vertex attribute data is stored
   GLint pos_loc0 = glGetAttribLocation(shader_program[0], "pos_attrib"); //get a reference to an attrib variable name in a shader
	glEnableVertexAttribArray(pos_loc0); //enable this attribute
	glVertexAttribPointer(pos_loc0, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(0)); //tell opengl how to get the attribute values out of the vbo (stride and offset)
	glDrawArrays(GL_TRIANGLES, 0, 3); // draw 3 vertices as triangles

   glUseProgram(shader_program[1]);
   glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
   GLint pos_loc1 = glGetAttribLocation(shader_program[1], "pos_attrib");
	glEnableVertexAttribArray(pos_loc1);
	glVertexAttribPointer(pos_loc1, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(0));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glutSwapBuffers();
}


#endif