#ifndef MESH_H_
#define MESH_H_

namespace mesh {
	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 textCoord;
	};

	struct Triangle {
		glm::vec3 p_0;
		glm::vec3 p_1;
		glm::vec3 p_2;
	};

	class Mesh {
	private:
		GLuint m_vbo_location;
		GLuint m_index_location;
		bool m_has_normals;
		bool m_has_texture;
		bool m_in_gpu;
		std::vector<Vertex> m_vertices;
		std::vector<unsigned int> m_indices;
		glm::vec3 m_upper_corner;
		glm::vec3 m_lower_corner;
		
		void update_bounding_box();
		void ask_locations();
		void release_locations();
		
	public:
		Mesh();
		Mesh(const std::string& input_file);
		~Mesh();
		bool loadFromFile(const std::string& input_file);
		bool empthy() const;
		bool hasTexture() const;
		bool hasNormals() const;
		void clear();
		bool inGPU() const;
		void sendToGPU();
		GLuint getVertexLocation() const;
		GLuint getIndexLocation() const;
		void drawTriangles(const GLint& position_location, const GLint& normal_location, const GLint& texture_coordinates_location, const GLsizei& instances = 1) const;
		void setVertices(const std::vector<Vertex>& vertices, bool normals = false, bool textCoords = false);
		void setIndex(const std::vector<unsigned int>& indices);
		void indexFromTriangles(const std::vector<Triangle>& triangles);
		float scaleFactor() const;
		glm::vec3 getBBCenter() const;
		glm::vec3 getBBSize() const;
		GLsizei trianglesCount() const;
		GLsizei indicesCount() const;
		GLsizei vertexCount() const;
	};
};
#endif