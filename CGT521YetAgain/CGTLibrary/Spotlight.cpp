#include "stdafx.h"
#include "Spotlight.h"
#include "MathConstants.h"


namespace lighting {
	using namespace math;

	Spotlight::Spotlight() : Light() {
		setPosition(glm::vec3(0.0f, 0.0f, 1.0f));
		setTarget(glm::vec3(0.0f));
		setAperture(30.0f * TO_RADIANS);
	}

	Spotlight::Spotlight(const Spotlight& other) {
		m_position = other.m_position;
		m_target = other.m_target;
		m_aperture = other.m_aperture;
		m_intensity = other.m_intensity;
		m_color = other.m_color;
		m_ratio = other.m_ratio;
	}

	Spotlight::Spotlight(const glm::vec3& position, const glm::vec3& target, float aperture) : Light() {
		setPosition(position);
		setTarget(target);
		setAperture(aperture);
		setRatio(0.5f);
	}

	Spotlight::Spotlight(const glm::vec3& color, float intensity, float ratio) {
		setPosition(glm::vec3(0.0f, 0.0f, 1.0f));
		setTarget(glm::vec3(0.0f));
		setAperture(30.0f * TO_RADIANS);
		setColor(color);
		setIntensity(intensity);
		setRatio(ratio);
	}

	Spotlight::Spotlight(const glm::vec3& position, const glm::vec3& target, float aperture, const glm::vec3& color, float intensity, float ratio) {
		setPosition(position);
		setTarget(target);
		setAperture(aperture);
		setColor(color);
		setIntensity(intensity);
		setRatio(ratio);
	}

	glm::vec3 Spotlight::getPosition() const {
		return m_position;
	}

	float Spotlight::getAperture() const {
		return m_aperture;
	}

	float Spotlight::getRatio() const {
		return m_ratio;
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

	void Spotlight::setRatio(float ratio) {
		m_ratio = glm::clamp(ratio, 0.0f, 1.0f);
	}

	void Spotlight::setLookAt(const glm::vec3& position, const glm::vec3& target, float aperture) {
		setPosition(position);
		setTarget(target);
		setAperture(aperture);
	}

	glm::mat4 Spotlight::getP() const {
		return glm::perspective(m_aperture, 1.0f, 0.1f, 100.0f);
	}

	glm::mat4 Spotlight::getM() const {
		return glm::lookAt(m_position, m_target, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::mat4 Spotlight::getPM() const {
		return getP() * getM();
	}
}