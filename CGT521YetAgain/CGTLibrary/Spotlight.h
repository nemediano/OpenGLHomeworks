#ifndef SPOT_LIGHT_H_
#define SPOT_LIGHT_H_

#include "Texture.h"

namespace lighting {

	class Spotlight {

	private:
		glm::vec3 m_position;
		glm::vec3 m_target;
		float m_aperture;
		float m_near;
		float m_far;
		image::Texture m_stencil;
		bool m_useStencil;
		bool m_useShadowMap;
		//FBO for the shadow map
		GLuint m_fbo_id;
		GLuint m_depth_texture_id;
		int m_resolution;
		
	public:
		Spotlight();
		Spotlight(const Spotlight& other);
		Spotlight(const glm::vec3& position, const glm::vec3& target, float aperture);
		Spotlight(const glm::vec3& position, const glm::vec3& target, float aperture, const glm::vec3& color, float intensity, float ratio);
		~Spotlight();
		glm::vec3 getPosition() const;
		float getAperture() const;
		glm::vec3 getTarget() const;
		glm::vec3 getDirection() const;
		glm::mat4 getP() const;
		glm::mat4 getM() const;
		glm::mat4 getPM() const;
		GLuint getFBO() const;
		GLuint getShadowMap() const;
		int getShadowResolution() const;
		void createShadowBuffer();
		GLuint getStencil() const;
		void setStencil(const image::Texture& stencil);
		void createTexturesGPU();
		void setDepthRange(float nearDistance, float farDistance);
		void setPosition(const glm::vec3& position);
		void setTarget(const glm::vec3& target);
		void setDirection(const glm::vec3& direction);
		void setAperture(float aperture);
		void setLookAt(const glm::vec3& position, const glm::vec3& target, float aperture);
	
	};
}

#endif