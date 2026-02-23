#ifdef WINDOWS_VERSION
#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

extern int RGBA;
extern int grey;

unsigned char* loadImage(const char* filename, int* x, int* y, int* comp, int req_comp);

#endif // IMAGEUTILS_H
#endif