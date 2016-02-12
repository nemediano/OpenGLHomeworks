#include <fstream>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/PostProcess.h>
#include "../opengl/HelperFunctions.h"
#include "Mesh.h"

namespace scene {
	Mesh::Mesh() : m_has_normals(false), m_has_texture(false), m_index_location(0), m_vbo_location(0), m_scale_factor(1.0f), m_in_gpu(false) {
		m_lower_corner = m_upper_corner = m_dimensions = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	Mesh::Mesh(const std::string& input_file_name) : Mesh::Mesh() {
		load_mesh_from_file(input_file_name);
	}

	Mesh::~Mesh() {
		release_locations();
	}

	bool Mesh::load_mesh_from_file(const std::string& input_file_name) {
		// Create an instance of the Importer class
		Assimp::Importer importer;
		//check if file exists
		std::ifstream input_file(input_file_name.c_str());
		if (!input_file.fail())	{
			input_file.close();
		} else {
			cerr << "Couldn't open file: " << input_file_name << endl;
			return false;
		}

		const aiScene* scene_ptr = importer.ReadFile(input_file_name, aiProcessPreset_TargetRealtime_Quality);
		// If the import failed, report it
		if (!scene_ptr)	{
			cerr << "File: " << input_file_name << "is damaged" << endl;
			string error(importer.GetErrorString());
			cerr << error << endl;
			return false;
		}
		else {
			cout << "Import of file " << input_file_name << " succeeded." << endl;
		}

		update_bounding_box(scene_ptr);

		//Parse Mesh data
		const aiMesh* mesh = scene_ptr->mMeshes[0];
		unsigned int num_faces = mesh->mNumFaces;
		for (unsigned int t = 0; t < num_faces; ++t) {
			const aiFace* face = &mesh->mFaces[t];
			for (unsigned int i = 0; i < face->mNumIndices; ++i) {
				m_indices.push_back(face->mIndices[i]);
			}
		}

		Vertex v;
		m_has_normals = mesh->HasNormals();
		m_has_texture = mesh->HasTextureCoords(0);
		for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
			if (mesh->HasPositions()) {
				v.position.x = mesh->mVertices[i].x;
				v.position.y = mesh->mVertices[i].y;
				v.position.z = mesh->mVertices[i].z;
			}
			if (m_has_normals) {
				v.normal.x = mesh->mNormals[i].x;
				v.normal.y = mesh->mNormals[i].y;
				v.normal.z = mesh->mNormals[i].z;
			}
			if (m_has_texture) {
				v.text_coordinates.s = mesh->mTextureCoords[0][i].x;
				v.text_coordinates.t = mesh->mTextureCoords[0][i].y;
			}
			m_vertices.push_back(v);
		}
		
		//delete scene_ptr;
		return true;
	}

	bool Mesh::is_empthy() const {
		return m_vertices.empty();
	}

	bool Mesh::has_texture() const {
		return m_has_texture;
	}

	bool Mesh::has_normals() const {
		return m_has_normals;
	}

	void Mesh::recalculate_normals() {

	}

	void Mesh::clear() {
		m_has_normals = m_has_texture = false;
		m_indices.clear();
		m_vertices.clear();
		m_lower_corner = m_upper_corner = m_dimensions = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	GLuint Mesh::get_vertex_location() const {
		return m_vbo_location;
	}

	GLuint Mesh::get_index_location() const {
		return m_index_location;
	}

	void Mesh::draw_triangles(const GLint& position_location, const GLint& normal_location, const GLint& texture_coordinates_location) const {
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
			glVertexAttribPointer(texture_coordinates_location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), OFFSET_OF(Vertex, text_coordinates));
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_location);

		/* Draw */
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, BUFFER_OFFSET(0));

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
	}

	void Mesh::set_vertices(const std::vector<Vertex>& vertices) {
		if (!vertices.empty()) {
			m_vertices = vertices;
		}
	}

	void Mesh::set_index(const std::vector<unsigned int>& indices) {
		if (!indices.empty()) {
			m_indices = indices;
		}
	}
	
	void Mesh::index_from_triangles(const std::vector<Triangle>& triangles) {

	}

	bool Mesh::is_in_gpu() const {
		return m_in_gpu;
	}

	void Mesh::send_data_to_gpu() {
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

	void Mesh::update_bounding_box(const aiScene* scene_ptr) {
		//Extract the bounding box
		m_lower_corner = 1e10f * glm::vec3(1.0f);
		m_upper_corner = -1e10f * glm::vec3(1.0f);

		unsigned int num_meshes = scene_ptr->mNumMeshes;
		//For all meshes
		for (unsigned int n = 0; n < num_meshes; ++n) {
			const aiMesh* mesh = scene_ptr->mMeshes[0];
			//For all vertex in the mesh
			for (unsigned int t = 0; t < mesh->mNumVertices; ++t) {
				aiVector3D tmp = mesh->mVertices[t];
				//Update lower bounding box
				m_lower_corner.x = std::min(m_lower_corner.x, tmp.x);
				m_lower_corner.y = std::min(m_lower_corner.y, tmp.y);
				m_lower_corner.z = std::min(m_lower_corner.z, tmp.z);
				//Update upper bounding box
				m_upper_corner.x = std::max(m_upper_corner.x, tmp.x);
				m_upper_corner.y = std::max(m_upper_corner.y, tmp.y);
				m_upper_corner.z = std::max(m_upper_corner.z, tmp.z);
			}
		}

		m_dimensions = m_upper_corner - m_lower_corner;
		m_scale_factor = 2.0f / glm::max(m_dimensions.x, glm::max(m_dimensions.y, m_dimensions.z));
	}

	float Mesh::get_scale_factor() const {
		return m_scale_factor;
	}
}