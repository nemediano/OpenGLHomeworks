#include <cassert>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "FBOHandler.h"
#include "../textures/TextureHandler.h"
using namespace std;

namespace fbo {
	FBOHandler::FBOHandler(int width, int height) {
		assert(width > 0);
		assert(height > 0);
		m_width = width;
		m_height = height;
		//create_texture();
		m_texture_handler_ptr = new texture::TextureHandler(m_width, m_height);
		//Create the FBO object
		glGenRenderbuffers(1, &m_depth_buffer_id);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depth_buffer_id);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
		glGenFramebuffers(1, &m_fbo_id);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_id);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture_handler_ptr->get_id(), 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_buffer_id);
		check_frame_buffer_status();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	FBOHandler::~FBOHandler() {
		//Unbind frame buffer just in case
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		if (m_texture_handler_ptr != nullptr) {
			delete m_texture_handler_ptr;
			m_texture_handler_ptr = nullptr;
		}
		m_width = 0;
		m_height = 0;
	}

	bool FBOHandler::check_frame_buffer_status() {
		GLenum status;
		status = (GLenum) glCheckFramebufferStatus(GL_FRAMEBUFFER);
		switch(status) {
			
			case GL_FRAMEBUFFER_COMPLETE:
			return true;
			
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
				cout << "Framebuffer incomplete, incomplete attachment" << endl;
			return false;

			case GL_FRAMEBUFFER_UNSUPPORTED:
				cout << "Unsupported framebuffer format" << endl;
			return false;
			
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
				cout << "Framebuffer incomplete, missing attachment" << endl;
			return false;
			
			/*case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
				cout << "Framebuffer incomplete, attached images must have same dimensions" << endl;
			return false;
			
			case GL_FRAMEBUFFER_INCOMPLETE_FORMATS:
				cout << "Framebuffer incomplete, attached images must have same format" << endl;
			return false;*/
			
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
				cout << "Framebuffer incomplete, missing draw buffer" << endl;
			return false;
			
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
				cout << "Framebuffer incomplete, missing read buffer" << endl;
			return false;
		}
		return false;
	}

	void FBOHandler::render(bool texture_animation, float time) {
	   glUseProgram(0); //This is very important return to fix pipeline before drawing teapot
	   
	   glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_id);
       glDrawBuffer(GL_COLOR_ATTACHMENT0); //select which attachment to render into
	   glViewport(0, 0, m_width, m_height); //set viewport to size of attachedtexture
	   glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	   //Some old school OpenGL for nice teapot!!
	   glEnable(GL_POLYGON_SMOOTH);
	   glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	   glEnable(GL_BLEND);
       glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	   
	   glEnable(GL_COLOR_MATERIAL);
	   glEnable(GL_LIGHTING);
	   glEnable(GL_LIGHT0);
	   glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
       glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
       glEnable(GL_DEPTH_TEST);

	   glMatrixMode(GL_PROJECTION);
       glLoadIdentity();
       glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0); 
       glMatrixMode(GL_MODELVIEW);
       glLoadIdentity();
	   glColor3f(0.0, 0.0, 1.0);
	   glPushMatrix();
	      glTranslatef(0.0f, 0.15f, 0.0f);
	      if (texture_animation) {
			  glRotatef(time * 30.0f, 0.0f, 1.0f, 0.0f);
		      glRotatef(-45.0f, 1.0f, 0.0f, 0.0f);
	      }
          glutSolidTeapot(0.2f); //draw objects into FBO
       glPopMatrix();
	   //To prevent side effects
	   glDisable(GL_LIGHT0);
	   glDisable(GL_LIGHTING);
	   glDisable(GL_COLOR_MATERIAL);
	   glDisable(GL_BLEND);
	   glDisable(GL_POLYGON_SMOOTH);

	   glBindFramebuffer(GL_FRAMEBUFFER, 0); //unbind FBO
	   glDrawBuffer(GL_BACK); //resume drawing to back buffer
	}

	int FBOHandler::get_width() {
		return m_width;
	}
	
	int FBOHandler::get_height() {
		return m_height;
	}

	GLuint FBOHandler::get_texture_id() {
		if (m_texture_handler_ptr == nullptr) {
			return 0;
		}
		else {
			return m_texture_handler_ptr->get_id();
		}
	}
}
