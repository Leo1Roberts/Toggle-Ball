#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "opengl/GLUtilities.h"

struct FramebufferConfig {
	int width;
	int height;
	int samples = 4;
};

class Framebuffer {
public:
	Framebuffer() = default;
	Framebuffer(const FramebufferConfig& cfg) : config(cfg) {
		invalidate();
	}

	Framebuffer(const Framebuffer&) = delete;
	Framebuffer& operator=(const Framebuffer&) = delete;

	void bind() const {
		glBindFramebuffer(GL_FRAMEBUFFER, config.samples > 1 ? msaaFBO : resolveFBO);
		glViewport(0, 0, config.width, config.height);
	}

	static void unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	bool resize(int width, int height);

	void setConfig(const FramebufferConfig& cfg) {
		config = cfg;
		invalidate();
	}

	[[nodiscard]] int getWidth() const { return config.width; }
	[[nodiscard]] int getHeight() const { return config.height; }

	[[nodiscard]] const GLTexture& getTexture() const {
		if (config.samples > 1)
			resolve();
		return colorAttachment;
	}

private:
	void invalidate();

	void resolve() const;

	GLFramebuffer msaaFBO;
	GLRenderbuffer msaaColorBuffer;
	GLRenderbuffer msaaDepthBuffer;

	GLFramebuffer resolveFBO;
	GLTexture colorAttachment;

	FramebufferConfig config;
};

#endif // FRAMEBUFFER_H