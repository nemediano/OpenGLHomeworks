#ifndef TRACKBALL_H_
#define TRACKBALL_H_

namespace camera {
	class Trackball	{

	private:
		// Quaternion trackball related
		glm::quat m_base_rotation;
		glm::quat m_new_rotation;
		float projection_on_curve(const glm::vec2& projected);
		//For the mouse dragging 
		bool m_drag;
		glm::vec2 m_start_drag;
		glm::ivec2 m_window_size;
		float m_radius;

	public:
		Trackball();
		Trackball(const glm::ivec2& windowSize);
		Trackball(int width, int height);
		Trackball(float radius, const glm::ivec2& windowSize);
		void resetRotation();
		void startDrag(const glm::vec2& mousePos);
		void setWindowSize(const glm::ivec2& windowSize);
		void setWindowSize(int width, int height);
		glm::ivec2 getWindowSize() const;
		void drag(const glm::vec2& mousePos);
		void endDrag(const glm::vec2& mousePos);
		glm::mat4 getRotation() const;
		~Trackball();
	};
}

#endif