#pragma once
namespace lighting {
	class LightPhong {
		glm::vec3 m_La;
		glm::vec3 m_Ls;
		glm::vec3 m_Ld;
	public:
		LightPhong();
		LightPhong(const LightPhong& other);
		LightPhong(const glm::vec3& La, const glm::vec3& Ld, const glm::vec3& Ls);
		void setLa(const glm::vec3& La);
		void setLd(const glm::vec3& Ld);
		void setLs(const glm::vec3& Ls);
		glm::vec3 getLa() const;
		glm::vec3 getLd() const;
		glm::vec3 getLs() const;
		~LightPhong();
	};
}