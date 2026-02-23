#ifndef OBJ_SHADER_H
#define OBJ_SHADER_H

struct Model;

/*!
 * A class representing a simple shader program. It consists of vertex and fragment components. The
 * input attributes are a position (as a Vector3) and a uv (as a Vector2). It also takes a uniform
 * to be used as the entire model/view/projection matrix. The shader expects a single texture for
 * fragment shading, and does no other lighting calculations (thus no uniforms for lights or normal
 * attributes).
 */
struct ObjShader {
	/*!
	 * Loads a shader given the full sourcecode and names for necessary attributes and uniforms to
	 * link to. Returns a valid shader on success or null on failure. ObjShader resources are
	 * automatically cleaned up on destruction.
	 *
	 * @param vertexSource The full source code for your vertex program
	 * @param fragmentSource The full source code of your fragment program
	 * @param positionAttributeName The name of the position attribute in your vertex program
	 * @param uvAttributeName The name of the uv coordinate attribute in your vertex program
	 * @param projectionMatrixUniformName The name of your model/view/projection matrix uniform
	 * @return a valid ObjShader on success, otherwise null.
	 */
	static ObjShader* loadObjShader(
			const std::string& vertexSource,
			const std::string& fragmentSource,
			const std::string& positionAttributeName,
			const std::string& uvAttributeName,
			const std::string& normalAttributeName,
			const std::string& colorAttributeName);

	inline ~ObjShader() {
		if (program) {
			glDeleteProgram(program);
			program = 0;
		}
	}

	static void deleteShaders();

	void activate() const { glUseProgram(program); };

	void setupVertexAttribs() const;

	//void setupBuffers(Model &model) const;

	/*!
	 * Renders a single object
	 * @param object a object to render
	 */
	void drawObject(const Model* model, const TextureAsset* texture) const;

	GLint getUniformLocation(const std::string& name) const;

private:

	constexpr ObjShader(
			GLuint iProgram,
			GLint iPosition,
			GLint iUv,
			GLint iNormal,
			GLint iColor)
			: program(iProgram),
			  position(iPosition),
			  uv(iUv),
			  normal(iNormal),
			  color(iColor) {}

	GLuint program;
	GLint position;
	GLint uv;
	GLint normal;
	GLint color;
};

extern ObjShader* objShader;
extern ObjShader* outlineShader;

#endif // OBJ_SHADER_H