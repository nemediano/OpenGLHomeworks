#ifndef PUNCTUAL_H_
#define PUNCTUAL_H_

namespace lighting {
	class Punctual {
	private:
		glm::vec3 m_position;

	public:
		Punctual();
		Punctual(const Punctual& other);
		Punctual(const glm::vec3 position);
		~Punctual();
		void setPosition(const glm::vec3& position);
		glm::vec3 getPosition() const;
		void move(const glm::vec3& displacment);
	};

}

#endif