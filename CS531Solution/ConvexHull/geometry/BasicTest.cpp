#include "BasicTest.h"
#include <glm/gtx/norm.hpp>

namespace cgeometry {

	bool point_in_triangle(const glm::vec3& v_1, const glm::vec3& v_2, const glm::vec3& v_3, const glm::vec3& p) {
		return false;
	}

	std::vector<size_t> convex_hull_by_graham_scan(const std::vector<glm::vec2>& point_cloud) {
		std::vector<size_t> hull;

		//Handle special cases (one point)

		//Look for the leftmost down point
		size_t first = 0;
		for (size_t i = 0; i < point_cloud.size(); ++i) {
			auto point = point_cloud[i];
			if (point.y <= point_cloud[first].y && point.x <= point_cloud[first].x) {
				first = i;
			}
		}
		hull.push_back(first);

		//Order all of the points by slope


		return hull;
	}

	std::vector<unsigned short> convex_hull(const std::vector<glm::vec3>& point_cloud) {
		std::vector<unsigned short> hull;

		return hull;
	}

	bool isAlmostZero(const double& number) {
		return number > -DOUBLE_EPSILON && number < DOUBLE_EPSILON;
	}

	bool isAlmostZero(const float& number) {
		return number > -SINGLE_EPSILON && number < SINGLE_EPSILON;
	}

	bool isAlmostZero(const glm::vec2& vec) {
		return glm::length2(vec) < SINGLE_EPSILON;
	}

	bool isAlmostZero(const glm::vec3& vec) {
		return glm::length2(vec) < SINGLE_EPSILON;
	}

	bool areAlmostEqual(const double& a, const double& b) {
		return isAlmostZero(a - b);
	}

	bool areAlmostEqual(const float& a, const float& b) {
		return isAlmostZero(a - b);
	}

	bool areAlmostEqual(const glm::vec2& a, const glm::vec2& b) {
		return isAlmostZero(a - b);
	}

	bool areAlmostEqual(const glm::vec3& a, const glm::vec3& b) {
		return isAlmostZero(a - b);
	}
}