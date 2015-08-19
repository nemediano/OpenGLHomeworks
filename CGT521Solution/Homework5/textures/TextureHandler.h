#ifndef TEXTURE_HANDLER_H_
#define TEXTURE_HANDLER_H_

namespace texture {

class TextureHandler {
private:
	unsigned char* m_texture_data;
	int m_width;
	int m_height;
	GLuint m_texture_id;
	
public:
	TextureHandler(int width, int height);
	virtual ~TextureHandler();
	int get_width();
	GLuint get_id();
	int get_height();
	unsigned char* get_texture();
};

}
#endif