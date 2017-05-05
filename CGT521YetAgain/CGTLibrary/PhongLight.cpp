#include "stdafx.h"
#include "PhongLight.h"

namespace lighting {
	PhongLight::PhongLight() : PhongLight(glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(1.0f)) {

	}

	PhongLight::PhongLight(const PhongLight& other) {
		m_La = other.m_La;
		m_Ls = other.m_Ls;
		m_Ld = other.m_Ld;
	}

	PhongLight::PhongLight(const glm::vec3& La, const glm::vec3& Ld, const glm::vec3& Ls) {
		setLa(La);
		setLd(Ld);
		setLs(Ls);
	}

	void PhongLight::setLa(const glm::vec3& La) {
		m_La = glm::clamp(La, glm::vec3(0.0f), glm::vec3(1.0f));
	}

	void PhongLight::setLd(const glm::vec3& Ld) {
		m_Ld = glm::clamp(Ld, glm::vec3(0.0f), glm::vec3(1.0f));
	}

	void PhongLight::setLs(const glm::vec3& Ls) {
		m_Ls = glm::clamp(Ls, glm::vec3(0.0f), glm::vec3(1.0f));
	}

	glm::vec3 PhongLight::getLa() const {
		return m_La;
	}

	glm::vec3 PhongLight::getLd() const {
		return m_Ld;
	}

	glm::vec3 PhongLight::getLs() const {
		return m_Ls;
	}

	PhongLight::~PhongLight() {

	}
}