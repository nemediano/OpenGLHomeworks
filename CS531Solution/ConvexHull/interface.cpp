#include <glm/glm.hpp>

#include "interface.h"
#include "Globals.h"
#include "opengl/HelperFunctions.h"
#include "convex_hull.h"

using namespace options;

void reshape(int new_window_width, int new_window_height) {
	glViewport(0, 0, new_window_width, new_window_height);

	//Avoid zero division. This is degenerated case anyway
	if (new_window_height == 0) {
		new_window_height = 1;
	}

	GLfloat aspect_ratio = static_cast<float>(new_window_width) / static_cast<float>(new_window_height);

	const GLfloat desired_world = 1.0f;

	if (new_window_width >= new_window_height) { //Aspect ratio is grater than 1
		world_high.x = desired_world * aspect_ratio;
		world_low.x = -desired_world * aspect_ratio;
		world_high.y = desired_world;
		world_low.y = -desired_world;
	}
	else {
		world_high.x = desired_world;
		world_low.x = -desired_world;
		world_high.y = desired_world / aspect_ratio;
		world_low.y = -desired_world / aspect_ratio;
	}

}

void idle() {
	//timers for time-based animation
	static int last_time = 0;
	int time = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = time - last_time;
	float delta_seconds = 0.001f * elapsed;
	last_time = time;

	opengl::gl_error("At idle"); //check for errors and print error strings
	glutPostRedisplay();
}

void keyboard(unsigned char key, int mouse_x, int mouse_y) {
	if (key == 27) {//press ESC to exit
		exit_glut();
	}
	else if (key == 'c' || key == 'C') {
		nPoints -= 3;
		create_primitives();
	}
	else if (key == 'v' || key == 'V') {
		nPoints += 3;
		create_primitives();
	}

	glutPostRedisplay();
}

void mouse(int button, int state, int mouse_x, int mouse_y) {
	using glm::vec2;
	using glm::vec3;
	using glm::ivec2;

	vec2 mouse_in_world;
	ivec2 window_size = ivec2(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	vec2 mouse_in_window = vec2(mouse_x, window_size.y - mouse_y);
	vec3 world_size = world_high - world_low;
	mouse_in_world = vec2(world_size.x / window_size.x, world_size.y / window_size.y) * mouse_in_window - 0.5f * vec2(world_size.x, world_size.y);

	vec3 red = vec3(1.0f, 0.0f, 0.0f);
	if (button == GLUT_LEFT && state == GLUT_DOWN) {
		Vertex tmp;
		tmp.position = mouse_in_world;
		tmp.color = red;
		vertices.push_back(tmp);
		indices_points.push_back(vertices.size() - 1);
		update_gpu_data();
	}

	glutPostRedisplay();
}