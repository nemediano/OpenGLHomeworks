#ifndef FBO_HANDLER_H_
#define FBO_HANDLER_H_
#include "../textures/TextureHandler.h"
namespace fbo {

class FBOHandler {
private:
	int m_width;
	int m_height;
	GLuint m_fbo_id;
	texture::TextureHandler* m_texture_handler_ptr;
	//unsigned char m_texture[256][256];
	GLuint m_depth_buffer_id;
	//GLuint m_texture_id;
	//bool create_texture();
public:
	
	FBOHandler(int width, int height);
	virtual ~FBOHandler();
	void render(bool texture_animation, float time);
	int get_width();
	int get_height();
	bool check_frame_buffer_status();
	GLuint get_texture_id();
};

}
#endif