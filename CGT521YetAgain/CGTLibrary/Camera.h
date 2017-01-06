#pragma once

namespace camera {
	class Camera {
	private:
		glm::vec3 m_position;
		glm::vec3 m_target;
		glm::vec3 m_up;
		float m_aperture;
		float m_near;
		float m_far;
		float m_aspect;
	public:
		Camera();
		Camera(const glm::vec3& position, const glm::vec3& target);
		~Camera();
		void setLookAt(const glm::vec3& position, const glm::vec3& target);
		void setPosition(const glm::vec3& position);
		glm::vec3 getPosition() const;
		void setTarget(const glm::vec3& target);
		glm::vec3 getTarget() const;
		void setAspectRatio(float aspect);
		void setAspectRatio(int width, int height);
		float getAspectRatio() const;
		void setFovY(float fov_y);
		void addFovY(float deltaAngle);
		float getFovY() const;
		void setNear(float Near);
		float getNear() const;
		void setFar(float Far);
		float getFar() const;
		void setDepthView(float Near, float Far);
		float getDepthView() const;
		glm::mat4 getViewMatrix() const;
		glm::mat4 getProjectionMatrix() const;
		glm::mat4 getPVMatrix() const;
	};

}

