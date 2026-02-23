#ifdef WINDOWS_VERSION
#include "ImageUtils.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

int RGBA = STBI_rgb_alpha;
int grey = STBI_grey;

unsigned char* loadImage(const char* filename, int* x, int* y, int* comp, int req_comp)
{
	return stbi_load(filename, x, y, comp, req_comp);
}
#endif