#ifndef CONVEX_HULL_H
#define CONVEX_HULL_H

#include <glm/glm.hpp>

//Program management
void exit_glut();
void init_OpenGL();
void init_program();
void create_glut_window();
void create_glut_callbacks();
void allocate_buffers();
void update_gpu_data();

//Scene creation
void create_primitives();
void render_points(const glm::mat4& PVM);
void render_lines(const glm::mat4& PVM);
void render_triangles(const glm::mat4& PVM);

//Only callback function not defined in interface
void display();

#endif