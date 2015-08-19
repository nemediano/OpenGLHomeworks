#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>

//This is a bug on GLM in 32 bits I hope they solve it soon
#define GLM_FORCE_PURE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include <iostream>
#include <random>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <string>
#include <chrono>

#include "OpenGLProgram.h"

using namespace std;
using namespace shaders;

/* For randoms number generation */
typedef std::chrono::high_resolution_clock myclock;
myclock::time_point beginning;

std::mt19937 generator;


static int win = 0;

OpenGLProgram* pProgram;
GLint M1_loc = -1;
GLint M2_loc = -1;
GLint M3_loc = -1;
GLint vColor_loc = -1;

//float coeficient_of_restitution;

struct Particle {
	glm::vec2 position;
	glm::vec2 velocity;
	glm::vec2 acceleration;
	glm::vec3 color;
	float radius;
};

struct Obstacle {
	glm::vec2 position;
	glm::vec3 color;
	float radius;
};

struct Block {
	glm::vec2 position;
	glm::vec3 color;
	float width;
	float height;
};

Particle ball;
Block player;
Obstacle obstacle_1;

void draw_text (const string& message, const float& posX, const float& posY);
void draw_high_score();

void special_key(int key, int mouse_x, int mouse_y);
void mouse_passive(int mouse_x, int mouse_y);
void initialize_game_state();
void place_obstacle();

enum GameState{MENU, PLAY, GAME_OVER, PAUSE};
GameState current_game_state;
float game_over_timer = 0.0f;
unsigned int game_score, high_score;
int last_time;
bool obstacle_on;
bool big_ball;
bool big_player;

void exitGlut() {
	delete pProgram;
	glutDestroyWindow(win);
	exit(EXIT_SUCCESS);
}

void initOpenGL() {
	
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
	}
	cout << "Hardware specification: " << endl;
	cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Software specification: " << endl;
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;
	cout << "Using OpenGL " << glGetString(GL_VERSION) << endl;
	cout << "Using GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	int ver = glutGet(GLUT_VERSION);
	cout << "Using freeglut version: " << ver / 10000 << "." << (ver / 100) % 100 << "." << ver % 100 << endl;

	pProgram = new OpenGLProgram("vertexShader.glsl", "fragmentShader.glsl");

	if (!pProgram->is_ok()) {
		cerr << "Error at GL program creation" << endl;
		gl_error();
		exit(EXIT_FAILURE);
	}

	M1_loc = pProgram->get_uniform_location("M1");
	M2_loc = pProgram->get_uniform_location("M2");
	M3_loc = pProgram->get_uniform_location("M3");
	vColor_loc = pProgram->get_uniform_location("vColor");

	//initialize some basic rendering state
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glPointSize(2.0f);
	glLineWidth(1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	//Initialize all game state variables
	current_game_state = MENU;
	ball.color = glm::vec3(1.0f, 0.0f, 0.0f);
	player.color = glm::vec3(0.0f, 0.0f, 1.0f);
	obstacle_1.color = glm::vec3(0.0f, 1.0f, 0.0f);

	initialize_game_state();

	high_score = 0;
	
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT); 

	glm::mat4 S(1.0f);
	glm::mat4 T(1.0f);
	glm::mat4 I(1.0f);

	float scale_x, scale_y, scale_z;
	float translate_x, translate_y, translate_z;

	scale_x = scale_y = scale_z = 0.05f;
	S = glm::scale(I, glm::vec3(scale_x, scale_y, scale_z));

	translate_x = translate_y = 0.5f;
	translate_z = 0.0f;
	T = glm::translate(I, glm::vec3(translate_x, translate_y, translate_z));

	pProgram->use_program();

	glUniformMatrix4fv(M1_loc, 1, GL_FALSE, glm::value_ptr(I));
	glUniformMatrix4fv(M2_loc, 1, GL_FALSE, glm::value_ptr(I));
	glUniformMatrix4fv(M3_loc, 1, GL_FALSE, glm::value_ptr(I));
	
	if (current_game_state == MENU) {
		draw_text("Press F1 to start", -0.4f, -0.1f);
		draw_high_score();
	} else if (current_game_state == GAME_OVER) {
		draw_text("Game over", -0.33f, 0.0f);
		draw_high_score();
	} else {
		//Draw score
		draw_high_score();
		string score = "Score " + std::to_string(game_score) + "\n";
		draw_text(score, 0.63f, 0.85f);
		
		if (current_game_state == PAUSE) {
			draw_text("PAUSE", -0.2f, 0.0f);
		}

		//Draw ball
		glUniform3fv(vColor_loc, 1, glm::value_ptr(ball.color));
		scale_x = scale_y = ball.radius;
		S = glm::scale(I, glm::vec3(scale_x, scale_y, scale_z));
		glUniformMatrix4fv(M1_loc, 1, GL_FALSE, glm::value_ptr(S));
		translate_x = ball.position.x;
		translate_y = ball.position.y;
		T = glm::translate(I, glm::vec3(translate_x, translate_y, translate_z));
		glUniformMatrix4fv(M2_loc, 1, GL_FALSE, glm::value_ptr(T));
		glUniformMatrix4fv(M3_loc, 1, GL_FALSE, glm::value_ptr(I));
		glutSolidSphere(1.0f, 20, 20);
		//Draw player
		//set color to draw with
		glUniform3fv(vColor_loc, 1, glm::value_ptr(player.color));
		scale_x = player.width;
		scale_y = player.height;
		S = glm::scale(I, glm::vec3(scale_x, scale_y, scale_z));
		glUniformMatrix4fv(M1_loc, 1, GL_FALSE, glm::value_ptr(S));
		translate_x = player.position.x;
		translate_y = player.position.y;
		T = glm::translate(I, glm::vec3(translate_x, translate_y, translate_z));
		glUniformMatrix4fv(M2_loc, 1, GL_FALSE, glm::value_ptr(T));
		glUniformMatrix4fv(M3_loc, 1, GL_FALSE, glm::value_ptr(I));
		glutSolidCube(1.0f);
		if (obstacle_on) {
			glUniform3fv(vColor_loc, 1, glm::value_ptr(obstacle_1.color));
			scale_x = scale_y = obstacle_1.radius;
			S = glm::scale(I, glm::vec3(scale_x, scale_y, scale_z));
			glUniformMatrix4fv(M1_loc, 1, GL_FALSE, glm::value_ptr(S));
			translate_x = obstacle_1.position.x;
			translate_y = obstacle_1.position.y;
			T = glm::translate(I, glm::vec3(translate_x, translate_y, translate_z));
			glUniformMatrix4fv(M2_loc, 1, GL_FALSE, glm::value_ptr(T));
			glUniformMatrix4fv(M3_loc, 1, GL_FALSE, glm::value_ptr(I));
			glutSolidSphere(1.0f, 20, 20);
		}
		
	}
	glutSwapBuffers();
	gl_error("At the end of render!");
}

void keyboard(unsigned char key, int x, int y) {
   switch (key) { //Esc to exit
	 case 27: 
		exitGlut();
	 break;

	 case 'P':
	 case 'p':
		 if (current_game_state == PLAY) {
			 current_game_state = PAUSE;
		 } else if (current_game_state == PAUSE) {
			 current_game_state = PLAY;
		 }
	 break;

	 case 'O':
	 case 'o':
		 if (current_game_state == PLAY) {
			obstacle_on = (!obstacle_on);
			if (obstacle_on) {
				place_obstacle();
			}
		 }
	 break;
   }

}

void idle() { 
	if (current_game_state == MENU) {
		return;
	}

   //timers for time-based animation
   int time = glutGet(GLUT_ELAPSED_TIME);
   int elapsed = time - last_time;

   if (current_game_state == GAME_OVER && elapsed < 5.0f) {
	   return;
   }
   
   if (current_game_state == GAME_OVER) {
	   current_game_state = MENU;
   }
   float delta_seconds = 0.001f * elapsed;
   last_time = time;
   
   if (current_game_state == PAUSE) {
	   return;
   }

   /*Integrate newton equation to get ball's new position*/
   ball.velocity += ball.acceleration * delta_seconds;
   ball.position += ball.velocity * delta_seconds;
   
   const float speed_adjust = -0.9f;

   //Left wall collision detection
   if( (ball.position.x - ball.radius) < -1.0f) {
	   ball.position.x = -1.0f + ball.radius;
	   ball.velocity.x *= speed_adjust;
	   //Right wall collision detection
   } else if((ball.position.x + ball.radius) > +1.0f) {
	   ball.position.x =  1.0f - ball.radius;
	   ball.velocity.x *= speed_adjust;
	   //Top wall collision detection
   } else if((ball.position.y + ball.radius) > +1.0f) {
	   ball.position.y =  1.0f - ball.radius;
	   ball.velocity.y *= speed_adjust;
   }

   //Collision detection against obstacle
   if (obstacle_on) {
	   float distance = glm::distance(ball.position, obstacle_1.position);
	   // Check if we hit the obstacle
	   if (distance <= (ball.radius + obstacle_1.radius)) {
		   //Some vector magic to solve the collision
		   //Calculating normal to collision plane
		   glm::vec2 collision_normal = ball.position - obstacle_1.position; //vector from obstacle to ball
		   collision_normal = glm::normalize(collision_normal);
		   //Adjust position
		   ball.position = obstacle_1.position + (ball.radius + obstacle_1.radius) * collision_normal;
		   //Adjust velocity
		   ball.velocity = ball.velocity - 2.0f * glm::dot(ball.velocity, collision_normal) * collision_normal;
	   }
   }

   //Collision detection against player
   if (ball.position.x >= (player.position.x - player.width / 2.0f) && 
	   ball.position.x <= (player.position.x + player.width / 2.0f) &&
	   ball.position.y - ball.radius <= (player.position.y + player.height / 2.0f)) {
	   
	   ball.position.y =  player.position.y + player.height / 2.0f + ball.radius + 0.01f; //For stability reasons weird!!!!
	   ball.velocity *= 1.2f;
	   ball.velocity.y *= -1.0f;

	   game_score++;
   }

   //Check for game over
   //Bottom wall collision detection
   if((ball.position.y - ball.radius)  < -1.0f) {
	   current_game_state = GAME_OVER;
	   if (game_score > high_score) {
		   high_score = game_score;
	   }
   }

   gl_error("At the end of idle"); //check for errors and print error strings
   glutPostRedisplay();
}

void createGlutWindow() {
	glutInitDisplayMode (GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize (512, 512);
	win = glutCreateWindow ("Jorge Garcia, Project 2");
}

void special_key(int key, int mouse_x, int mouse_y) {
	switch (key) {
		case GLUT_KEY_F1:
			if (current_game_state == MENU) {
				current_game_state = PLAY;
				last_time = glutGet(GLUT_ELAPSED_TIME);
				initialize_game_state();
			}
		break;

		case GLUT_KEY_F2:
			if (current_game_state == PLAY) {
				big_ball = (!big_ball);
				ball.radius = big_ball ? 0.1f : 0.05f;
			}
		break;

		case GLUT_KEY_F3:
			if (current_game_state == PLAY) {
				big_player = (!big_player);
				player.width = big_player ? 0.5f : 0.25f;
			}
		break;

		case GLUT_KEY_F4:
			if (current_game_state == PLAY && obstacle_on) {
				place_obstacle();
			}
		break;
	}
	glutPostRedisplay();
}

void createGlutCallbacks() {
	glutDisplayFunc		(display);
	glutIdleFunc		(idle);
	glutKeyboardFunc	(keyboard);
	glutSpecialFunc		(special_key);
	glutPassiveMotionFunc(mouse_passive);
}

int main (int argc, char* argv[]) {

	//Take the timestamp for the random number generator
	myclock::time_point beginning = myclock::now();

	glutInit(&argc, argv); 
	createGlutWindow();
	createGlutCallbacks();
	initOpenGL();
	glutMainLoop();
	glutDestroyWindow(win);

	return EXIT_SUCCESS;
}

void draw_text (const string& message, const float& posX, const float& posY) {
	
	assert(posX > -1.0f);
	assert(posX <  1.0f);
	assert(posY > -1.0f);
	assert(posY <  1.0f);

	glRasterPos2f(posX, posY);
	
	for(auto letter : message) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, letter);
	}
	
}

void draw_high_score() {
	string high_score_msg = "High score: " + std::to_string(high_score) + "\n";
	draw_text(high_score_msg, -0.95f, 0.85f);
}

void mouse_passive(int mouse_x, int mouse_y) {
	player.position.x = 2.0f * static_cast<float>(mouse_x) / 511.0f - 1.0f;
}

void initialize_game_state() {
	//Initialize random number generator 
	myclock::duration elapsed_time = myclock::now() - beginning;
	//With the number of millisecond elapsed since the beginning of the program
	generator.seed(static_cast<unsigned>(elapsed_time.count()));

	//Options
	big_ball = big_player = true;

	//Ball
	ball.position = glm::vec2(0.0f, 0.5f);
	ball.velocity = glm::vec2(0.0f, 0.0f);
	ball.acceleration = glm::vec2(0.0f, -0.8f);
	ball.radius = big_ball ? 0.1f : 0.05f;
	//Random initial velocity
	const float TAU = 6.28318530718f;
	std::uniform_real_distribution<float> distribution(0.0f, TAU);
	float angle = distribution(generator);
	ball.velocity = glm::vec2(cos(angle), sin(angle));
	
	//Player
	player.height = 0.2f;
	player.width = big_player ? 0.5f : 0.25f;
	player.position = glm::vec2(0.0f, -1.0f + player.height / 2.0f);

	//Reset obstacle
	obstacle_on = false;
	obstacle_1.position = glm::vec2(-1.0f, -1.0f);
	obstacle_1.radius = 0.0f;

	//Score
	game_score = 0;
}

void place_obstacle() {
	obstacle_1.radius = 0.1f;

	std::uniform_real_distribution<float> distribution_x (-1.0f + obstacle_1.radius, 1.0f - obstacle_1.radius);
	std::uniform_real_distribution<float> distribution_y (-1.0f + player.height + obstacle_1.radius, 1.0f - obstacle_1.radius);
	obstacle_1.position = glm::vec2(distribution_y(generator), distribution_y(generator));
}