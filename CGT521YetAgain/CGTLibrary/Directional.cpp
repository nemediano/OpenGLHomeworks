#include "stdafx.h"
#include "Directional.h"
#include "Geometry.h"

namespace lighting {
	using namespace math;
	using glm::vec3;

	Directional::Directional() {
		setDirection(vec3(0.0f, 0.0f, -1.0f));
	}

	Directional::Directional(const Directional& other) {
		m_direction = other.m_direction;
	}

	Directional::Directional(const glm::vec3 direction) {
		setDirection(direction);
	}

	Directional::~Directional() {

	}

	void Directional::setDirection(const glm::vec3 direction) {
		if (!almostZero(direction)) {
			m_direction = glm::normalize(direction);
		}
	}

	glm::vec3 Directional::getDirection() const {
		return m_direction;
	}

	void Directional::rotate(const glm::vec3 eulerAngles) {
		rotate(eulerAngles.x, eulerAngles.y, eulerAngles.z);
	}

	void Directional::rotate(const glm::quat rotation) {
		glm::quat rot = glm::normalize(rotation);
		m_direction = vec3(glm::mat4_cast(rot) * glm::vec4(m_direction, 0.0f));
	}

	void Directional::rotate(float angleX, float angleY, float angleZ) {
		m_direction = vec3(glm::eulerAngleYXZ(angleY, angleX, angleZ) * glm::vec4(m_direction, 0.0f));
	}
}