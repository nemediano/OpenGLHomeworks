#include "stdafx.h"
#include "Trackball.h"


namespace camera {
	Trackball::Trackball() {
		m_base_rotation = glm::normalize(glm::quat(1.0f, glm::vec3(0.0f, 0.0f, 0.0f)));
		m_new_rotation = glm::normalize(glm::quat(1.0f, glm::vec3(0.0f, 0.0f, 0.0f)));
		
		//For the mouse dragging 
		m_drag = false;
		m_start_drag = glm::vec2(0.0f, 0.0f);
		m_window_size = glm::ivec2(0, 0);

		//For curve
		m_radius = 0.8f;
	}

	Trackball::Trackball(const glm::ivec2& windowSize) : Trackball() {
		setWindowSize(windowSize);
	}

	Trackball::Trackball(int width, int height) : Trackball() {
		setWindowSize(glm::ivec2(width, height));
	}

	Trackball::Trackball(float radius, const glm::ivec2& windowSize) : Trackball() {
		setWindowSize(windowSize);
		m_radius = glm::clamp(radius, 0.01f, 1.0f);
	}

	Trackball::~Trackball() {

	}

	void Trackball::setWindowSize(const glm::ivec2& windowSize) {
		//TODO: validate here if needed
		m_window_size = glm::abs(windowSize);
	}

	void Trackball::setWindowSize(int width, int height) {
		setWindowSize(glm::ivec2(width, height));
	}

	glm::ivec2 Trackball::getWindowSize() const {
		return m_window_size;
	}

	void Trackball::resetRotation() {
		m_base_rotation = glm::normalize(glm::quat(1.0f, glm::vec3(0.0f, 0.0f, 0.0f)));
		m_new_rotation = glm::normalize(glm::quat(1.0f, glm::vec3(0.0f, 0.0f, 0.0f)));
		m_drag = false;
	}

	glm::mat4 Trackball::getRotation() const {
		return glm::mat4_cast(m_new_rotation * m_base_rotation);
	}

	void Trackball::startDrag(const glm::vec2& mousePos) {
		m_drag = true;
		m_start_drag = mousePos;
	}

	void Trackball::drag(const glm::vec2& mousePos) {
		using glm::vec2;
		using glm::vec3;

		if (m_drag) {
			vec2 mouse_current = mousePos;
			/*
			At this point mouse_start_drag and mouse_current are in pixel coordinates (device)
			we need to transform them in world coordinates is a two step process:
			1.- Translating to the scene center (Mouse coordinates are not in the center of the window but
			rather in the upper left corner of the window).
			2.- Scale to the same coordinate system, remember they are in pixel, not in the [-1, 1] x [-1, 1]
			that we are after projection.
			3.- Invert the Y coordinate since in most window systems the pixel coordinates are reversed. I. e.
			positive direction is down, not up.
			*/
			vec2 window_center = 0.5f * glm::vec2(m_window_size);
			vec2 scale_factors = vec2(2.0f / m_window_size.x, -2.0f / m_window_size.y);
			vec2 mouse_current_in_world = scale_factors * (mouse_current - window_center);
			vec2 mouse_start_drag_in_world = scale_factors * (m_start_drag - window_center);
			/* Update the new rotation using the algorithm described in https://www.opengl.org/wiki/Trackball */
			vec3 v_1 = glm::normalize(vec3(mouse_current_in_world, projection_on_curve(mouse_current_in_world)));
			vec3 v_2 = glm::normalize(vec3(mouse_start_drag_in_world, projection_on_curve(mouse_start_drag_in_world)));
			glm::vec3 axis = glm::cross(v_1, v_2);
			float angle = glm::angle(v_1, v_2);
			m_new_rotation = glm::normalize(glm::quat(glm::cos(0.5f * angle), glm::sin(0.5f * angle) * axis));
		}
	}

	void Trackball::endDrag(const glm::vec2& mousePos) {
		m_drag = false;
		/* Calculate the accumulated rotation: base rotation plus new one */
		m_base_rotation = glm::normalize(m_new_rotation * m_base_rotation);
		/* Reset new rotation to identity */
		m_new_rotation = glm::normalize(glm::quat(1.0f, glm::vec3(0.0f, 0.0f, 0.0f)));
	}

	/*
	Which curve you use affect the numerical stability of the algorithm.
	This is why most of the people don't actually uses a sphere but rather a
	the hyperbolic sheet described in:
	https://www.opengl.org/wiki/Object_Mouse_Trackball
	*/
	float Trackball::projection_on_curve(const glm::vec2& projected) {
		float z = 0.0f;
		if (glm::length2(projected) <= (0.5f * m_radius * m_radius)) {
			//Inside the sphere
			z = glm::sqrt(m_radius * m_radius - glm::length2(projected));
		}
		else {
			//Outside of the sphere using hyperbolic sheet
			z = (0.5f * m_radius * m_radius) / glm::length(projected);
		}
		return z;
	}
}