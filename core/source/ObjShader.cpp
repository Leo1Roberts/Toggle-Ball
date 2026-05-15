#include <GLUtilities.h>
#include "main.h"
#include "MatrixUtilities.h"
#include "Game.h"
#include "ObjShader.h"
#include "LoadOBJ.h"

ObjShader* ObjShader::loadObjShader(const std::string& vertexSource, const std::string& fragmentSource) {
	ObjShader* shader = nullptr;

	GLuint vertexShader = 0;
	vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);
	if (!vertexShader) {
		return nullptr;
	}

	GLuint fragmentShader = 0;
	fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);
	if (!fragmentShader) {
		glDeleteShader(vertexShader);
		CHECK_ERROR();
		return nullptr;
	}

	GLuint program = glCreateProgram();
	CHECK_ERROR();
	if (program) {
		glAttachShader(program, vertexShader);
		CHECK_ERROR();
		glAttachShader(program, fragmentShader);
		CHECK_ERROR();

		glLinkProgram(program);
		CHECK_ERROR();
		GLint linkStatus = GL_FALSE;
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
		CHECK_ERROR();
		if (linkStatus != GL_TRUE) {
			GLint logLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
			CHECK_ERROR();

			// If we fail to link the shader program, log the result for debugging
			if (logLength) {
				GLchar* log = new GLchar[logLength];
				glGetProgramInfoLog(program, logLength, nullptr, log);
				CHECK_ERROR();
				delete[] log;
			}

			glDeleteProgram(program);
			CHECK_ERROR();
		} else {
			shader = new ObjShader(program);
		}
	}

	// The shaders are no longer needed once the program is linked. Release their memory.
	glDeleteShader(vertexShader);
	CHECK_ERROR();
	glDeleteShader(fragmentShader);
	CHECK_ERROR();

	return shader;
}

void ObjShader::setupVertexAttribs() const {
	// The position attribute is 3 floats
	glVertexAttribPointer(
			0, // attrib
			3, // elements
			GL_FLOAT, // of type float
			GL_FALSE, // don't normalize
			sizeof(Vertex), // stride is Vertex bytes
	(void*)offsetof(Vertex, position) // pull from the start of the vertex data
	);
	CHECK_ERROR();
	glEnableVertexAttribArray(0);
	CHECK_ERROR();

	// The uv attribute is 2 floats
	glVertexAttribPointer(
			1, // attrib
			2, // elements
			GL_FLOAT, // of type float
			GL_FALSE, // don't normalize
			sizeof(Vertex), // stride is Vertex bytes
			(void*)offsetof(Vertex, uv) // offset vec3 from the start
	);
	CHECK_ERROR();
	glEnableVertexAttribArray(1);
	CHECK_ERROR();

	// The normal attribute is 3 floats
	glVertexAttribPointer(
			2, // attrib
			3, // elements
			GL_FLOAT, // of type float
		GL_FALSE, // don't normalize
			sizeof(Vertex), // stride is Vertex bytes
	(void*)offsetof(Vertex, normal) // offset vec3 + vec2 from the start
	);
	CHECK_ERROR();
	glEnableVertexAttribArray(2);
	CHECK_ERROR();

	// The color attribute is 4 bytes
	glVertexAttribPointer(
			3, // attrib
			4, // elements
			GL_UNSIGNED_BYTE, // of type byte
			GL_TRUE, // normalize
			sizeof(Vertex), // stride is Vertex bytes
	(void*)offsetof(Vertex, color) // offset vec3 * 2 + vec2 from the start
	);
	CHECK_ERROR();
	glEnableVertexAttribArray(3);
	CHECK_ERROR();
}

void ObjShader::drawObject(const Model* model, const Texture* texture) const {
	// Setup the texture
	texture->bind(0);

	glBindVertexArray(model->vao);
	CHECK_ERROR();
	// Draw as indexed triangles
	glDrawElements(GL_TRIANGLES, (GLsizei) model->indices.size(), GL_UNSIGNED_SHORT, 0);
	CHECK_ERROR();
}

GLint ObjShader::getUniformLocation(const std::string& name) const {
	GLint uniformLocation = glGetUniformLocation(program, name.c_str());
	CHECK_ERROR();
	if (uniformLocation == -1)
		int error = true; // Error: uniform not found
	return uniformLocation;
}

ObjShader* objShader;
ObjShader* outlineShader;

void ObjShader::deleteShaders() {
	delete objShader;
	delete outlineShader;
}