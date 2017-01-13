#pragma once
#include "Mesh.h"
namespace mesh {
	class Geometries {
	public:
		static Mesh cube();
		static Mesh sphere(int slices = 8, int rings = 8);
		static Mesh icosphere(int subdiv = 3);
		static Mesh cylinder(int subAxis = 8, int subHeig = 8);
		static Mesh plane();
	};
}
