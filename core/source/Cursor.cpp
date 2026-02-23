#include "main.h"
#include "Sizes.h"
#include "MatrixUtilities.h"
#include "Cursor.h"

TextureAsset *Cursor::arrowTex, *Cursor::handTex, *Cursor::resizeTex;
TextureAsset* Cursor::tex;
vec2 Cursor::pos;
float Cursor::angle;
float Cursor::size;
bool Cursor::visible;

GLuint Cursor::program;
GLint Cursor::position;
GLint Cursor::uv;
GLint Cursor::projectionMatrix;

GLuint Cursor::vao;
GLuint Cursor::vertex_buffer;
CursorVertex Cursor::vertices[6];

bool Cursor::initShader(
        const std::string& vertexSource,
        const std::string& fragmentSource,
        const std::string& positionAttributeName,
        const std::string& uvAttributeName,
        const std::string& projectionMatrixUniformName) {
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
			GLint positionAttribute = glGetAttribLocation(iProgram, positionAttributeName.c_str());
			GLint uvAttribute = glGetAttribLocation(iProgram, uvAttributeName.c_str());
			GLint projectionMatrixUniform = glGetUniformLocation(iProgram,
			                                                     projectionMatrixUniformName.c_str());

			if (positionAttribute != -1 && uvAttribute != -1 && projectionMatrixUniform != -1) {
				program = iProgram;
				position = positionAttribute;
				uv = uvAttribute;
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

	// The position attribute is 2 floats
	glVertexAttribPointer(
	        position,            // attrib
	        2,                   // elements
	        GL_FLOAT,            // of type float
	        GL_FALSE,            // don't normalize
	        sizeof(CursorVertex),// stride is CursorVertex bytes
	        0                    // pull from the start of the vertex data
	);
	glEnableVertexAttribArray(position);

	// The uv attribute is 2 floats
	glVertexAttribPointer(
	        uv,                   // attrib
	        2,                    // elements
	        GL_FLOAT,             // of type float
	        GL_FALSE,             // don't normalize
	        sizeof(CursorVertex), // stride is CursorVertex bytes
	        (uint8_t*)sizeof(vec2)// offset vec3 from the start
	);
	glEnableVertexAttribArray(uv);

#ifdef WINDOWS_VERSION
	arrowTex = TextureAsset::loadAsset("C:/Users/leo/AndroidStudioProjects/ToggleBall/app/src/main/assets/cursors/arrow.png", false);
	handTex = TextureAsset::loadAsset("C:/Users/leo/AndroidStudioProjects/ToggleBall/app/src/main/assets/cursors/hand.png", false);
	resizeTex = TextureAsset::loadAsset("C:/Users/leo/AndroidStudioProjects/ToggleBall/app/src/main/assets/cursors/resize.png", false);
#endif

	tex = arrowTex;
	angle = 0;
	visible = false;

	return true;
}

void Cursor::updateProjectionMatrix() {
	mat4 projMat;
	buildOrthographicMatrix(&projMat, 1.0f, RATIO, -1.0f, 1.0f);
	glUniformMatrix4fv(projectionMatrix, 1, false, projMat.m16);
}

void Cursor::drawCursor() {
	if (!visible) return;

	vertices[0] = vertices[3] = {pos + vec2(cosf(angle + PI * 0.75f), sinf(angle + PI * 0.75f)) * size,
	                             {0.0f, 0.0f}};
	vertices[1] = {pos + vec2(cosf(angle + PI * -0.75f), sinf(angle + PI * -0.75f)) * size,
	               {0.0f, 1.0f}};
	vertices[2] = vertices[4] = {pos + vec2(cosf(angle + PI * -0.25f), sinf(angle + PI * -0.25f)) * size,
	                             {1.0f, 1.0f}};
	vertices[5] = {pos + vec2(cosf(angle + PI * 0.25f), sinf(angle + PI * 0.25f)) * size,
	               {1.0f, 0.0f}};

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex->textureID);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}
