#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "Globals.h"
#include "../opengl/HelperFunctions.h"
#include "../Homework2.h"
#include "Callbacks.h"

void reshape(int new_window_width, int new_window_height) {
	glViewport(0, 0, new_window_width, new_window_height);
}

void idle() {
	//timers for time-based animation
	static int last_time = 0;
	int time = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = time - last_time;
	options::delta_seconds = 0.001f * elapsed;
	options::elapsed_time += options::delta_seconds;
	last_time = time;

	opengl::gl_error("At idle"); //check for errors and print error strings
	glutPostRedisplay();
}

void keyboard(unsigned char key, int mouse_x, int mouse_y) {
	if (key == 27) {//press ESC to exit
		exit_glut();
	} else if (key == 'c' || key == 'C') {
		reset_camera();
	} else if (key == 'r' || key == 'R') {
		options::rotate_fish = (!options::rotate_fish);
	} else if (key == '1') {
		options::filter_option = 1;
	} else if (key == '2') {
		options::filter_option = 2;
	} else if (key == '3') {
		options::filter_option = 3;
	} else if (key == '4') {
		options::filter_option = 0;
	}

}

void mouse_wheel(int wheel, int direction, int mouse_x, int mouse_y) {
	const float DELTA_ANGLE = PI / 20.0f;
	if (wheel == 0) {
		if (direction > 0 && options::field_of_view_y < (PI - 2.0f * DELTA_ANGLE)) {
			options::field_of_view_y += DELTA_ANGLE;
		} else if (options::field_of_view_y > 2.0 * DELTA_ANGLE){
			options::field_of_view_y -= DELTA_ANGLE;
		}
	}
}

void mouse_active(int mouse_x, int mouse_y) {
	glm::vec2 mouse_current;
	if (options::mouse_dragging) {
		mouse_current = glm::vec2(static_cast<float>(mouse_x), static_cast<float>(mouse_y));
		glm::vec2 scale_factors = glm::vec2(2.0f / glutGet(GLUT_WINDOW_WIDTH), -2.0f / glutGet(GLUT_WINDOW_HEIGHT));	
		glm::vec2 window_center = 0.5f * glm::vec2(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		glm::vec2 mouse_current_in_world = scale_factors * (mouse_current - window_center);
		glm::vec2 mouse_start_drag_in_world = scale_factors * (options::mouse_start_drag - window_center);
		
		glm::vec3 v_1 = glm::vec3(mouse_current_in_world, projection_on_curve(mouse_current_in_world));
		glm::vec3 v_2 = glm::vec3(mouse_start_drag_in_world, projection_on_curve(mouse_start_drag_in_world));
		v_1 = glm::normalize(v_1);
		v_2 = glm::normalize(v_2);
		glm::vec3 axis = glm::cross(v_1, v_2);
		float angle = glm::angle(v_1, v_2);
		options::camera_new_rotation = glm::normalize(glm::quat(glm::cos(0.5f * angle), glm::sin(0.5f * angle) * axis));
		
	}
	glutPostRedisplay();
}

float projection_on_curve(glm::vec2 projected) {
	const float radius = 0.5f;
	float z = 0.0f;
	if (glm::length2(projected) <= (0.5f * radius * radius)) {
		//Inside the sphere
		z = glm::sqrt(radius * radius - glm::length2(projected));
	}
	else {
		//Outside of the sphere using hyperbolic sheet
		z = (0.5f * radius * radius) / glm::length(projected);
	}
	return z;
}

void mouse(int button, int state, int mouse_x, int mouse_y) {

	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
		options::mouse_dragging = true;
		options::mouse_start_drag.x = static_cast<float>(mouse_x);
		options::mouse_start_drag.y = static_cast<float>(mouse_y);
	}
	else if (button == GLUT_LEFT_BUTTON) {
		options::mouse_dragging = false;
		/* Calculate the accumulated rotation: base rotation plus new one */
		options::camera_base_rotation = glm::normalize(options::camera_new_rotation * options::camera_base_rotation);
		/* Reset new rotation to identity */
		options::camera_new_rotation = glm::normalize(glm::quat(1.0f, glm::vec3(0.0f, 0.0f, 0.0f)));
	}
	else if (state == GLUT_DOWN && button == GLUT_RIGHT_BUTTON) {
		//Read the pixel value form the FBO used to render the first pass attachment 2
		//Which is the one that contains the instances ID
		GLubyte pixel[4];
		glBindFramebuffer(GL_FRAMEBUFFER, options::fbo_id);
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		int height = glutGet(GLUT_WINDOW_HEIGHT);
		glReadPixels(mouse_x, height - mouse_y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//To check against background 
		//Query the blue channel (in instances is zero)
		int blue = static_cast<int>(pixel[1]);
		if (blue != 0) {
			options::selected_id = options::INSTANCE_NUMBER;
			std::cout << "You clicked the background!" << std::endl;
		}
		else {
			//To query the instance number use red channel
			int red = static_cast<int>(pixel[0]);
			options::selected_id = red - 1;
			std::cout << "You clicked on instance: " << options::selected_id << std::endl;
			
		}
		
	 }
	
	glutPostRedisplay();
}


void reset_camera() {
	options::camera_position = glm::vec3(0.0f, 0.0f, options::world_radious);
	options::camera_center = glm::vec3(0.0f, 0.0f, 0.0f);
	options::camera_up = glm::vec3(0.0f, 1.0f, 0.0);
	options::field_of_view_y = PI / 3.0f;
	options::camera_base_rotation = glm::quat(1.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	options::camera_new_rotation = glm::quat(1.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	options::camera_pan = glm::vec2(0.0, 0.0);
}

void special_keyboard(int key, int mouse_x, int mouse_y) {
	switch (key) {
		case GLUT_KEY_PAGE_UP:
			++options::filter_option %= options::FILTERS_NUMBER;
		break;

		case GLUT_KEY_PAGE_DOWN: 
		{
			options::filter_option += options::FILTERS_NUMBER;
			--options::filter_option %= options::FILTERS_NUMBER;
		}	
		break;

		default:
		break;
	}
}