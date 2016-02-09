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
void create_sphere();
void subdivide_face(glm::vec3& p_0, glm::vec3& p_1, glm::vec3& p_2, const size_t& level);
void create_indexed_mesh();
void pass_light_and_material();

//Only callback function outside interface
void display();

#endif