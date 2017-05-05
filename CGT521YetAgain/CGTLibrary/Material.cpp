#include "stdafx.h"
#include "Material.h"

namespace lighting {
	Material::Material() : Material(0.0f, 0.5f, glm::vec3(0.85f), glm::vec3(1.0f)) {

	}

	Material::Material(const Material& other) {
		m_baseColor = other.m_baseColor;
		m_F0 = other.m_F0;
		m_roughness = other.m_roughness;
		m_metalicity = other.m_metalicity;
	}

	Material::Material(float metalicity, float rougness, glm::vec3 baseColor, glm::vec3 F0) {
		setMetalicity(metalicity);
		setRoughness(rougness);
		setBaseColor(baseColor);
		setF0(F0);
	}

	float Material::getRoughness() const {
		return m_roughness;
	}

	float Material::getMetalicity() const {
		return m_metalicity;
	}

	glm::vec3 Material::getBaseColor() const {
		return m_baseColor;
	}

	glm::vec3 Material::getF0() const {
		return m_F0;
	}

	void Material::setMetalicity(float Metalicity) {
		m_metalicity = glm::clamp(Metalicity, 0.0f, 1.0f);
	}

	void Material::setRoughness(float roughness) {
		m_roughness = glm::clamp(roughness, 0.0f, 1.0f);
	}

	void Material::setBaseColor(const glm::vec3& baseColor) {
		m_baseColor = glm::clamp(baseColor, glm::vec3(0.0f), glm::vec3(1.0f));
	}

	void Material::setF0(const glm::vec3& F0) {
		m_F0 = glm::clamp(F0, glm::vec3(0.0f), glm::vec3(1.0f));
	}

	void Material::setF0(float f0) {
		m_F0 = glm::clamp(glm::vec3(f0), glm::vec3(0.0f), glm::vec3(1.0f));
	}

	Material::~Material() {

	}
}