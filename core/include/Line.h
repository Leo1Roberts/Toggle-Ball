#ifndef LINE_H
#define LINE_H

struct LineVertex {
	constexpr LineVertex(const vec3& iPosition, const col& iColor) : position(iPosition),
	                                                                 color(iColor) {}

	LineVertex() : position(0.0f, 0.0f, 0.0f),
	               color(BLACK) {}

	vec3 position;
	col color;
};

struct Line {
	static GLuint program;
	static GLint position;
	static GLint color;
	static GLint projectionMatrix;

	static GLuint vao;
	static GLuint vertex_buffer;
	static LineVertex vertices[];

	static int numLines;

	static mat4 projMat;

	static void activateShader() { glUseProgram(program); }

	static void drawLines();

	static bool initLineShader();

	static bool addLine(const vec3& from, const vec3& to, const col& lineColor);
};


#endif// LINE_H
