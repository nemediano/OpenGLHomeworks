#pragma once
#include "Mesh.h"
namespace mesh {
	class Geometries {
	public:
		static Mesh cube();
		static Mesh sphere(int slices = 8, int rings = 8);
		static Mesh icosphere(int subdiv = 3);
		static Mesh cylinder(int subAxis = 3, int divisions = 20, bool caps = true);
		static Mesh plane();
		static Mesh insideOutCube();
		static Mesh cone(int rings = 4, int divisions = 18, bool cap = true);
		static Mesh superQuadric();
		static Mesh teapot();
		static Mesh tethrahedra();
	};
}
