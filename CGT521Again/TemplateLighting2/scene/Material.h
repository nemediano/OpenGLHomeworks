#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>
#include <glm/glm.hpp>

namespace scene {
	class Material {
		private:
			//Ambient material color
			glm::vec3 m_Ka;
			//Diffuse material color
			glm::vec3 m_Kd;
			//Specular material color
			glm::vec3 m_Ks;
			//Shininess material coefficient (Phong)
			float m_shininess;
			//Eta (index of refraction for Cook torrance)
			float m_refraction;
			//M average of slope microfactes (for Cook Torrance)
			float m_slope;
			//Name of the material
			std::string m_name;
		public:
			Material();
			Material(const glm::vec3& Ka, const glm::vec3& Kd, const glm::vec3& Ks, const float& shininess);
			Material(const glm::vec3& Ka, const glm::vec3& Kd, const glm::vec3& Ks, const float& shininess, const std::string& name);
			Material(const glm::vec3& Ka, const glm::vec3& Kd, const glm::vec3& Ks, const std::string& name);
			Material(const glm::vec3& Ka, const glm::vec3& Kd, const glm::vec3& Ks, const float& refraction, const float& slope, const std::string& name);
			Material(const glm::vec3& Ka, const glm::vec3& Kd, const glm::vec3& Ks, const float& refraction, const float& slope);
			Material(const glm::vec3& Ka, const glm::vec3& Kd, const glm::vec3& Ks);
			Material(const glm::vec3& Ka, const glm::vec3& Kd);
			Material(const glm::vec3& ambientAndDiffuse, const glm::vec3& Ks, const float& shininess);
			Material(const glm::vec3& ambientAndDiffuse, const float& shininess);
			Material(const glm::vec3& ambientAndDiffuse);
			//Getter and setters
			glm::vec3 getKa() const;
			glm::vec3 getKd() const;
			glm::vec3 getKs() const;
			float getShininnes() const;
			float getRoughness() const;
			float getRefraction() const;
			std::string getName() const;
			void setKa(const glm::vec3& Ka);
			void setKd(const glm::vec3& Kd);
			void setKs(const glm::vec3& Ks);
			void setShininess(const float& shininess);
			void setRefraction(const float& refraction);
			void setRoghness(const float& rougness);
			void setName(const std::string& name);
	};
	using glm::vec3;
	const float sha = 128.0f;
	//They calculate the shininess factor different in the equation, I need to adjust
	const Material EMERALD = Material(vec3(0.0215f, 0.1745f, 0.0215f), vec3(0.07568f, 0.61424f, 0.07568f), vec3(0.633f, 0.727811f, 0.633f), sha * 0.6f, "Emerald");
	const Material JADE = Material(vec3(0.135f, 0.2225f, 0.1575f), vec3(0.54f, 0.89f, 0.63f), vec3(0.316228f, 0.316228f, 0.316228f), sha * 0.1f, "Jade");
	const Material OBSIDIAN = Material(vec3(0.05375f, 0.05f, 0.06625f), vec3(0.18275f, 0.17f, 0.22525f), vec3(0.332741f, 0.328634f, 0.346435f), sha * 0.3f, "Obsidian");
	const Material PEARL = Material(vec3(0.25f, 0.20725f, 0.20725f), vec3(1.0f, 0.829f, 0.829f), vec3(0.296648f, 0.296648f, 0.296648f), sha * 0.088f, "Pearl");
	const Material RUBY = Material(vec3(0.1745f, 0.01175f, 0.01175f), vec3(0.61424f, 0.04136f, 0.04136f), vec3(0.727811f, 0.626959f, 0.626959f), sha * 0.6f, "Ruby");
	const Material TURQUOISE = Material(vec3(0.1f, 0.18725f, 0.1745f), vec3(0.396f, 0.74151f, 0.69102f), vec3(0.297254f, 0.30829f, 0.306678f), sha * 0.1f, "Turquoise");

	const Material BRASS = Material(vec3(0.329412f, 0.223529f, 0.027451f), vec3(0.780392f, 0.568627f, 0.113725f), vec3(0.992157f, 0.941176f, 0.807843f), sha * 0.21794872f, "Brass");
	const Material BRONZE = Material(vec3(0.2125f, 0.1275f, 0.054f), vec3(0.714f, 0.4284f, 0.18144f), vec3(0.393548f, 0.271906f, 0.166721f), sha * 0.2f, "Bronze");
	const Material CHROME = Material(vec3(0.25f, 0.25f, 0.25f), vec3(0.4f, 0.4f, 0.4f), vec3(0.774597f, 0.774597f, 0.774597f), sha * 0.6f, "Chrome");
	const Material COPPER = Material(vec3(0.19125f, 0.0735f, 0.0225f), vec3(0.7038f, 0.27048f, 0.0828f), vec3(0.256777f, 0.137622f, 0.086014f), sha * 0.1f, "Copper");
	const Material GOLD = Material(vec3(0.24725f, 0.1995f, 0.0745f), vec3(0.75164f, 0.60648f, 0.22648f), vec3(0.628281f, 0.555802f, 0.366065f), sha * 0.4f, "Gold");
	const Material SILVER = Material(vec3(0.19225f, 0.19225f, 0.19225f), vec3(0.50754f, 0.50754f, 0.50754f), vec3(0.508273f, 0.508273f, 0.508273f), sha * 0.4f, "Silver");

	const Material BLACK_PLASTIC = Material(vec3(0.0f, 0.0f, 0.0f), vec3(0.01f, 0.01f, 0.01f), vec3(0.50f, 0.50f, 0.50f), sha * 0.25f, "Black plastic");
	const Material CYAN_PLASTIC = Material(vec3(0.0f, 0.1f, 0.06f), vec3(0.0f, 0.50980392f, 0.50980392f), vec3(0.50196078f, 0.50196078f, 0.50196078f), sha * 0.25f, "Cyan plastic");
	const Material GREEN_PLASTIC = Material(vec3(0.0f, 0.0f, 0.0f), vec3(0.1f, 0.35f, 0.1f), vec3(0.45f, 0.55f, 0.45f), sha * 0.25f, "Green Plastic");
	const Material RED_PLASTIC = Material(vec3(0.0f, 0.0f, 0.0f), vec3(0.5f, 0.0f, 0.0f), vec3(0.7f, 0.6f, 0.6f), sha * 0.25f, "Red plastic");
	const Material WHITE_PLASTIC = Material(vec3(0.0f, 0.0f, 0.0f), vec3(0.55f, 0.55f, 0.55f), vec3(0.70f, 0.70f, 0.70f), sha * 0.25f, "White plastic");
	const Material YELLOW_PLASTIC = Material(vec3(0.0f, 0.0f, 0.0f), vec3(0.5f, 0.5f, 0.0f), vec3(0.60f, 0.60f, 0.50f), sha * 0.25f, "Yellow plastic");


	const Material BLACK_RUBBER = Material(vec3(0.02f, 0.02f, 0.02f), vec3(0.01f, 0.01f, 0.01f), vec3(0.4f, 0.4f, 0.4f), sha * 0.078125f, "Black rubber");
	const Material CYAN_RUBBER = Material(vec3(0.0f, 0.05f, 0.05f), vec3(0.4f, 0.5f, 0.5f), vec3(0.04f, 0.7f, 0.7f), sha * 0.078125f, "Cyan rubber");
	const Material GREEN_RUBBER = Material(vec3(0.0f, 0.05f, 0.0f), vec3(0.4f, 0.5f, 0.4f), vec3(0.04f, 0.7f, 0.04f), sha * 0.078125f, "Green rubber");
	const Material RED_RUBBER = Material(vec3(0.05f, 0.0f, 0.0f), vec3(0.5f, 0.4f, 0.4f), vec3(0.7f, 0.04f, 0.04f), sha * 0.078125f, "Red rubber");
	const Material WHITE_RUBBER = Material(vec3(0.05f, 0.05f, 0.05f), vec3(0.5f, 0.5f, 0.5f), vec3(0.7f, 0.7f, 0.7f), sha * 0.078125f, "White rubber");
	const Material YELLOW_RUBBER = Material(vec3(0.05f, 0.05f, 0.0f), vec3(0.5f, 0.5f, 0.4f), vec3(0.7f, 0.7f, 0.04f), sha * 0.078125f, "Yellow rubber");
}
#endif