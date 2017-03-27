#include "stdafx.h"
#include "MathConstants.h"
#include "MathHelpers.h"

namespace math {
	float toRadians(const float& degree) {
		return degree * TO_RADIANS;
	}

	float toDegree(const float& radians) {
		return radians * TO_DEGREE;
	}
}