#pragma once
namespace lighting {
	class Material {
		float m_metalicity;
		float m_roughness;
		glm::vec3 m_F0;
		glm::vec3 m_baseColor;

	public:
		Material();
		Material(const Material& other);
		Material(float metalicity, float roughness, glm::vec3 baseColor, glm::vec3 F0);
		float getRoughness() const;
		float getMetalicity() const;
		glm::vec3 getBaseColor() const;
		glm::vec3 getF0() const;
		void setMetalicity(float metalicity);
		void setRoughness(float roughness);
		void setBaseColor(const glm::vec3& baseColor);
		void setF0(const glm::vec3& F0);
		void setF0(float f0);
		~Material();
	};
}