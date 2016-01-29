#ifndef INSTANCE_LAB_H
#define INSTANCE_LAB_H

//Program management
void exit_glut();
void init_OpenGL();
void init_program();
void create_glut_window();
void create_glut_callbacks();


//Scene creation
void create_primitives();
void pass_light_and_material();

//Only callback function outside interface
void display();

#endif