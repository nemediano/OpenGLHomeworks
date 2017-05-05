#ifndef MAT_COOK_TORRANCE_H_
#define MAT_COOK_TORRANCE_H_

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
const MatCookTorrance EMERALD = MatCookTorrance(glm::vec3(0.0215f, 0.1745f, 0.0215f), glm::vec3(0.07568f, 0.61424f, 0.07568f), glm::vec3(0.633f, 0.727811f, 0.633f), 1.48f, 0.083f);
const MatCookTorrance BLACK_PLASTIC = MatCookTorrance(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.01f, 0.01f, 0.1f), glm::vec3(0.5f, 0.5f, 0.5f), 1.78f, 0.265f);
}

#endif