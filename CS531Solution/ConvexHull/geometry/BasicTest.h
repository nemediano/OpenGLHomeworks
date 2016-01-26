#ifndef BASIC_TEST_H
#define BASIC_TEST_H

#include <vector>
#include <glm/glm.hpp>

namespace cgeometry {

bool point_in_triangle(const glm::vec3& v_1, const glm::vec3& v_2, const glm::vec3& v_3, const glm::vec3& p);

std::vector<unsigned short> convex_hull(const std::vector<glm::vec2>& point_cloud);

std::vector<unsigned short> convex_hull(const std::vector<glm::vec3>& point_cloud);

}

#endif