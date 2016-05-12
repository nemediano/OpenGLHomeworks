#include "Material.h"

namespace scene {
	Material::Material() {
		setKa(glm::vec3(0.25f, 0.25f, 0.25f));
		setKd(glm::vec3(0.5f, 0.5f, 0.5f));
		setKs(glm::vec3(1.0f, 1.0f, 1.0f));
		setShininess(8.0f);
		setName("");
	}

	Material::Material(const glm::vec3& Ka, const glm::vec3& Kd, const glm::vec3& Ks, const float& shininess) {
		setKa(Ka);
		setKd(Kd);
		setKs(Ks);
		setShininess(shininess);
		setName("");
	}

	Material::Material(const glm::vec3& Ka, const glm::vec3& Kd, const glm::vec3& Ks, const float& shininess, const std::string& name) {
		setKa(Ka);
		setKd(Kd);
		setKs(Ks);
		setShininess(shininess);
		setName(name);
	}
	
	Material::Material(const glm::vec3& Ka, const glm::vec3& Kd, const glm::vec3& Ks, const std::string& name) {

	}
	
	Material::Material(const glm::vec3& Ka, const glm::vec3& Kd, const glm::vec3& Ks, const float& refraction, const float& slope, const std::string& name) {

	}

	Material::Material(const glm::vec3& Ka, const glm::vec3& Kd, const glm::vec3& Ks, const float& refraction, const float& slope) {

	}

	Material::Material(const glm::vec3& Ka, const glm::vec3& Kd, const glm::vec3& Ks) {
		setKa(Ka);
		setKd(Kd);
		setKs(Ks);
		setShininess(1.0f);
		setName("");
	}

	Material::Material(const glm::vec3& Ka, const glm::vec3& Kd) {
		setKa(Ka);
		setKd(Kd);
		setKs(glm::vec3(1.0f, 1.0f, 1.0f));
		setShininess(1.0f);
		setName("");
	}

	Material::Material(const glm::vec3& ambientAndDiffuse, const glm::vec3& Ks, const float& shininess) {
		setKa(ambientAndDiffuse);
		setKd(ambientAndDiffuse);
		setKs(Ks);
		setShininess(shininess);
		setName("");
	}

	Material::Material(const glm::vec3& ambientAndDiffuse, const float& shininess) {
		setKa(ambientAndDiffuse);
		setKd(ambientAndDiffuse);
		setKs(glm::vec3(1.0f, 1.0f, 1.0f));
		setShininess(shininess);
		setName("");
	}

	Material::Material(const glm::vec3& ambientAndDiffuse) {
		setKa(ambientAndDiffuse);
		setKd(ambientAndDiffuse);
		setKs(glm::vec3(1.0f, 1.0f, 1.0f));
		setShininess(1.0f);
		setName("");
	}
	//Getter and setters
	glm::vec3 Material::getKa() const {
		return m_Ka;
	}

	glm::vec3 Material::getKd() const {
		return m_Kd;
	}

	glm::vec3 Material::getKs() const {
		return m_Ks;
	}

	float Material::getShininnes() const {
		return m_shininess;
	}

	float Material::getRoughness() const {
		return m_slope;
	}

	float Material::getRefraction() const {
		return m_refraction;
	}

	std::string Material::getName() const {
		return m_name;
	}

	void Material::setKa(const glm::vec3& Ka) {
		if (glm::all(glm::greaterThanEqual(Ka, glm::vec3(0.0f)))) {
			m_Ka = Ka;
		} else {
			m_Ka = glm::vec3(0.0f);
		}
	}

	void Material::setKd(const glm::vec3& Kd) {
		if (glm::all(glm::greaterThanEqual(Kd, glm::vec3(0.0f)))) {
			m_Kd = Kd;
		} else {
			m_Kd = glm::vec3(0.0f);
		}
	}

	void Material::setKs(const glm::vec3& Ks) {
		if (glm::all(glm::greaterThanEqual(Ks, glm::vec3(0.0f)))) {
			m_Ks = Ks;
		}
		else {
			m_Ks = glm::vec3(0.0f);
		}
	}

	void Material::setShininess(const float& shininess) {
		if (shininess >= 0.0 && shininess <= 256.0) {
			m_shininess = shininess;
		} else {
			m_shininess = 1.0f;
		}
	}
	void Material::setRefraction(const float& refraction) {
		if (refraction >= 0.0 && refraction <= 256.0) {
			m_refraction = refraction;
		}
		else {
			m_refraction = 0.0f;
		}
	}

	void Material::setRoghness(const float& rougness) {
		if (rougness >= 0.0 && rougness <= 256.0) {
			m_slope = rougness;
		}
		else {
			m_slope = 1.0f;
		}
	}

	void Material::setName(const std::string& name) {
		m_name = name;
	}
}
