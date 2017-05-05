#include "stdafx.h"
#include "Punctual.h"

namespace lighting {
	Punctual::Punctual() {
		m_position = glm::normalize(glm::vec3(1.0f));
	}

	Punctual::Punctual(const Punctual& other) {
		m_position = other.m_position;
	}

	Punctual::Punctual(const glm::vec3 position) {
		m_position = position;
	}

	void Punctual::setPosition(const glm::vec3& position) {
		m_position = position;
	}

	glm::vec3 Punctual::getPosition() const {
		return m_position;
	}

	void Punctual::move(const glm::vec3& displacment) {
		m_position += displacment;
	}

	Punctual::~Punctual() {

	}
}
