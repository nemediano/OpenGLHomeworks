#include "stdafx.h"
#include "Camera.h"

namespace camera {

	Camera::Camera() {
		m_position = glm::vec3(0.0f, 0.0f, 2.0f);
		m_target = glm::vec3(0.0f, 0.0f, 0.0f);
		m_up = glm::vec3(0.0f, 1.0f, 0.0f);
		m_aperture = PI / 8.0f;
		m_near = 1.0f;
		m_far = 3.0f;
		m_aspect = 1.0f;
	}

	Camera::~Camera() {
	}

	Camera::Camera(const glm::vec3& position, const glm::vec3& target) : Camera() {
		m_position = position;
		m_target = target;
	}

	void Camera::setLookAt(const glm::vec3& position, const glm::vec3& target) {
		setPosition(position);
		setTarget(target);
	}
	
	void Camera::setPosition(const glm::vec3& position) {
		m_position = position;
	}

	glm::vec3 Camera::getPosition() const {
		return m_position;
	}

	void Camera::setTarget(const glm::vec3& target) {
		m_target = target;
	}

	glm::vec3 Camera::getTarget() const {
		return m_target;
	}

	void Camera::setAspectRatio(float aspect) {
		m_aspect = glm::clamp(aspect, 0.01f, 3000.0f);
	}

	void Camera::setAspectRatio(int width, int height) {
		m_aspect = fabs(height) < 0.01 ? 1.0f : float(width) / float(height);
	}

	float Camera::getAspectRatio() const {
		return m_aspect;
	}

	void Camera::setFovY(float fov_y) {
		const float EPSILON = 0.001f;
		m_aperture = glm::clamp(fov_y, 0.000001f, PI - EPSILON);
	}

	void Camera::addFovY(float deltaAngle) {
		setFovY(getFovY() + deltaAngle);
	}

	float Camera::getFovY() const {
		return m_aperture;
	}

	void Camera::setNear(float Near) {
		m_near = Near;
	}

	float Camera::getNear() const {
		return m_near;
	}

	void Camera::setFar(float Far) {
		m_far = Far;
	}

	float Camera::getFar() const {
		return m_far;
	}

	void Camera::setDepthView(float Near, float Far) {
		m_near = Near;
		m_far = Far;
	}

	float Camera::getDepthView() const {
		return (m_far - m_near);
	}

	glm::mat4 Camera::getViewMatrix() const {
		return glm::lookAt(m_position, m_target, m_up);
	}

	glm::mat4 Camera::getProjectionMatrix() const {
		return glm::perspective(m_aperture, m_aspect, m_near, m_far);
	}

	glm::mat4 Camera::getPVMatrix() const {
		glm::mat4 V = glm::lookAt(m_position, m_target, m_up);
		glm::mat4 P = glm::perspective(m_aperture, m_aspect, m_near, m_far);

		return P * V;
	}

}
