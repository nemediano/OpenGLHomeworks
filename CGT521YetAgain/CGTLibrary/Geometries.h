#ifndef GEOMETRIES_H_
#define GEOMETRIES_H_

#include "Mesh.h"
namespace mesh {
	class Geometries {
	public:
		static Mesh plane();
		static Mesh icosphere(int subdiv = 3);
		static Mesh cube();
		static Mesh insideOutCube();
		static Mesh cylinder(int subAxis = 3, int divisions = 20, bool caps = true);
		static Mesh cone(int rings = 4, int divisions = 18, bool cap = true);
		static Mesh tethrahedra();
		static Mesh pyramid();
		static Mesh torus(float externRadio = 1.0f, float internRadio = 0.25f, int rings = 36, int sections = 12);
		static Mesh sphere(int slices = 20, int stacks = 15);
		static Mesh superShape(float a, float b, float m, glm::vec3 n, int discretization = 128);
		static Mesh teapot(int subdivisions = 6);
	private:
		

	};
}

#endif