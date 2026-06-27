#ifndef GL_UTILITIES_H
#define GL_UTILITIES_H

#include "main.h"

struct BufferDeleter {
	void operator()(GLsizei n, const GLuint* ids) const { glDeleteBuffers(n, ids); }
};
struct VertexArrayDeleter {
	void operator()(GLsizei n, const GLuint* ids) const { glDeleteVertexArrays(n, ids); }
};
struct ProgramDeleter {
	void operator()(GLsizei, const GLuint* id) const { glDeleteProgram(*id); }
};
struct ShaderDeleter {
	void operator()(GLsizei, const GLuint* id) const { glDeleteShader(*id); }
};
struct TextureDeleter {
	void operator()(GLsizei n, const GLuint* ids) const { glDeleteTextures(n, ids); }
};

template <typename T, typename Deleter>
class GLHandle {
public:
	GLHandle() = default;
	explicit GLHandle(T id) : id(id) {}

	~GLHandle() { if (id) Deleter{}(1, &id); }

	GLHandle(const GLHandle&) = delete;
	GLHandle& operator=(const GLHandle&) = delete;

	GLHandle(GLHandle&& other) noexcept : id(other.id) { other.id = 0; }
	GLHandle& operator=(GLHandle&& other) noexcept {
		if (this != &other) {
			if (id) Deleter{}(1, &id);
			id = other.id;
			other.id = 0;
		}
		return *this;
	}

	operator T() const { return id; }
	T* ptr() { return &id; }

private:
	T id = 0;
};

using GLBuffer = GLHandle<GLuint, BufferDeleter>;
using GLVertexArray = GLHandle<GLuint, VertexArrayDeleter>;
using GLShader = GLHandle<GLuint, ShaderDeleter>;
using GLProgram = GLHandle<GLuint, ProgramDeleter>;
using GLTexture = GLHandle<GLuint, TextureDeleter>;

#endif // GL_UTILITIES_H