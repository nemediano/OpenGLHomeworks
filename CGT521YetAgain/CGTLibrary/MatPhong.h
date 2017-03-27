#pragma once
namespace lighting {
	class MatPhong {
		glm::vec3 m_Ka;
		glm::vec3 m_Ks;
		glm::vec3 m_Kd;
		float m_alpha;
	public:
		MatPhong();
		MatPhong(const MatPhong& other);
		MatPhong(const glm::vec3& ambientAndDiffuse);
		MatPhong(const glm::vec3& Ka, const glm::vec3& Kd, const glm::vec3& Ks, float alpha);
		void setKa(const glm::vec3& Ka);
		void setKd(const glm::vec3& Kd);
		void setKs(const glm::vec3& Ks);
		void setAlpha(float alpha);
		glm::vec3 getKa() const;
		glm::vec3 getKd() const;
		glm::vec3 getKs() const;
		float getAlpha() const;
		~MatPhong();
	};
}
