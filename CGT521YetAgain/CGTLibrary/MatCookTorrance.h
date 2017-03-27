#pragma once

namespace lighting {

class MatCookTorrance {
private:
	glm::vec3 m_Ka;
	glm::vec3 m_Ks;
	glm::vec3 m_Kd;
	float m_eta;
	float m_m;
public:
	MatCookTorrance();
	MatCookTorrance(const MatCookTorrance& other);
	MatCookTorrance(const glm::vec3& ambientAndDiffuse);
	MatCookTorrance(const glm::vec3& Ka, const glm::vec3& Kd, const glm::vec3& Ks, float eta, float m);
	void setKa(const glm::vec3& Ka);
	void setKd(const glm::vec3& Kd);
	void setKs(const glm::vec3& Ks);
	void setRefractionIndex(float eta);
	void setMicrofacetSlope(float m);
	glm::vec3 getKa() const;
	glm::vec3 getKd() const;
	glm::vec3 getKs() const;
	float getRefractionIndex() const;
	float getMicrofacetSlope() const;
	~MatCookTorrance();
};

}