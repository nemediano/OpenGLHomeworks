#ifndef CUBE_MAP_H_
#define CUBE_MAP_H_

#include <string>
#include <GL/glew.h>
#include <GL/gl.h>

namespace image {

class CubeMap {
private:
	unsigned int m_width;
	unsigned int m_height;
	//std::vector<unsigned char> m_data;
	GLuint m_cubemap_id;
	void release_location();
	void ask_locations();
public:
	CubeMap();
	CubeMap(const std::vector<std::string>& input_file_names);
	~CubeMap();
	bool load_texture(const std::vector<std::string>& input_file_names);
	void bind() const;
	//void send_to_gpu();
	int get_width() const;
	int get_height() const;
	GLuint get_id() const;
};
}
#endif