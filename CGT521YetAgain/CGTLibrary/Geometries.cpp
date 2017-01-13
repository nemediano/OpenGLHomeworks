#include "stdafx.h"
#include "Geometries.h"
#include "MathConstants.h"

namespace mesh {
	using glm::vec3;
	using glm::vec2;
	using namespace std;
	using namespace math;

	Mesh Geometries::cube() {
		Mesh cube;
		vector<vec3> points(8);
		vector<vec3> normals(6);
		vector<vec2> textCoords(4);

		points[0] = 0.5f * vec3(-1.0f, -1.0f, -1.0f);
		points[1] = 0.5f * vec3(1.0f, -1.0f, -1.0f);
		points[2] = 0.5f * vec3(1.0f, 1.0f, -1.0f);
		points[3] = 0.5f * vec3(-1.0f, 1.0f, -1.0f);
		points[4] = 0.5f * vec3(-1.0f, -1.0f, 1.0f);
		points[5] = 0.5f * vec3(1.0f, -1.0f, 1.0f);
		points[6] = 0.5f * vec3(1.0f, 1.0f, 1.0f);
		points[7] = 0.5f * vec3(-1.0f, 1.0f, 1.0f);

		normals[0] = vec3(0.0f, 0.0f, -1.0f);
		normals[1] = vec3(0.0f, -1.0f, 0.0f);
		normals[2] = vec3(-1.0f, 0.0f, 0.0f);
		normals[3] = vec3(0.0f, 0.0f, 1.0f);
		normals[4] = vec3(0.0f, 1.0f, 0.0f);
		normals[5] = vec3(1.0f, 0.0f, 0.0f);

		textCoords[0] = vec2(0.0f, 0.0f);
		textCoords[1] = vec2(0.0f, 1.0f);
		textCoords[2] = vec2(1.0f, 1.0f);
		textCoords[3] = vec2(1.0f, 0.0f);

		vector<unsigned int> indices;
		vector<Vertex> vertices;
		Vertex v;
		//Back face of the cube
		v.position = points[0]; v.normal = normals[0]; v.textCoord = textCoords[2]; vertices.push_back(v); //0
		v.position = points[1]; v.normal = normals[0]; v.textCoord = textCoords[1]; vertices.push_back(v); //1
		v.position = points[2]; v.normal = normals[0]; v.textCoord = textCoords[0]; vertices.push_back(v); //2
		v.position = points[3]; v.normal = normals[0]; v.textCoord = textCoords[3]; vertices.push_back(v); //3
		indices.push_back(2);
		indices.push_back(1);
		indices.push_back(0);
		indices.push_back(3);
		indices.push_back(2);
		indices.push_back(0);
		//Bottom face of the cube
		v.position = points[0]; v.normal = normals[1]; v.textCoord = textCoords[3]; vertices.push_back(v); //4
		v.position = points[1]; v.normal = normals[1]; v.textCoord = textCoords[0]; vertices.push_back(v); //5
		v.position = points[5]; v.normal = normals[1]; v.textCoord = textCoords[1]; vertices.push_back(v); //6
		v.position = points[4]; v.normal = normals[1]; v.textCoord = textCoords[2]; vertices.push_back(v); //7
		indices.push_back(4);
		indices.push_back(5);
		indices.push_back(6);
		indices.push_back(4);
		indices.push_back(6);
		indices.push_back(7);
		//Left face of the cube
		v.position = points[0]; v.normal = normals[2]; v.textCoord = textCoords[1]; vertices.push_back(v); //8
		v.position = points[3]; v.normal = normals[2]; v.textCoord = textCoords[0]; vertices.push_back(v); //9
		v.position = points[4]; v.normal = normals[2]; v.textCoord = textCoords[2]; vertices.push_back(v); //10
		v.position = points[7]; v.normal = normals[2]; v.textCoord = textCoords[3]; vertices.push_back(v); //11
		indices.push_back(8);
		indices.push_back(10);
		indices.push_back(11);
		indices.push_back(8);
		indices.push_back(11);
		indices.push_back(9);
		//Top face of the cube
		v.position = points[2]; v.normal = normals[4]; v.textCoord = textCoords[3]; vertices.push_back(v); //12
		v.position = points[3]; v.normal = normals[4]; v.textCoord = textCoords[0]; vertices.push_back(v); //13
		v.position = points[6]; v.normal = normals[4]; v.textCoord = textCoords[2]; vertices.push_back(v); //14
		v.position = points[7]; v.normal = normals[4]; v.textCoord = textCoords[1]; vertices.push_back(v); //15
		indices.push_back(12);
		indices.push_back(13);
		indices.push_back(14);
		indices.push_back(13);
		indices.push_back(15);
		indices.push_back(14);
		//Right face of the cube
		v.position = points[1]; v.normal = normals[5]; v.textCoord = textCoords[2]; vertices.push_back(v); //16
		v.position = points[2]; v.normal = normals[5]; v.textCoord = textCoords[3]; vertices.push_back(v); //17
		v.position = points[5]; v.normal = normals[5]; v.textCoord = textCoords[1]; vertices.push_back(v); //18
		v.position = points[6]; v.normal = normals[5]; v.textCoord = textCoords[0]; vertices.push_back(v); //19
		indices.push_back(16);
		indices.push_back(17);
		indices.push_back(18);
		indices.push_back(17);
		indices.push_back(19);
		indices.push_back(18);
		//Front face of the cube
		v.position = points[4]; v.normal = normals[3]; v.textCoord = textCoords[1]; vertices.push_back(v); //20
		v.position = points[5]; v.normal = normals[3]; v.textCoord = textCoords[2]; vertices.push_back(v); //21
		v.position = points[6]; v.normal = normals[3]; v.textCoord = textCoords[3]; vertices.push_back(v); //22
		v.position = points[7]; v.normal = normals[3]; v.textCoord = textCoords[0]; vertices.push_back(v); //23
		indices.push_back(20);
		indices.push_back(21);
		indices.push_back(22);
		indices.push_back(20);
		indices.push_back(22);
		indices.push_back(23);

		cube.setVertices(vertices, true, true);
		cube.setIndex(indices);
		return cube;
	}

	Mesh Geometries::sphere(int slices, int rings) {
		Mesh m;

		return m;
	}


	//Temporal container for triangles before mesh indexin
	std::vector<Triangle> triangles;
	void subdivide_face(vec3 v0, vec3 v1, vec3 v2, const size_t& level) {
		if (level > 0) {
			//Calculate midpoints and project them to sphere
			vec3 new_01 = glm::normalize(0.5f * (v0 + v1));
			vec3 new_12 = glm::normalize(0.5f * (v1 + v2));
			vec3 new_20 = glm::normalize(0.5f * (v2 + v0));
			//Subdivide again recursively using the new vertex
			subdivide_face(v0, new_01, new_20, level - 1);
			subdivide_face(new_01, new_12, new_20, level - 1);
			subdivide_face(new_01, v1, new_12, level - 1);
			subdivide_face(new_20, new_12, v2, level - 1);
		} else {
			//We reach the bottom of the recursion, we need to generate a triangle
			Triangle triangle;
			triangle.p_0 = v0;
			triangle.p_1 = v1;
			triangle.p_2 = v2;
			triangles.push_back(triangle);
		}
	}

	Mesh Geometries::icosphere(int subdiv) {
		Mesh sphere;

		//Start creating the 12 original vertices
		std::vector<vec3> initial_vertices(12);
		//Spherical coordinates
		float phi = 0.0f; //Between [0, and Pi]
		float psy = 0.0f; //Between [0, and Tau]
		float radio = 1.0f;

		//North pole
		initial_vertices[0] = radio * vec3(sin(phi) * cos(psy), sin(phi) * sin(psy), cos(phi));
		//Create five vertex below the north pole at TAU/5 gaps
		phi = PI / 3.0f;;
		for (int i = 1; i <= 5; ++i) {
			initial_vertices[i] = radio * vec3(sin(phi) * cos(psy), sin(phi) * sin(psy), cos(phi));
			psy += TAU / 5.0f;
		}
		//Create another five vertex below the first strip. At TAU/GAP and a TAU/10 offset
		psy = TAU / 10.0f;
		phi = PI - (PI / 3.0f);
		for (int i = 1; i <= 5; ++i) {
			initial_vertices[i + 5] = radio * vec3(sin(phi) * cos(psy), sin(phi) * sin(psy), cos(phi));
			psy += TAU / 5.0f;
		}
		//South pole
		phi = PI;
		psy = 0.0f;
		initial_vertices[11] = radio * vec3(sin(phi) * cos(psy), sin(phi) * sin(psy), cos(phi));

		int subdiv_level = glm::abs(subdiv);

		//Generate the initial 20 faces
		/************************************************************************/
		/* Connect the north pole to the first strip, a triangle fan            */
		/************************************************************************/
		subdivide_face(initial_vertices[0], initial_vertices[1], initial_vertices[2], subdiv_level);
		subdivide_face(initial_vertices[0], initial_vertices[2], initial_vertices[3], subdiv_level);
		subdivide_face(initial_vertices[0], initial_vertices[3], initial_vertices[4], subdiv_level);
		subdivide_face(initial_vertices[0], initial_vertices[4], initial_vertices[5], subdiv_level);
		subdivide_face(initial_vertices[0], initial_vertices[5], initial_vertices[1], subdiv_level);
		/************************************************************************/
		/* Connect the two mid rows of vertex in a triangle strip fashion       */
		/************************************************************************/
		subdivide_face(initial_vertices[1], initial_vertices[6], initial_vertices[2], subdiv_level);
		subdivide_face(initial_vertices[2], initial_vertices[6], initial_vertices[7], subdiv_level);
		subdivide_face(initial_vertices[2], initial_vertices[7], initial_vertices[3], subdiv_level);
		subdivide_face(initial_vertices[3], initial_vertices[7], initial_vertices[8], subdiv_level);
		subdivide_face(initial_vertices[3], initial_vertices[8], initial_vertices[4], subdiv_level);
		subdivide_face(initial_vertices[4], initial_vertices[8], initial_vertices[9], subdiv_level);
		subdivide_face(initial_vertices[4], initial_vertices[9], initial_vertices[5], subdiv_level);
		subdivide_face(initial_vertices[5], initial_vertices[9], initial_vertices[10], subdiv_level);
		subdivide_face(initial_vertices[5], initial_vertices[10], initial_vertices[1], subdiv_level);
		subdivide_face(initial_vertices[1], initial_vertices[10], initial_vertices[6], subdiv_level);
		/************************************************************************/
		/* Connect the south pole to the second strip, a triangle fan           */
		/************************************************************************/
		subdivide_face(initial_vertices[6], initial_vertices[11], initial_vertices[7], subdiv_level);
		subdivide_face(initial_vertices[7], initial_vertices[11], initial_vertices[8], subdiv_level);
		subdivide_face(initial_vertices[8], initial_vertices[11], initial_vertices[9], subdiv_level);
		subdivide_face(initial_vertices[9], initial_vertices[11], initial_vertices[10], subdiv_level);
		subdivide_face(initial_vertices[10], initial_vertices[11], initial_vertices[6], subdiv_level);

		sphere.indexFromTriangles(triangles);

		//Get the vectors back to add normal and text coords

		triangles.clear();
		return sphere;
	}

	Mesh Geometries::cylinder(int subAxis, int subHeig) {
		Mesh m;

		return m;
	}

	Mesh Geometries::plane() {

		Mesh plane;

		Vertex v;
		vector<Vertex> vertices;
		v.normal = vec3(0.0f, 0.0f, 1.0f);

		v.position = vec3(-0.5f, 0.5f, 0.0f);
		v.textCoord = vec2(0.0, 0.0);
		vertices.push_back(v);

		v.position = vec3(-0.5f, -0.5f, 0.0f);
		v.textCoord = vec2(0.0, 1.0);
		vertices.push_back(v);

		v.position = vec3(0.5f, -0.5f, 0.0f);
		v.textCoord = vec2(1.0, 1.0);
		vertices.push_back(v);

		v.position = vec3(0.5f, 0.5f, 0.0f);
		v.textCoord = vec2(1.0, 0.0);
		vertices.push_back(v);
		
		vector<unsigned int> indices;
		
		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(3);

		indices.push_back(1);
		indices.push_back(2);
		indices.push_back(3);

		plane.setVertices(vertices, true, true);
		plane.setIndex(indices);

		return plane;
	}
}