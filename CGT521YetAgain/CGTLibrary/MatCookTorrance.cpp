#include "stdafx.h"
#include "MathConstants.h"
#include "MatCookTorrance.h"

namespace lighting {
	MatCookTorrance::MatCookTorrance() : MatCookTorrance(glm::vec3(0.5f)) {
		
	}

	MatCookTorrance::MatCookTorrance(const MatCookTorrance& other) {
		m_Ka = other.m_Ka;
		m_Kd = other.m_Kd;
		m_Ks = other.m_Ks;
		m_eta = other.m_eta;
		m_m = other.m_m;
	}

	MatCookTorrance::MatCookTorrance(const glm::vec3& ambientAndDiffuse) {
		setKa(0.25f * ambientAndDiffuse);
		setKd(ambientAndDiffuse);
		setKs(glm::vec3(0.5f));
		setRefractionIndex(3.2f);
		setMicrofacetSlope(0.3f);
	}

	MatCookTorrance::MatCookTorrance(const glm::vec3& Ka, const glm::vec3& Kd, const glm::vec3& Ks, float eta, float m) {
		setKa(Ka);
		setKd(Kd);
		setKs(Ks);
		setRefractionIndex(eta);
		setMicrofacetSlope(m);
	}

	void MatCookTorrance::setKa(const glm::vec3& Ka) {
		m_Ka = glm::clamp(Ka, glm::vec3(0.0f), glm::vec3(1.0f));
	}

	void MatCookTorrance::setKd(const glm::vec3& Kd) {
		m_Kd = glm::clamp(Kd, glm::vec3(0.0f), glm::vec3(1.0f));
	}

	void MatCookTorrance::setKs(const glm::vec3& Ks) {
		m_Ks = glm::clamp(Ks, glm::vec3(0.0f), glm::vec3(1.0f));
	}

	void MatCookTorrance::setRefractionIndex(float eta) {
		m_eta = glm::clamp(eta, 0.0f, 5.0f);
	}

	void MatCookTorrance::setMicrofacetSlope(float m) {
		m_m = glm::clamp(m, 0.0f, math::PI);
	}

	glm::vec3 MatCookTorrance::getKa() const {
		return m_Ka;
	}

	glm::vec3 MatCookTorrance::getKd() const {
		return m_Kd;
	}

	glm::vec3 MatCookTorrance::getKs() const {
		return m_Ks;
	}

	float MatCookTorrance::getRefractionIndex() const {
		return m_eta;
	}

	float MatCookTorrance::getMicrofacetSlope() const {
		return m_m;
	}

	MatCookTorrance::~MatCookTorrance() {

	}
}
