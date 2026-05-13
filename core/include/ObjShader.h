#ifndef OBJ_SHADER_H
#define OBJ_SHADER_H

struct Model;

struct ObjShader {
	static ObjShader* loadObjShader(const std::string& vertexSource, const std::string& fragmentSource);

	inline ~ObjShader() {
		if (program) {
			glDeleteProgram(program);
			program = 0;
		}
	}

	static void deleteShaders();

	void activate() const { glUseProgram(program); };

	void setupVertexAttribs() const;

	void drawObject(const Model* model, const TextureAsset* texture) const;

	GLint getUniformLocation(const std::string& name) const;

private:

	constexpr ObjShader(GLuint iProgram) : program(iProgram) {}

	GLuint program;
};

extern ObjShader* objShader;
extern ObjShader* outlineShader;

#endif // OBJ_SHADER_H