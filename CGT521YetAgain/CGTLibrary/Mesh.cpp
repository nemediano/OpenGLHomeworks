#include "stdafx.h"
#include "Mesh.h"
#include "MathConstants.h"
#include "MathHelpers.h"
#include "Geometry.h"
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))
#define OFFSET_OF(type, member) ((GLvoid*)(offsetof(type, member)))

using namespace std;

namespace mesh {
	Mesh::Mesh() : m_has_normals(false), m_has_texture(false), m_index_location(0), m_vbo_location(0), m_in_gpu(false) {
		m_lower_corner = m_upper_corner = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	Mesh::Mesh(const std::string& input_file_name) : Mesh::Mesh() {
		loadFromFile(input_file_name);
		update_bounding_box();
	}

	Mesh::~Mesh() {
		release_locations();
		clear();
	}

	bool Mesh::loadFromFile(const std::string& input_file_name) {
		using std::cout;
		using std::cerr;
		using std::endl;

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string err;
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, input_file_name.c_str());
		if (!err.empty()) {
			cerr << "Error at loading obj file:" << input_file_name << endl;
			cerr << err.c_str();
		}

		if (!ret) {
			return false;
		}

		m_has_normals = attrib.normals.size() > 0;
		m_has_texture = attrib.texcoords.size() > 0;
		m_vertices.clear();
		m_vertices = std::vector<Vertex>(attrib.vertices.size() / 3);
		m_indices.clear();

		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces (polygon, hopefully triangle =) )
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				int fv = shapes[s].mesh.num_face_vertices[f];
				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					m_vertices[idx.vertex_index].position.x = attrib.vertices[3 * idx.vertex_index + 0];
					m_vertices[idx.vertex_index].position.y = attrib.vertices[3 * idx.vertex_index + 1];
					m_vertices[idx.vertex_index].position.z = attrib.vertices[3 * idx.vertex_index + 2];

					if (m_has_normals) {
						m_vertices[idx.vertex_index].normal.x = attrib.normals[3 * idx.normal_index + 0];
						m_vertices[idx.vertex_index].normal.y = attrib.normals[3 * idx.normal_index + 1];
						m_vertices[idx.vertex_index].normal.z = attrib.normals[3 * idx.normal_index + 2];
					}

					if (m_has_texture) {
						m_vertices[idx.vertex_index].textCoord.s = attrib.texcoords[2 * idx.texcoord_index + 0];
						m_vertices[idx.vertex_index].textCoord.t = attrib.texcoords[2 * idx.texcoord_index + 1];
					}

					m_indices.push_back(idx.vertex_index);
				}
				index_offset += fv;
			}
		}
		
		return true;
	}

	bool Mesh::empthy() const {
		return m_vertices.empty();
	}

	bool Mesh::hasTexture() const {
		return m_has_texture;
	}

	bool Mesh::hasNormals() const {
		return m_has_normals;
	}

	void Mesh::clear() {
		m_has_normals = m_has_texture = false;
		m_indices.clear();
		m_vertices.clear();
		m_lower_corner = m_upper_corner = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	GLuint Mesh::getVertexLocation() const {
		return m_vbo_location;
	}

	GLuint Mesh::getIndexLocation() const {
		return m_index_location;
	}

	void Mesh::drawTriangles(const GLint& position_location, const GLint& normal_location, const GLint& texture_coordinates_location, const GLsizei& instances) const {
		if (!m_in_gpu) {
			cout << "Data is not in GPU memory!" << endl;
			return;
		}
		
		/* Bind */
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_location);
		if (position_location != -1) {
			glEnableVertexAttribArray(position_location);
			glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSET_OF(Vertex, position));
		}
		if (normal_location != -1) {
			glEnableVertexAttribArray(normal_location);
			glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSET_OF(Vertex, normal));
		}
		if (texture_coordinates_location != -1) {
			glEnableVertexAttribArray(texture_coordinates_location);
			glVertexAttribPointer(texture_coordinates_location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSET_OF(Vertex, textCoord));
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_location);

		/* Draw */
		if (instances == 1) {
			glDrawElements(GL_TRIANGLES, indicesCount(), GL_UNSIGNED_INT, BUFFER_OFFSET(0));
		} else {
			glDrawElementsInstanced(GL_TRIANGLES, indicesCount(), GL_UNSIGNED_INT, 0, instances);
		}

		/* Unbind and clean */
		if (position_location != -1) {
			glDisableVertexAttribArray(position_location);
		}
		if (normal_location != -1) {
			glDisableVertexAttribArray(normal_location);
		}
		if (texture_coordinates_location != -1) {
			glDisableVertexAttribArray(texture_coordinates_location);
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void Mesh::setVertices(const std::vector<Vertex>& vertices, bool normals, bool textCoords) {
		if (!vertices.empty()) {
			m_vertices = vertices;
			m_has_normals = normals;
			m_has_texture = textCoords;
			update_bounding_box();
		}
	}

	void Mesh::setIndex(const std::vector<unsigned int>& indices) {
		if (!indices.empty()) {
			m_indices = indices;
		}
	}

	/* Compare function for using to define a weak order in a container */
	bool Vector3DLessThan(const glm::vec3& rhs, const glm::vec3& lhs) {
		const float EPSILON = 1e-5f;
		if (glm::abs(lhs.x - rhs.x) > EPSILON) {
			return lhs.x < rhs.x;
		}
		else if (glm::abs(lhs.y - rhs.y) > EPSILON) {
			return lhs.y < rhs.y;
		}
		else if (glm::abs(lhs.z - rhs.z) > EPSILON) {
			return lhs.z < rhs.z;
		}
		else {
			return false;
		}
	}

	void Mesh::transform(const glm::mat4& T) {
		glm::mat4 normalMat = glm::inverse(glm::transpose(T));
		for (auto& v : m_vertices) {
			v.position = glm::vec3(T * glm::vec4(v.position, 1.0f));
			v.normal = glm::vec3(normalMat * glm::vec4(v.normal, 0.0f));
		}
		update_bounding_box();
	}

	void Mesh::toUnitCube() {

		float s = this->scaleFactor();
		glm::vec3 c = this->getBBCenter();

		glm::mat4 T(1.0f);
		T = glm::scale(T, glm::vec3(s));
		T = glm::translate(T, c);
		
		this->transform(T);
	}
	
	void Mesh::indexFromTriangles(const std::vector<Triangle>& triangles) {
		//This set will be used to get rid of the duplicate vertex
		std::set<glm::vec3, bool(*)(const glm::vec3&, const glm::vec3&)> tmp_storage(Vector3DLessThan);
		//Clear the previous data in the indices and points arrays, since we are about to start a new indexing
		m_indices.clear();
		m_vertices.clear();
		//Insert all the vertex in the tmp_storage
		for (auto triangle : triangles) {
			tmp_storage.insert(triangle.p_0);
			tmp_storage.insert(triangle.p_1);
			tmp_storage.insert(triangle.p_2);
		}

		//Insert index for the vertices
		for (auto triangle : triangles) {
			// V_0
			auto it = tmp_storage.find(triangle.p_0);
			auto index = std::distance(tmp_storage.begin(), it);
			m_indices.push_back(static_cast<unsigned int>(index));
			// V_1
			it = tmp_storage.find(triangle.p_1);
			index = std::distance(tmp_storage.begin(), it);
			m_indices.push_back(static_cast<unsigned int>(index));
			// P_2
			it = tmp_storage.find(triangle.p_2);
			index = std::distance(tmp_storage.begin(), it);
			m_indices.push_back(static_cast<unsigned int>(index));
		}

		//Create the Vertex storage
		Vertex tmp_vertex;
		for (auto position : tmp_storage) {
			tmp_vertex.position = position;
			//tmp_vertex.normal = glm::normalize(position);
			//tmp_vertex.textCoord = glm::vec2(0.0f);
			m_vertices.push_back(tmp_vertex);
		}

		cout << "theta\tphi\ts\tt" << endl;
		for (auto& v : m_vertices) {
			v.normal = glm::normalize(v.position);
			//r == 1 in the unit sphere thta is why I dont divide it.
			//test if the arguments if valid
			float theta = glm::acos(v.position.y);
			float phi = glm::atan(v.position.z, v.position.x) + math::PI;
			//This happens in the nort pole and the south pole
			if (math::almostZero(v.position.z) && math::almostZero(v.position.x)) {
				phi = math::PI;
			}
			
			v.textCoord.s = glm::clamp(1.0f - phi / math::TAU, 0.001f, 1.0f);
			v.textCoord.t = glm::clamp(1.0f - theta / math::PI, 0.001f, 1.0f);
			std::cout.unsetf(std::ios::floatfield);                // floatfield not set
			std::cout.precision(3);
			cout << math::toDegree(theta) << "\t" << math::toDegree(phi) << "\t";
			cout << v.textCoord.s << "\t" << v.textCoord.t << endl;
		}

		m_has_normals = true;
		m_has_texture = false;

		update_bounding_box();
	}

	bool Mesh::save(const std::string& out_obj_file) const {
		std::ofstream output_file;
		//Open for output erases previous file if necessary
		output_file.open(out_obj_file.c_str(), std::ios::out | std::ios::trunc);
		if (!output_file.is_open()) {
			return false;
		}

		output_file << "o " << "Test name" << "\n";
		//write all vertex
		output_file << "# List of vertices..." << "\n";
		for (auto v : m_vertices) {
			output_file << "v " << v.position.x << " " << v.position.y << " " << v.position.z << "\n";
		}

		if (m_has_normals) {
			output_file << "# List of normals..." << "\n";
			for (auto v : m_vertices) {
				output_file << "vn " << v.normal.x << " " << v.normal.y << " " << v.normal.z << "\n";
			}
		}

		if (m_has_texture) {
			output_file << "# List of texture coordinates..." << "\n";
			for (auto v : m_vertices) {
				output_file << "vt " << v.textCoord.s << " " << v.textCoord.t << "\n";
			}
		}

		//Only indexed triangular meshes supported by this class
		output_file << "# List of texture coordinates..." << "\n";
		for (int i = 0; i < m_indices.size(); i +=3) {
			//Remember obj index start at 1, not zero
			unsigned int v1 = m_indices[i] + 1;
			unsigned int v2 = m_indices[i + 1] + 1;
			unsigned int v3 = m_indices[i + 2] + 1;
			if (m_has_normals && m_has_texture) {
				output_file << "f " << v1 << "/" << v1 << "/" << v1 << " " << v2 << "/" << v2 << "/" << v2 << " " << v3 << "/" << v3 << "/" << v3 << "\n";
			} else if (m_has_normals) {
				output_file << "f " << v1 << "//" << v1 << " " << v2 << "//" << v2 << " " << v3 << "//" << v3 << "\n";
			} else if (m_has_texture) {
				output_file << "f " << v1 << "/" << v1 << " " << v2 << "/" << v2 << " " << v3 << "/" << v3 << "\n";
			} else {
				output_file << "f " << v1 << " " << v2 << " " << v3 << "\n";
			}
			
		}

		output_file.close();
		return true;
	}

	bool Mesh::inGPU() const {
		return m_in_gpu;
	}

	void Mesh::sendToGPU() {
		if (m_vbo_location == 0 && m_index_location ==  0) {
			ask_locations();
		}
		//First send the vertices
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_location);
		glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		//Now, the indices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_location);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		m_in_gpu = true;
	}
	
	void Mesh::ask_locations() {
		//Create the buffers
		glGenBuffers(1, &m_index_location);
		glGenBuffers(1, &m_vbo_location);
	}

	void Mesh::release_locations() {
		glDeleteBuffers(1, &m_vbo_location);
		glDeleteBuffers(1, &m_index_location);
	}

	void Mesh::update_bounding_box() {
		using glm::vec3;
		//Extract the bounding box
		m_lower_corner = FLT_MAX * vec3(1.0f);
		m_upper_corner = -FLT_MAX * vec3(1.0f);

		for (auto v : m_vertices) {
			//Check if this vertex changes the bounding box
			m_upper_corner = glm::max(v.position, m_upper_corner);
			m_lower_corner = glm::min(v.position, m_lower_corner);
		}

	}

	glm::vec3 Mesh::getBBCenter() const {
		return 0.5f * (m_upper_corner + m_lower_corner);
	}

	glm::vec3 Mesh::getBBSize() const {
		return (m_upper_corner - m_lower_corner);
	}

	float Mesh::scaleFactor() const {
		glm::vec3 size = getBBSize();
		return 1.0f / (glm::max(size.x, glm::max(size.y, size.z)));
	}

	GLsizei Mesh::trianglesCount() const {
		return indicesCount() / 3;
	}

	GLsizei Mesh::indicesCount() const {
		return static_cast<GLsizei>(m_indices.size());
	}

	GLsizei Mesh::vertexCount() const {
		return static_cast<GLsizei>(m_vertices.size());
	}
}