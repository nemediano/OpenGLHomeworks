#ifndef BASIC_TEST_H
#define BASIC_TEST_H

#include <vector>
#include <glm/glm.hpp>

namespace cgeometry {

bool point_in_triangle(const glm::vec3& v_1, const glm::vec3& v_2, const glm::vec3& v_3, const glm::vec3& p);

std::vector<size_t> convex_hull_by_graham_scan(const std::vector<glm::vec2>& point_cloud);

std::vector<unsigned short> convex_hull(const std::vector<glm::vec3>& point_cloud);

const float SINGLE_EPSILON = 1e-4f;
const double DOUBLE_EPSILON = 1e-6;

bool isAlmostZero(const double& number);

bool isAlmostZero(const float& number);

bool isAlmostZero(const glm::vec2& vec);

bool isAlmostZero(const glm::vec3& vec);

bool areAlmostEqual(const double& a, const double& b);

bool areAlmostEqual(const float& a, const float& b);

bool areAlmostEqual(const glm::vec2& a, const glm::vec2& b);

bool areAlmostEqual(const glm::vec3& a, const glm::vec3& b);

}

#endif