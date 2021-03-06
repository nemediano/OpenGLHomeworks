#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <string>
#include <GL/glew.h>
#include <GL/gl.h>

namespace image {
	class Texture {
	private:
		unsigned int m_width;
		unsigned int m_height;
		unsigned char* m_data;
		GLuint m_texture_id;
		void send_to_gpu();
		void release_location();
		void ask_locations();
	public:
		Texture();
		Texture(const std::string& input_file_name);
		~Texture();
		bool load_texture(const std::string& input_file_name);
		void bind() const;
		int get_width() const;
		int get_height() const;
	};
}
#endif