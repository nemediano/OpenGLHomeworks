#ifndef INTERFACE_H
#define INTERFACE_H

//Callback function
void reshape(int new_window_width, int new_window_height);
void keyboard(unsigned char key, int mouse_x, int mouse_y);
//void special_keyboard(int key, int mouse_x, int mouse_y);
//void mouse_active(int mouse_x, int mouse_y);
void mouse(int button, int state, int mouse_x, int mouse_y);
//void mouse_wheel(int wheel, int direction, int mouse_x, int mouse_y);
void idle();

#endif // !INTERFACE_H
