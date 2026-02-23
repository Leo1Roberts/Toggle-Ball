#include "main.h"
#include "Colors.h"
#include "Line.h"

const int MAX_LINES = 100;

LineVertex Line::vertices[MAX_LINES * 2];

GLuint Line::program;
GLint Line::position;
GLint Line::color;
GLint Line::projectionMatrix;

GLuint Line::vao;
GLuint Line::vertex_buffer;

int Line::numLines;

mat4 Line::projMat;

void Line::drawLines() {
	activateShader();
	glUniformMatrix4fv(projectionMatrix, 1, false, projMat.m16);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(LineVertex) * numLines * 2, vertices, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_LINES, 0, numLines * 2);
	glUseProgram(0);
	numLines = 0;
}

bool Line::initLineShader() {
	const char* vertexSource =
	        "#version 300 es\n"
	        "in vec3 inPos;\n"
	        "in vec4 inCol;\n"
	        "out vec4 fragCol;\n"
	        "uniform mat4 uProjection;\n"
	        "void main() {\n"
	        "fragCol = inCol;\n"
	        "gl_Position = uProjection * vec4(inPos, 1.0);\n"
	        "}\n";

	const char* fragmentSource =
	        "#version 300 es\n"
	        "precision mediump float;\n"
	        "in vec4 fragCol;\n"
	        "out vec4 outColor;\n"
	        "void main() {\n"
	        "outColor = fragCol;\n"
	        "}\n";

	GLuint vertexShader = 0;
	vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);
	if (!vertexShader) {
		return false;
	}

	GLuint fragmentShader = 0;
	fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);
	if (!fragmentShader) {
		glDeleteShader(vertexShader);
		return false;
	}

	GLuint iProgram = glCreateProgram();
	if (iProgram) {
		glAttachShader(iProgram, vertexShader);
		glAttachShader(iProgram, fragmentShader);

		glLinkProgram(iProgram);
		GLint linkStatus = GL_FALSE;
		glGetProgramiv(iProgram, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE) {
			GLint logLength = 0;
			glGetProgramiv(iProgram, GL_INFO_LOG_LENGTH, &logLength);

			// If we fail to link the shader program, log the result for debugging
			if (logLength) {
				GLchar* log = new GLchar[logLength];
				glGetProgramInfoLog(iProgram, logLength, nullptr, log);
				delete[] log;
			}

			glDeleteProgram(iProgram);
		} else {
			GLint positionAttribute = glGetAttribLocation(iProgram, "inPos");
			GLint colorAttribute = glGetAttribLocation(iProgram, "inCol");
			GLint projectionMatrixUniform = glGetUniformLocation(iProgram, "uProjection");

			if (positionAttribute != -1 && colorAttribute != -1 && projectionMatrixUniform != -1) {
				program = iProgram;
				position = positionAttribute;
				color = colorAttribute;
				projectionMatrix = projectionMatrixUniform;
			} else {
				glDeleteProgram(iProgram);
			}
		}
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	activateShader();

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	// The position attribute is 3 floats
	glVertexAttribPointer(
	        position,          // attrib
	        3,                 // elements
	        GL_FLOAT,          // of type float
	        GL_FALSE,          // don't normalize
	        sizeof(LineVertex),// stride is LineVertex bytes
	        0                  // pull from the start of the vertex data
	);
	glEnableVertexAttribArray(position);

	// The color attribute is 4 bytes
	glVertexAttribPointer(
	        color,                // attrib
	        4,                    // elements
	        GL_UNSIGNED_BYTE,     // of type byte
	        GL_TRUE,              // normalize
	        sizeof(LineVertex),   // stride is LineVertex bytes
	        (uint8_t*)sizeof(vec3)// offset vec3
	);
	glEnableVertexAttribArray(color);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return true;
}

bool Line::addLine(const vec3& from, const vec3& to, const col& lineColor) {
	if (numLines >= MAX_LINES) return false;
	vertices[numLines * 2] = LineVertex(from, lineColor);
	vertices[numLines * 2 + 1] = LineVertex(to, lineColor);
	numLines++;
	return true;
}
