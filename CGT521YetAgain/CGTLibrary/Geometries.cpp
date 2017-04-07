#include "stdafx.h"
#include "Geometries.h"
#include "MathConstants.h"

namespace mesh {
	using glm::vec3;
	using glm::vec4;
	using glm::vec2;
	using glm::mat4;
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
		Mesh sphere;
		vector<unsigned int> indices;
		vector<Vertex> vertices;

		
		float deltaAzimuth = PI / (rings - 1);
		float deltaPolar = TAU / slices;

		float polar = 0.0f;
		float azimuth = deltaAzimuth;
		for (int i = 1; i < (rings - 1); ++i) {
			polar = 0.0f;
			for (int j = 0; j < slices; j++) {
				Vertex v;
				v.position.x = sin(azimuth) * cos(polar);
				v.position.y = cos(azimuth);
				v.position.z = sin(azimuth) * sin(polar);;
				v.normal = v.position;
				v.textCoord.s = polar / TAU;
				v.textCoord.t = azimuth / PI;

				vertices.push_back(v);
				polar += deltaPolar;
			}
			azimuth += deltaAzimuth;
		}
		int start = vertices.size() - slices;

		for (int i = 0; i < (rings - 3); ++i) {
			for (int j = 0; j < slices; j++) {
				int a = i * slices + j;
				int b = i * slices + ((j + 1) % slices);
				int c = ((i + 1) % rings) * slices + j;
				int d = ((i + 1) % rings) * slices + ((j + 1) % slices);

				indices.push_back(a);
				indices.push_back(b);
				indices.push_back(d);

				indices.push_back(a);
				indices.push_back(d);
				indices.push_back(c);
			}
		}

		//North pole 
		int pole = static_cast<int>(vertices.size());
		Vertex v;
		v.position = vec3(0.0f, 1.0f, 0.0);
		v.normal = v.position;
		v.textCoord = vec2(0.5, 0.0f);
		vertices.push_back(v);
		for (int i = 0; i < slices; ++i) {
			indices.push_back(pole);
			indices.push_back((i + 1) % slices);
			indices.push_back(i);
		}

		//South pole
		pole = static_cast<int>(vertices.size());
		v.position = vec3(0.0f, -1.0f, 0.0);
		v.normal = v.position;
		v.textCoord = vec2(0.5, 1.0f);
		vertices.push_back(v);
		for (int i = 0; i < slices; ++i) {
			indices.push_back(pole);
			indices.push_back(i + start);
			indices.push_back((i + 1) % slices + start);
		}

		sphere.setVertices(vertices, true, true);
		sphere.setIndex(indices);
		return sphere;
	}

	Mesh Geometries::cylinder(int subAxis, int divisions, bool caps) {
		
		Mesh cylinder;
		vector<unsigned int> indices;
		vector<Vertex> vertices;

		float deltaHeight = 1.0f / subAxis;
		for (int i = 0; i <= subAxis; ++i) {
			float angle = 0.0f;
			float deltaAngle = TAU / divisions;
			for (int j = 0; j < divisions; ++j) {
				Vertex v;
				v.position.x = cos(angle);
				v.position.y = i * deltaHeight;
				v.position.z = sin(angle);
				v.normal = glm::normalize(vec3(v.position.x, 0.0f, v.position.z));
				v.textCoord.s = angle / TAU;
				v.textCoord.t = v.position.y;
				vertices.push_back(v);
				angle += deltaAngle;
				//Start to create the triangles from second iteration and so on
				if (j > 0 && i > 0) {
					//Create two triangle
					int a = (i - 1) * divisions + (j - 1);
					int b = (i - 1) * divisions + j;
					int c = i * divisions + (j - 1);
					int d = i * divisions + j;
					indices.push_back(c);
					indices.push_back(b);
					indices.push_back(a);

					indices.push_back(c);
					indices.push_back(d);
					indices.push_back(b);
				}
			}
			//Last two 
			if (i > 0) {
				int a = (i - 1) * divisions + (divisions - 1);
				int b = (i - 1) * divisions + 0;
				int c = i * divisions + (divisions - 1);
				int d = i * divisions + 0;
				indices.push_back(c);
				indices.push_back(b);
				indices.push_back(a);

				indices.push_back(c);
				indices.push_back(d);
				indices.push_back(b);
			}
			
		}


		if (caps) {
			int last_index = static_cast<int>(vertices.size());
			Vertex v;
			v.position = vec3(0.0f);
			v.normal = vec3(0.0f, -1.0f, 0.0f);
			v.textCoord = vec2(0.5f, 0.5f);
			vertices.push_back(v);
			//Bottom cap
			float angle = 0.0f;
			float deltaAngle = TAU / divisions;
			for (int i = 0; i < divisions; i++) {
				Vertex u;
				u.position.x = cos(angle);
				u.position.y = 0;
				u.position.z = sin(angle);
				u.normal = vec3(0.0f, -1.0f, 0.0f);
				u.textCoord.s = angle / TAU;
				u.textCoord.t = u.position.y;
				vertices.push_back(u);
				angle += deltaAngle;
			}
			//Remember that index start at 0
			for (int i = 1; i <= divisions; i++) {
				indices.push_back(last_index);
				int tmp = i;
				indices.push_back(last_index + tmp);
				tmp = (i % divisions) + 1;
				indices.push_back(last_index + tmp);
			}
			//Top cap
			last_index = static_cast<int>(vertices.size());
			v.position = vec3(0.0f, 1.0f, 0.0);
			v.normal = vec3(0.0f, 1.0f, 0.0f);
			v.textCoord = vec2(0.5f, 0.5f);
			vertices.push_back(v);
			angle = 0.0f;
			for (int i = 0; i < divisions; i++) {
				Vertex u;
				u.position.x = cos(angle);
				u.position.y = 1.0f;
				u.position.z = sin(angle);
				u.normal = vec3(0.0f, 1.0f, 0.0f);
				u.textCoord.s = angle / TAU;
				u.textCoord.t = u.position.y;
				vertices.push_back(u);
				angle += deltaAngle;
			}
			//Remember that index start at 0
			for (int i = 1; i <= divisions; i++) {
				indices.push_back(last_index);
				int tmp = (i % divisions) + 1;
				indices.push_back(last_index + tmp);
				tmp = i;
				indices.push_back(last_index + tmp);
			}
		}

		cylinder.setVertices(vertices, true, true);
		cylinder.setIndex(indices);
		return cylinder;
	}

	Mesh Geometries::teapot() {
		Mesh teapot;

		return teapot;
	}

	Mesh Geometries::torus(float externRadio, float internRadio, int rings, int sections) {
		Mesh torus;
		vector<unsigned int> indices;
		vector<Vertex> vertices;

		//Create a circular section
		float angle = 0.0f;
		float deltaAngle = TAU / sections;
		vector<Vertex> circle;
		for (int i = 0; i < sections; ++i) {
			Vertex v;
			v.position.x = internRadio * cos(angle);
			v.position.y = internRadio * sin(angle);
			v.position.z = 0.0f;
			v.normal = glm::normalize(v.position);
			v.textCoord.s = 0.0f;
			v.textCoord.t = i / (sections - 1);
			angle += deltaAngle;
			circle.push_back(v);
		}

		//Transform the circle and insert it in vector
		angle = 0.0;
		deltaAngle = TAU / rings;
		for (int i = 0; i < rings; ++i) {
			mat4 T(1.0f);
			T = glm::rotate(T, angle, vec3(0.0f, 1.0f, 0.0));
			T = glm::translate(T, vec3(externRadio, 0.0f, 0.0f));
			vector<Vertex> transformedCircle = circle;
			for (auto j = 0; j < circle.size(); ++j) {
				transformedCircle[j].position = vec3(T * vec4(circle[j].position, 1.0f));
				transformedCircle[j].normal = vec3(glm::inverse(glm::transpose(T)) * vec4(circle[j].normal, 0.0f));
				transformedCircle[j].textCoord.s = j / (rings - 1);
			}
			vertices.insert(vertices.end(), transformedCircle.begin(), transformedCircle.end());
			angle += deltaAngle;
		}

		//Create the triangles
		for (int i = 0; i < rings; ++i) {
			/*These are the literal vertex of the quadrialteral 
			  that I want to create using two triangles.
			  I could not came uo with a better naming convention */
			int a, b, c, d;
			for (int j = 0; j < sections; ++j) {
				a = i * sections + j;
				b = i * sections + ((j + 1) % sections);
				c = ((i + 1) % rings) * sections + j;
				d = ((i + 1) % rings) * sections + ((j + 1) % sections);
				//Create the two triangles
				indices.push_back(a);
				indices.push_back(d);
				indices.push_back(b);

				indices.push_back(a);
				indices.push_back(c);
				indices.push_back(d);
			}
		}

		torus.setVertices(vertices, true, true);
		torus.setIndex(indices);
		return torus;
	}

	Mesh Geometries::insideOutCube() {
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
		v.position = points[0]; v.normal = normals[3]; v.textCoord = textCoords[2]; vertices.push_back(v); //0
		v.position = points[1]; v.normal = normals[3]; v.textCoord = textCoords[1]; vertices.push_back(v); //1
		v.position = points[2]; v.normal = normals[3]; v.textCoord = textCoords[0]; vertices.push_back(v); //2
		v.position = points[3]; v.normal = normals[3]; v.textCoord = textCoords[3]; vertices.push_back(v); //3
		indices.push_back(2);
		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(3);
		indices.push_back(0);
		indices.push_back(2);
		//Bottom face of the cube
		v.position = points[0]; v.normal = normals[4]; v.textCoord = textCoords[3]; vertices.push_back(v); //4
		v.position = points[1]; v.normal = normals[4]; v.textCoord = textCoords[0]; vertices.push_back(v); //5
		v.position = points[5]; v.normal = normals[4]; v.textCoord = textCoords[1]; vertices.push_back(v); //6
		v.position = points[4]; v.normal = normals[4]; v.textCoord = textCoords[2]; vertices.push_back(v); //7
		indices.push_back(4);
		indices.push_back(6);
		indices.push_back(5);
		indices.push_back(4);
		indices.push_back(7);
		indices.push_back(6);
		//Left face of the cube
		v.position = points[0]; v.normal = normals[5]; v.textCoord = textCoords[1]; vertices.push_back(v); //8
		v.position = points[3]; v.normal = normals[5]; v.textCoord = textCoords[0]; vertices.push_back(v); //9
		v.position = points[4]; v.normal = normals[5]; v.textCoord = textCoords[2]; vertices.push_back(v); //10
		v.position = points[7]; v.normal = normals[5]; v.textCoord = textCoords[3]; vertices.push_back(v); //11
		indices.push_back(8);
		indices.push_back(11);
		indices.push_back(10);
		indices.push_back(8);
		indices.push_back(9);
		indices.push_back(11);
		//Top face of the cube
		v.position = points[2]; v.normal = normals[1]; v.textCoord = textCoords[3]; vertices.push_back(v); //12
		v.position = points[3]; v.normal = normals[1]; v.textCoord = textCoords[0]; vertices.push_back(v); //13
		v.position = points[6]; v.normal = normals[1]; v.textCoord = textCoords[2]; vertices.push_back(v); //14
		v.position = points[7]; v.normal = normals[1]; v.textCoord = textCoords[1]; vertices.push_back(v); //15
		indices.push_back(12);
		indices.push_back(14);
		indices.push_back(13);
		indices.push_back(13);
		indices.push_back(14);
		indices.push_back(15);
		//Right face of the cube
		v.position = points[1]; v.normal = normals[2]; v.textCoord = textCoords[2]; vertices.push_back(v); //16
		v.position = points[2]; v.normal = normals[2]; v.textCoord = textCoords[3]; vertices.push_back(v); //17
		v.position = points[5]; v.normal = normals[2]; v.textCoord = textCoords[1]; vertices.push_back(v); //18
		v.position = points[6]; v.normal = normals[2]; v.textCoord = textCoords[0]; vertices.push_back(v); //19
		indices.push_back(16);
		indices.push_back(18);
		indices.push_back(17);
		indices.push_back(17);
		indices.push_back(18);
		indices.push_back(19);
		//Front face of the cube
		v.position = points[4]; v.normal = normals[0]; v.textCoord = textCoords[1]; vertices.push_back(v); //20
		v.position = points[5]; v.normal = normals[0]; v.textCoord = textCoords[2]; vertices.push_back(v); //21
		v.position = points[6]; v.normal = normals[0]; v.textCoord = textCoords[3]; vertices.push_back(v); //22
		v.position = points[7]; v.normal = normals[0]; v.textCoord = textCoords[0]; vertices.push_back(v); //23
		indices.push_back(20);
		indices.push_back(22);
		indices.push_back(21);
		indices.push_back(20);
		indices.push_back(23);
		indices.push_back(22);

		cube.setVertices(vertices, true, true);
		cube.setIndex(indices);

		return cube;
	}

	Mesh Geometries::cone(int rings, int divisions, bool cap) {
		Mesh cone;
		vector<unsigned int> indices;
		vector<Vertex> vertices;

		float deltaHeight = 1.0f / rings;
		for (int i = 0; i <= rings; ++i) {
			float angle = 0.0f;
			float deltaAngle = TAU / divisions;
			for (int j = 0; j < divisions; ++j) {
				Vertex v;
				v.position.x = (1.0f - i * deltaHeight) * cos(angle);
				v.position.y = i * deltaHeight;
				v.position.z = (1.0f - i * deltaHeight) * sin(angle);
				v.normal = glm::normalize(vec3(v.position.x, cos(TAU / 8.0f), v.position.z));
				v.textCoord.s = angle / TAU;
				v.textCoord.t = v.position.y;
				vertices.push_back(v);
				angle += deltaAngle;
				//Start to create the triangles from second iteration and so on
				if (j > 0 && i > 0) {
					//Create two triangle
					int a = (i - 1) * divisions + (j - 1);
					int b = (i - 1) * divisions + j;
					int c = i * divisions + (j - 1);
					int d = i * divisions + j;
					indices.push_back(c);
					indices.push_back(b);
					indices.push_back(a);
					if (i < rings) {
						indices.push_back(c);
						indices.push_back(d);
						indices.push_back(b);
					}
					
				}
			}
			//Last two 
			if (i > 0) {
				int a = (i - 1) * divisions + (divisions - 1);
				int b = (i - 1) * divisions + 0;
				int c = i * divisions + (divisions - 1);
				int d = i * divisions + 0;
				indices.push_back(c);
				indices.push_back(b);
				indices.push_back(a);

				indices.push_back(c);
				indices.push_back(d);
				indices.push_back(b);
			}

		}


		if (cap) {
			int last_index = static_cast<int>(vertices.size());
			Vertex v;
			v.position = vec3(0.0f);
			v.normal = vec3(0.0f, -1.0f, 0.0f);
			v.textCoord = vec2(0.5f, 0.5f);
			vertices.push_back(v);
			//cap
			float angle = 0.0f;
			float deltaAngle = TAU / divisions;
			for (int i = 0; i < divisions; i++) {
				Vertex u;
				u.position.x = cos(angle);
				u.position.y = 0;
				u.position.z = sin(angle);
				u.normal = vec3(0.0f, -1.0f, 0.0f);
				u.textCoord.s = angle / TAU;
				u.textCoord.t = u.position.y;
				vertices.push_back(u);
				angle += deltaAngle;
			}
			//Remember that index start at 0
			for (int i = 1; i <= divisions; i++) {
				indices.push_back(last_index);
				int tmp = i;
				indices.push_back(last_index + tmp);
				tmp = (i % divisions) + 1;
				indices.push_back(last_index + tmp);
			}
		}

		cone.setVertices(vertices, true, true);
		cone.setIndex(indices);
		return cone;
	}

	Mesh Geometries::superQuadric() {
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

	Mesh Geometries::tethrahedra() {
		Mesh teth;
		vector<Vertex> vertices;
		vector<unsigned int> indices;

		vec3 pos[4];
		pos[0] = vec3(0.0f, 0.0f, 1.0f);
		pos[1] = vec3(0.0f, (2.0f / 3.0f) * glm::sqrt(2.0f), -1.0f / 3.0f);
		pos[2] = vec3(-glm::sqrt(2.0f / 3.0f), -glm::sqrt(2.0f) / 3.0f, -1.0f / 3.0f);
		pos[3] = vec3(glm::sqrt(2.0f / 3.0f), -glm::sqrt(2.0f) / 3.0f, -1.0f / 3.0f);

		vec2 text[6];
		text[0] = vec2(0.0f, 0.0f);
		text[1] = vec2(0.5f, 0.0f);
		text[2] = vec2(1.0f, 0.0f);
		text[3] = vec2(0.25f, 0.5f);
		text[4] = vec2(0.75f, 0.5f);
		text[5] = vec2(0.5f, 1.0f);

		vec3 normal = glm::normalize(glm::cross(pos[3] - pos[0], pos[1] - pos[0]));

		Vertex v;
		v.position = pos[0];
		v.textCoord = text[2];
		v.normal = normal;
		vertices.push_back(v); //0

		v.position = pos[3];
		v.textCoord = text[4];
		v.normal = normal;
		vertices.push_back(v); //1

		v.position = pos[1];
		v.textCoord = text[2];
		v.normal = normal;
		vertices.push_back(v); //2

		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(2);

		//Trinagle 2
		normal = glm::normalize(glm::cross(pos[1] - pos[0], pos[2] - pos[0]));
		v.position = pos[0];
		v.textCoord = text[0];
		v.normal = normal;
		vertices.push_back(v); //3

		v.position = pos[1];
		v.textCoord = text[1];
		v.normal = normal;
		vertices.push_back(v); //4

		v.position = pos[2];
		v.textCoord = text[3];
		v.normal = normal;
		vertices.push_back(v); //5

		indices.push_back(3);
		indices.push_back(4);
		indices.push_back(5);

		//Trinagle 3
		normal = glm::normalize(glm::cross(pos[2] - pos[0], pos[3] - pos[0]));
		v.position = pos[0];
		v.textCoord = text[5];
		v.normal = normal;
		vertices.push_back(v); //6

		v.position = pos[2];
		v.textCoord = text[3];
		v.normal = normal;
		vertices.push_back(v); //7

		v.position = pos[3];
		v.textCoord = text[4];
		v.normal = normal;
		vertices.push_back(v); //8

		indices.push_back(6);
		indices.push_back(7);
		indices.push_back(8);

		//Trinagle 4
		normal = glm::normalize(glm::cross(pos[3] - pos[1], pos[2] - pos[1]));
		v.position = pos[1];
		v.textCoord = text[1];
		v.normal = normal;
		vertices.push_back(v); //9

		v.position = pos[3];
		v.textCoord = text[4];
		v.normal = normal;
		vertices.push_back(v); //10

		v.position = pos[2];
		v.textCoord = text[3];
		v.normal = normal;
		vertices.push_back(v); //11

		indices.push_back(9);
		indices.push_back(10);
		indices.push_back(11);

		teth.setVertices(vertices, true, true);
		teth.setIndex(indices);
		return teth;
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