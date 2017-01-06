#include "stdafx.h"
#include "Geometry.h"
#include "MathConstants.h"

namespace math {
	bool almostZero(glm::vec3 v) {
		return glm::length2(v) < EPSILON *  EPSILON;
	}
}