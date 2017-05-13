#include "stdafx.h"
#include "Spotlight.h"
#include "MathConstants.h"
#include "OGLHelpers.h"

namespace lighting {
	using namespace math;

	Spotlight::Spotlight() {
		setPosition(glm::vec3(0.0f, 0.0f, 1.0f));
		setTarget(glm::vec3(0.0f));
		setAperture(30.0f * TO_RADIANS);
		m_near = 0.1f;
		m_far = 100.0f;
		m_useShadowMap = false;
		m_useStencil = false;
		m_fbo_id = 0;
		m_depth_texture_id = 0;
		m_resolution = 1024;
	}

	Spotlight::Spotlight(const Spotlight& other) {
		m_position = other.m_position;
		m_target = other.m_target;
		m_aperture = other.m_aperture;
		m_stencil = other.m_stencil;
		m_near = other.m_near;
		m_far = other.m_far;
		m_useShadowMap = other.m_useShadowMap;
		m_useStencil = other.m_useStencil;
		m_fbo_id = other.m_fbo_id;
		m_depth_texture_id = other.m_depth_texture_id;
		m_resolution = other.m_resolution;
	}

	Spotlight::Spotlight(const glm::vec3& position, const glm::vec3& target, float aperture) {
		setPosition(position);
		setTarget(target);
		setAperture(aperture);
		m_near = 0.1f;
		m_far = 100.0f;
		m_useShadowMap = false;
		m_useStencil = false;
		m_fbo_id = 0;
		m_depth_texture_id = 0;
		m_resolution = 1024;
	}


	Spotlight::Spotlight(const glm::vec3& position, const glm::vec3& target, float aperture, const glm::vec3& color, float intensity, float ratio) {
		setPosition(position);
		setTarget(target);
		setAperture(aperture);
		m_near = 0.1f;
		m_far = 100.0f;
		m_useShadowMap = false;
		m_useStencil = false;
		m_fbo_id = 0;
		m_depth_texture_id = 0;
		m_resolution = 1024;
	}

	Spotlight::~Spotlight() {
		if (m_useShadowMap) {
			glDeleteTextures(1, &m_depth_texture_id);
			glDeleteFramebuffers(1, &m_fbo_id);
		}
	}

	GLuint Spotlight::getStencil() const {
		return m_useStencil ? m_stencil.get_id() : 0;
	}

	void Spotlight::setStencil(const image::Texture& stencil) {
		m_stencil = stencil;
		m_useStencil = true;
	}

	void Spotlight::setDepthRange(float nearDistance, float farDistance) {
		assert(nearDistance > 0.0f);
		assert(farDistance > 0.0);
		assert(nearDistance < farDistance);
		m_near = nearDistance;
		m_far = farDistance;
	}

	GLuint Spotlight::getFBO() const {
		return m_fbo_id;
	}

	GLuint Spotlight::getShadowMap() const {
		return m_depth_texture_id;
	}

	int Spotlight::getShadowResolution() const {
		return m_resolution;
	}

	void Spotlight::createShadowBuffer() {
		glGenTextures(1, &m_depth_texture_id);
		glBindTexture(GL_TEXTURE_2D, m_depth_texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, m_resolution, m_resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glm::vec4 borderColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(borderColor));
		glBindTexture(GL_TEXTURE_2D, 0);

		//Create framebuffer second and attach the texture to it
		glGenFramebuffers(1, &m_fbo_id);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_id);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depth_texture_id, 0);

		if (ogl::framebufferStatus()) {
			m_useShadowMap = true;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Spotlight::createTexturesGPU() {
		if (m_useStencil) {
			m_stencil.send_to_gpu();
		}
	}

	glm::vec3 Spotlight::getPosition() const {
		return m_position;
	}

	float Spotlight::getAperture() const {
		return m_aperture;
	}

	glm::vec3 Spotlight::getTarget() const {
		return m_target;
	}

	glm::vec3 Spotlight::getDirection() const {
		return glm::normalize(m_target - m_position);
	}

	void Spotlight::setPosition(const glm::vec3& position) {
		m_position = position;
	}

	void Spotlight::setTarget(const glm::vec3& target) {
		m_target = target;
	}

	void Spotlight::setDirection(const glm::vec3& direction) {
		m_target = m_position - direction;
	}

	void Spotlight::setAperture(float aperture) {
		m_aperture = glm::clamp(aperture, 0.0f, 90.0f * TO_RADIANS);
	}

	void Spotlight::setLookAt(const glm::vec3& position, const glm::vec3& target, float aperture) {
		setPosition(position);
		setTarget(target);
		setAperture(aperture);
	}

	glm::mat4 Spotlight::getP() const {
		return glm::perspective(m_aperture, 1.0f, m_near, m_far);
	}

	glm::mat4 Spotlight::getM() const {
		return glm::lookAt(m_position, m_target, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::mat4 Spotlight::getPM() const {
		return getP() * getM();
	}
}