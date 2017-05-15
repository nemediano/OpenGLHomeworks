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
		static Mesh cylinder(int slices = 20, int stacks = 1, bool caps = true);
		static Mesh cone(int slices = 18, int stacks = 4, bool cap = true);
		static Mesh coneTexture(int slices = 18, int stacks = 4, bool cap = true);
		static Mesh tethrahedra();
		static Mesh pyramid();
		static Mesh torus(float outerRadius = 1.0f, float innerRadius = 0.25f, int rings = 36, int sides = 12);
		static Mesh torusTexture(float outerRadius = 1.0f, float innerRadius = 0.25f, int rings = 36, int sides = 12);
		static Mesh sphere(int slices = 20, int stacks = 15);
		static Mesh superShape(float a, float b, float m, glm::vec3 n, int discretization = 128);
		static Mesh teapot(int subdivisions = 6);
		static Mesh cylinderTexture(int slices = 20, int stacks = 1, bool caps = true);
	private:
		

	};
}

#endif