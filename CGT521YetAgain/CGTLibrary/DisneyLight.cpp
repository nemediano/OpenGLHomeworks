#include "stdafx.h"
#include "DisneyLight.h"

namespace lighting {
	DisneyLight::DisneyLight() : DisneyLight(1.0f, glm::vec3(1.0f)) {

	}

	DisneyLight::DisneyLight(float intensity, glm::vec3 color) {
		setColor(color);
		setIntensity(intensity);
	}

	DisneyLight::~DisneyLight() {

	}

	void DisneyLight::setColor(const glm::vec3& color) {
		m_color = glm::clamp(color, glm::vec3(0.0f), glm::vec3(1.0f));
	}

	void DisneyLight::setIntensity(float intensity) {
		m_intensity = glm::clamp(intensity, 0.0f, 1.0f);
	}

	float DisneyLight::getIntensity() const {
		return m_intensity;
	}

	glm::vec3 DisneyLight::getColor() const {
		return m_color;
	}

}