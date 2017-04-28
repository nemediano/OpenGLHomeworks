#ifndef PROCEDURAL_TEXTURES_H_
#define PROCEDURAL_TEXTURES_H_

#include "Texture.h"

namespace image {
	Texture defaultStencil(unsigned int size = 1024);
	Texture chessBoard(unsigned int size = 1024, unsigned int cells = 8);
}

#endif