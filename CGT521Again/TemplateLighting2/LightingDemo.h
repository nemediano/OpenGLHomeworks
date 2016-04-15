#ifndef INSTANCE_LAB_H
#define INSTANCE_LAB_H

//Window and user input  management
void exit_glut();
void init_OpenGL();
void init_program();
void create_glut_window();
void create_glut_callbacks();

//Scene creation
void pass_light();
void pass_material();

//Only callback function outside interface
void display();

//For reloading thing after state change
void reload_shaders();
void reload_mesh_and_texture();

//For the gui
void draw_gui();

#endif