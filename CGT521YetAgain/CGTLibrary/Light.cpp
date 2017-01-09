#include "stdafx.h"
#include "Light.h"

namespace lighting {
	Light::Light() : Light(1.0f, glm::vec3(1.0f)) {

	}

	Light::Light(float intensity, glm::vec3 color) {
		setColor(color);
		setIntensity(intensity);
	}

	Light::~Light() {

	}

	void Light::setColor(const glm::vec3& color) {
		m_color = glm::clamp(color, glm::vec3(0.0f), glm::vec3(1.0f));
	}

	void Light::setIntensity(float intensity) {
		m_intensity = glm::clamp(intensity, 0.0f, 1.0f);
	}

	float Light::getIntensity() const {
		return m_intensity;
	}

	glm::vec3 Light::getColor() const {
		return m_color;
	}

}