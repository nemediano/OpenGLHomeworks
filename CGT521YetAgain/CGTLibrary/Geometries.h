#pragma once
#include "Mesh.h"
namespace mesh {
	class Geometries {
	public:
		static Mesh cube();
		static Mesh sphere(int slices = 8, int rings = 8);
		static Mesh icosphere(int subdiv = 3);
		static Mesh cylinder(int subAxis = 8, int divisions = 8, bool caps = true);
		static Mesh plane();
		static Mesh insideOutCube();
		static Mesh cone(int rings = 8, int divisions = 18);
		static Mesh superQuadric();
	};
}
