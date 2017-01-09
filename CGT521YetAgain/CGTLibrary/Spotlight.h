#pragma once
#include "Light.h"

namespace lighting {
	class Spotlight : public Light {

	public:
		Spotlight();
		Spotlight(const Spotlight& other);
		Spotlight(const glm::vec3& position, const glm::vec3& target, float aperture);
		Spotlight(const glm::vec3& color, float intensity);
		Spotlight(const glm::vec3& position, const glm::vec3& target, float aperture, const glm::vec3& color, float intensity);
		glm::vec3 getPosition() const;
		float getAperture() const;
		glm::vec3 getTarget() const;
		glm::vec3 getDirection() const;
		void setPosition(const glm::vec3& position);
		void setTarget(const glm::vec3& target);
		void setDirection(const glm::vec3& direction);
		void setAperture(float aperture);

	private:
		glm::vec3 m_position;
		glm::vec3 m_target;
		float m_aperture;
	};
}