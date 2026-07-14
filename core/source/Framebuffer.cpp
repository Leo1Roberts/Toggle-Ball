#include "Framebuffer.h"
#include <stdexcept>
#include <utility>

void Framebuffer::invalidate() {
	// 1. Setup Resolved Single-Sample Target (Standard 2D Texture)
	GLFramebuffer newResolveFBO;
	glGenFramebuffers(1, newResolveFBO.ptr());
	glBindFramebuffer(GL_FRAMEBUFFER, newResolveFBO);

	GLTexture newColorAttachment;
	glGenTextures(1, newColorAttachment.ptr());
	glBindTexture(GL_TEXTURE_2D, newColorAttachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, config.width, config.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, newColorAttachment, 0);

	GLFramebuffer newMsaaFBO;
	GLRenderbuffer newMsaaColor;
	GLRenderbuffer newDepthAttachment;

	if (config.samples > 1) {
		glGenFramebuffers(1, newMsaaFBO.ptr());
		glBindFramebuffer(GL_FRAMEBUFFER, newMsaaFBO);

		glGenRenderbuffers(1, newMsaaColor.ptr());
		glBindRenderbuffer(GL_RENDERBUFFER, newMsaaColor);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, config.samples, GL_RGBA8, config.width, config.height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, newMsaaColor);

		glGenRenderbuffers(1, newDepthAttachment.ptr());
		glBindRenderbuffer(GL_RENDERBUFFER, newDepthAttachment);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, config.samples, GL_DEPTH24_STENCIL8, config.width, config.height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, newDepthAttachment);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			throw std::runtime_error("Failed to create MSAA framebuffer");
		}
	} else {
		glGenRenderbuffers(1, newDepthAttachment.ptr());
		glBindRenderbuffer(GL_RENDERBUFFER, newDepthAttachment);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, config.width, config.height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, newDepthAttachment);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			throw std::runtime_error("Failed to create single-sample framebuffer");
	}

	unbind();

	msaaFBO = std::move(newMsaaFBO);
	msaaColorBuffer = std::move(newMsaaColor);
	msaaDepthBuffer = std::move(newDepthAttachment);
	resolveFBO = std::move(newResolveFBO);
	colorAttachment = std::move(newColorAttachment);
}

void Framebuffer::resolve() const {
	if (config.samples > 1) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);

		glBlitFramebuffer(0, 0, config.width, config.height,
		                  0, 0, config.width, config.height,
		                  GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

bool Framebuffer::resize(int width, int height) {
	if (width == 0 || height == 0 || (config.width == width && config.height == height))
		return false;

	config.width = width;
	config.height = height;
	invalidate();
	return true;
}