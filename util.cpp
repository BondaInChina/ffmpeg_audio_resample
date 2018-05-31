#include "util.h"
#include <iostream>
using namespace std;
double avio_r2d(AVRational ration)
{
    return ration.den == 0? 0 : (double)ration.num / (double)ration.den;
}

bool write_file(const char *path, const uint8_t *src, const int size)
{
    FILE *file = fopen(path, "ab+");
    if(file == NULL)
    {
        cout << "open file failed: " << path << endl;
        return false;
    }

    int ret = fwrite(src, 1, size, file);
    if(ret < size)
    {
        cout << "write file failed, write " << size << "only "<< ret << "is written!"<< endl;
        fclose(file);
        return false;
    }
    fclose(file);
    return true;
}
