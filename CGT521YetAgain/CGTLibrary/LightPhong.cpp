#include "stdafx.h"
#include "LightPhong.h"

namespace lighting {
	LightPhong::LightPhong() : LightPhong(glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(1.0f)) {

	}

	LightPhong::LightPhong(const LightPhong& other) {
		m_La = other.m_La;
		m_Ls = other.m_Ls;
		m_Ld = other.m_Ld;
	}

	LightPhong::LightPhong(const glm::vec3& La, const glm::vec3& Ld, const glm::vec3& Ls) {
		setLa(La);
		setLd(Ld);
		setLs(Ls);
	}

	void LightPhong::setLa(const glm::vec3& La) {
		m_La = glm::clamp(La, glm::vec3(0.0f), glm::vec3(1.0f));
	}

	void LightPhong::setLd(const glm::vec3& Ld) {
		m_Ld = glm::clamp(Ld, glm::vec3(0.0f), glm::vec3(1.0f));
	}

	void LightPhong::setLs(const glm::vec3& Ls) {
		m_Ls = glm::clamp(Ls, glm::vec3(0.0f), glm::vec3(1.0f));
	}

	glm::vec3 LightPhong::getLa() const {
		return m_La;
	}

	glm::vec3 LightPhong::getLd() const {
		return m_Ld;
	}

	glm::vec3 LightPhong::getLs() const {
		return m_Ls;
	}

	LightPhong::~LightPhong() {

	}
}