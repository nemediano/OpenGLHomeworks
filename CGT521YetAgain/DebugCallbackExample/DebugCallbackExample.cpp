#include "stdafx.h"
using namespace std;


int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 800;

GLuint shaderProgram;
GLuint positionUniformLocation;

GLuint vertexArrayObject;
GLuint vertexBuffer;

typedef struct {
	glm::vec4 position;
	glm::vec4 colour;
} Vertex;

GLuint InitShader(const char* vertexShaderFile,	const char* fragmentShaderFile);

void loadShader();
void display();

void loadBufferData() {
	Vertex vertexData[4] = {
		{ glm::vec4(-0.5, -0.5, 0.0, 1.0), glm::vec4(1.0, 0.0, 0.0, 1.0) },
		{ glm::vec4(-0.5,  0.5, 0.0, 1.0), glm::vec4(0.0, 1.0, 0.0, 1.0) },
		{ glm::vec4(0.5,  0.5, 0.0, 1.0),  glm::vec4(0.0, 0.0, 1.0, 1.0) },
		{ glm::vec4(0.5, -0.5, 0.0, 1.0),  glm::vec4(1.0, 1.0, 1.0, 1.0) }
	};

	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vertex), vertexData, GL_STATIC_DRAW);

	const GLuint positionAttributeLocation = 0;
	const GLuint colourAttributeLocation = 1;
	glEnableVertexAttribArray(positionAttributeLocation);
	glEnableVertexAttribArray(colourAttributeLocation);
	glVertexAttribPointer(positionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)0);
	glVertexAttribPointer(colourAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)sizeof(glm::vec4));
}

void loadShader() {
	shaderProgram = InitShader("color.vert", "color.frag");
	positionUniformLocation = glGetUniformLocation(shaderProgram, "p");
	if (positionUniformLocation < 0) {
		cerr << "Shader did not contain the 'p' uniform." << endl;
	}
}

void display() {
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClear(GL_DEPTH); // <-- Bug - this trigger a debug callback
	const float timeScale = 0.008f;

	glUseProgram(shaderProgram);

	GLfloat timeValue = glutGet(GLUT_ELAPSED_TIME)*timeScale;
	glm::vec2 p(0.5f * sinf(timeValue), 0.5f * cosf(timeValue));
	glUniform2fv(positionUniformLocation, 1, glm::value_ptr(p));

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glutSwapBuffers();
}

void reshape(int W, int H) {
	WINDOW_WIDTH = W;
	WINDOW_HEIGHT = H;
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

void animate() {
	glutPostRedisplay();
}

void visible(int vis) {
	if (vis == GLUT_VISIBLE) {
		glutIdleFunc(animate);
	} else {
		glutIdleFunc(0);
	}
}

void APIENTRY openglCallbackFunction(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam) {

	cout << "---------------------opengl-callback-start------------" << endl;
	cout << "message: " << message << endl;
	cout << "type: ";
	switch (type) {
		case GL_DEBUG_TYPE_ERROR:
			cout << "ERROR";
		break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			cout << "DEPRECATED_BEHAVIOR";
		break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			cout << "UNDEFINED_BEHAVIOR";
		break;
		case GL_DEBUG_TYPE_PORTABILITY:
			cout << "PORTABILITY";
		break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			cout << "PERFORMANCE";
		break;
		case GL_DEBUG_TYPE_OTHER:
			cout << "OTHER";
		break;
	}
	cout << endl;

	cout << "id: " << id << endl;
	cout << "severity: ";
	switch (severity) {
		case GL_DEBUG_SEVERITY_LOW:
			cout << "LOW";
		break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			cout << "MEDIUM";
		break;
		case GL_DEBUG_SEVERITY_HIGH:
			cout << "HIGH";
		break;
	}
	cout << endl;
	cout << "---------------------opengl-callback-end--------------" << endl;
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitContextVersion(4, 5);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE
#if _DEBUG		
		| GLUT_DEBUG
#endif
	);
	glutSetOption(
		GLUT_ACTION_ON_WINDOW_CLOSE,
		GLUT_ACTION_GLUTMAINLOOP_RETURNS
	);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("OpenGL 4 example");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutVisibilityFunc(visible);
	glutIdleFunc(animate);
	glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
	glewExperimental = true;
	GLint GlewInitResult = glewInit();
	if (GlewInitResult != GLEW_OK) {
		printf("ERROR: %s\n", glewGetErrorString(GlewInitResult));
	}

#if _DEBUG
	if (glDebugMessageCallback) {
		cout << "Register OpenGL debug callback " << endl;
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(openglCallbackFunction, nullptr);
		GLuint unusedIds = 0;
		glDebugMessageControl(GL_DONT_CARE,
			GL_DONT_CARE,
			GL_DONT_CARE,
			0,
			&unusedIds,
			true);
	}
	else
		cout << "glDebugMessageCallback not available" << endl;
#endif

	cout << "OpenGL initialized: OpenGL version: " << glGetString(GL_VERSION) << " GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;


	glEnable(GL_DEPTH_TEST);

	loadShader();
	loadBufferData();

	glutMainLoop();
}



	// Create a NULL-terminated string by reading the provided file
char* readShaderSource(const char* shaderFile) {

	FILE *filePointer;
	char *content = NULL;

	int count = 0;

	if (shaderFile != NULL) {
		filePointer = fopen(shaderFile, "rt");

		if (filePointer != NULL) {
			fseek(filePointer, 0, SEEK_END);
				count = ftell(filePointer);
				rewind(filePointer);

				if (count > 0) {
					content = (char *)malloc(sizeof(char) * (count + 1));
					count = fread(content, sizeof(char), count, filePointer);
					content[count] = '\0';
				}
				fclose(filePointer);
		}
	}
	return content;

}


// Create a GLSL program object from vertex and fragment shader files
GLuint InitShader(const char* vShaderFile, const char* fShaderFile) {

	struct Shader {
		const char*  filename;
		GLenum type;
		GLchar* source;
	} shaders[2] = {
		{ vShaderFile, GL_VERTEX_SHADER, NULL },
		{ fShaderFile, GL_FRAGMENT_SHADER, NULL }
	};

	GLuint program = glCreateProgram();

	for (int i = 0; i < 2; ++i) {
		Shader& s = shaders[i];
		s.source = readShaderSource(s.filename);
		if (shaders[i].source == NULL) {
			std::cerr << "Failed to read " << s.filename << std::endl;
			exit(EXIT_FAILURE);
		}
		GLuint shader = glCreateShader(s.type);
		glShaderSource(shader, 1, (const GLchar**)&s.source, NULL);
		glCompileShader(shader);

		GLint  compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled) {
			std::cerr << s.filename << " failed to compile:" << std::endl;
			GLint  logSize;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
			char* logMsg = new char[logSize];
			glGetShaderInfoLog(shader, logSize, NULL, logMsg);
			std::cerr << logMsg << std::endl;
			delete[] logMsg;

			exit(EXIT_FAILURE);
		}

		delete[] s.source;

		glAttachShader(program, shader);
	}

	/* link  and error check */
	glLinkProgram(program);

	GLint  linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked) {
		std::cerr << "Shader program failed to link" << std::endl;
		GLint  logSize;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
		char* logMsg = new char[logSize];
		glGetProgramInfoLog(program, logSize, NULL, logMsg);
		std::cerr << logMsg << std::endl;
		delete[] logMsg;

		exit(EXIT_FAILURE);
	}

	/* use program object */
	glUseProgram(program);

	return program;
}

