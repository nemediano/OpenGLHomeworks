#pragma once

namespace lighting {
	class Light {

	protected:
		float m_intensity;
		glm::vec3 m_color;

	public:
		Light();
		Light(float intensity, glm::vec3 color);
		virtual ~Light();
		void setColor(const glm::vec3& color);
		void setIntensity(float color);
		float getIntensity() const;
		glm::vec3 getColor() const;
	};
}