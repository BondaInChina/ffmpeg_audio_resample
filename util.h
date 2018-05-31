#ifndef UTIL_H
#define UTIL_H
extern "C"
{
#include "libavformat/avformat.h"
}
double avio_r2d(AVRational ration);
bool write_file(const char *path, const uint8_t *src, const int size);
#endif // UTIL_H
