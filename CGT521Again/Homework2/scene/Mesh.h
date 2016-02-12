#ifndef MESH_H_
#define MESH_H_

#include "../glut/Globals.h"
#include <GL/glew.h>
#include <vector>

#include <glm/glm.hpp>
#include <assimp/scene.h>

namespace scene {
	
	class Mesh {
	private:
		GLuint m_vbo_location;
		GLuint m_index_location;
		bool m_has_normals;
		bool m_has_texture;
		bool m_in_gpu;
		std::vector<Vertex> m_vertices;
		std::vector<unsigned int> m_indices;
		glm::vec3 m_dimensions;
		glm::vec3 m_upper_corner;
		glm::vec3 m_lower_corner;
		float m_scale_factor;
		void update_bounding_box(const aiScene* scene);
		void ask_locations();
		void release_locations();
	public:
		Mesh();
		Mesh(const std::string& input_file);
		~Mesh();
		bool load_mesh_from_file(const std::string& input_file);
		bool is_empthy() const;
		bool has_texture() const;
		bool has_normals() const;
		void recalculate_normals();
		void clear();
		bool is_in_gpu() const;
		void send_data_to_gpu();
		GLuint get_vertex_location() const;
		GLuint get_index_location() const;
		void draw_triangles(const GLint& position_location, const GLint& normal_location, const GLint& texture_coordinates_location) const;
		void set_vertices(const std::vector<Vertex>& vertices);
		void set_index(const std::vector<unsigned int>& indices);
		void index_from_triangles(const std::vector<Triangle>& triangles);
		float get_scale_factor() const;
	};
};
#endif