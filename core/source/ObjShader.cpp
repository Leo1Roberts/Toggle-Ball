#include "main.h"
#include "MatrixUtilities.h"
#include "Game.h"
#include "ObjShader.h"
#include "LoadOBJ.h"

ObjShader* ObjShader::loadObjShader(
		const std::string& vertexSource,
		const std::string& fragmentSource,
		const std::string& positionAttributeName,
		const std::string& uvAttributeName,
		const std::string& normalAttributeName,
		const std::string& colorAttributeName) {
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
			// Get the attribute and uniform locations by name. You may also choose to hardcode
			// indices with layout= in your shader, but it is not done in this sample
			GLint positionAttribute = glGetAttribLocation(program, positionAttributeName.c_str());
			CHECK_ERROR();
			GLint uvAttribute = glGetAttribLocation(program, uvAttributeName.c_str());
			CHECK_ERROR();
			GLint normalAttribute = glGetAttribLocation(program, normalAttributeName.c_str());
			CHECK_ERROR();
			GLint colorAttribute = glGetAttribLocation(program, colorAttributeName.c_str());
			CHECK_ERROR();

			// Only create a new shader if all the attributes are found.
			if (positionAttribute != -1
			    && uvAttribute != -1
			    && normalAttribute != -1
			    && colorAttribute != -1) {

				shader = new ObjShader(
						program,
						positionAttribute,
						uvAttribute,
						normalAttribute,
						colorAttribute);
			} else {
				glDeleteProgram(program);
				CHECK_ERROR();
			}
		}
	}

	// The shaders are no longer needed once the program is linked. Release their memory.
	glDeleteShader(vertexShader);
	CHECK_ERROR();
	glDeleteShader(fragmentShader);
	CHECK_ERROR();

	return shader;
}

/*void ObjShader::setupBuffers(Model &model) const {
	glGenVertexArrays(1, &model.vao);
	CHECK_ERROR();
	glBindVertexArray(model.vao);
	CHECK_ERROR();

	glGenBuffers(1, &model.vertex_buffer);
	CHECK_ERROR();
	glBindBuffer(GL_ARRAY_BUFFER, model.vertex_buffer);
	CHECK_ERROR();
	const Vertex *vertices = model.vertices.data();
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * model.vertices.size(), vertices,
	             GL_STATIC_DRAW);
	CHECK_ERROR();

	glGenBuffers(1, &model.index_buffer);
	CHECK_ERROR();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.index_buffer);
	CHECK_ERROR();
	const Index *indices = model.indices.data();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * model.indices.size(), indices,
	             GL_STATIC_DRAW);
	CHECK_ERROR();

	setupVertexAttribs();
}*/

void ObjShader::setupVertexAttribs() const {
	// The position attribute is 3 floats
	glVertexAttribPointer(
			position, // attrib
			3, // elements
			GL_FLOAT, // of type float
			GL_FALSE, // don't normalize
			sizeof(Vertex), // stride is Vertex bytes
			0 // pull from the start of the vertex data
	);
	CHECK_ERROR();
	glEnableVertexAttribArray(position);
	CHECK_ERROR();

	// The uv attribute is 2 floats
	glVertexAttribPointer(
			uv, // attrib
			2, // elements
			GL_FLOAT, // of type float
			GL_FALSE, // don't normalize
			sizeof(Vertex), // stride is Vertex bytes
			(uint8_t*) sizeof(vec3) // offset vec3 from the start
	);
	CHECK_ERROR();
	glEnableVertexAttribArray(uv);
	CHECK_ERROR();

	// The normal attribute is 3 floats
	glVertexAttribPointer(
			normal, // attrib
			3, // elements
			GL_FLOAT, // of type float
			GL_FALSE, // don't normalize
			sizeof(Vertex), // stride is Vertex bytes
			(uint8_t*) (sizeof(vec3) + sizeof(vec2)) // offset vec3 + vec2 from the start
	);
	CHECK_ERROR();
	glEnableVertexAttribArray(normal);
	CHECK_ERROR();

	// The color attribute is 4 bytes
	glVertexAttribPointer(
			color, // attrib
			4, // elements
			GL_UNSIGNED_BYTE, // of type byte
			GL_TRUE, // normalize
			sizeof(Vertex), // stride is Vertex bytes
			(uint8_t*) (sizeof(vec3) * 2 + sizeof(vec2)) // offset vec3 * 2 + vec2 from the start
	);
	CHECK_ERROR();
	glEnableVertexAttribArray(color);
	CHECK_ERROR();
}

void ObjShader::drawObject(const Model* model, const TextureAsset* texture) const {
	// Setup the texture
	glActiveTexture(GL_TEXTURE0);
	CHECK_ERROR();
	glBindTexture(GL_TEXTURE_2D, texture->textureID);
	CHECK_ERROR();

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