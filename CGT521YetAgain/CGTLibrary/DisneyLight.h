#ifndef DISNEY_LIGHT_H_
#define DISNEY_LIGHT_H_

namespace lighting {
	class DisneyLight {

	protected:
		float m_intensity;
		glm::vec3 m_color;

	public:
		DisneyLight();
		DisneyLight(float intensity, glm::vec3 color);
		virtual ~DisneyLight();
		void setColor(const glm::vec3& color);
		void setIntensity(float color);
		float getIntensity() const;
		glm::vec3 getColor() const;
	};
}

#endif