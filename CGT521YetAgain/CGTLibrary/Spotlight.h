#ifndef SPOT_LIGHT_H_
#define SPOT_LIGHT_H_

#include "Texture.h"

namespace lighting {

	class Spotlight {

	private:
		glm::vec3 m_position;
		glm::vec3 m_target;
		float m_aperture;
		float m_ratio;
		image::Texture m_stencil;
		bool m_useStencil;
		//FBO for the shadow map
		//GLuint m_fbo_id;
		//GLuint m_depth_texture_id;
		//int m_width;
		//int m_hight;
	public:
		Spotlight();
		Spotlight(const Spotlight& other);
		Spotlight(const glm::vec3& position, const glm::vec3& target, float aperture);
		Spotlight(const glm::vec3& position, const glm::vec3& target, float aperture, const glm::vec3& color, float intensity, float ratio);
		glm::vec3 getPosition() const;
		float getAperture() const;
		float getRatio() const;
		glm::vec3 getTarget() const;
		glm::vec3 getDirection() const;
		glm::mat4 getP() const;
		glm::mat4 getM() const;
		glm::mat4 getPM() const;
		//GLuint getFBO() const;
		//GLuint getShadowMap() const;
		GLuint getStencil() const;
		void setStencil(const image::Texture& stencil);
		void createTexturesGPU();
		void setPosition(const glm::vec3& position);
		void setTarget(const glm::vec3& target);
		void setDirection(const glm::vec3& direction);
		void setAperture(float aperture);
		void setRatio(float ratio);
		void setLookAt(const glm::vec3& position, const glm::vec3& target, float aperture);
	
	};
}

#endif