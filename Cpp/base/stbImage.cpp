#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include <string>

int main()
{
    int nW, nH, nChannels;
    unsigned char* data = stbi_load("E:/testFile/testLargePS.jpg", &nW, &nH, &nChannels, 0);

    unsigned char* data2 = stbi_load("E:/md.jpg", &nW, &nH, &nChannels, 0);

    auto sz = sizeof(data);
    // auto szi = strlen(data);

    return 0;
}