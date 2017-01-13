#pragma once
#include "Light.h"
namespace lighting {

	class Directional : public Light {
		public:
			Directional();
			Directional(const Directional& other);
			Directional(const glm::vec3 direction);
			~Directional();
			void setDirection(const glm::vec3 direction);
			glm::vec3 getDirection() const;
			void rotate(const glm::vec3 eulerAngles);
			void rotate(const glm::quat rotation);
			void rotate(float angleX, float angleY, float angleZ);
		private:
			glm::vec3 m_direction;

	};
}