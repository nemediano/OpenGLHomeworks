#include "stdafx.h"
#include "MathConstants.h"
#include "Geometry.h"

namespace math {
	bool almostZero(glm::vec3 v) {
		return glm::length2(v) < EPSILON *  EPSILON;
	}

	bool almostZero(const float& x) {
		return glm::abs(x) < EPSILON;
	}
}