// pag_node_exec.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <string>
#include <pag/pag.h>

// TODO: Reference additional headers your program requires here.

#include <cstdint>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <bitset>
#include <stdexcept>


inline void BmpWrite(unsigned char* image, int imageWidth, int imageHeight, const char* filename) {
    unsigned char header[54] = { 0x42, 0x4d, 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0, 40, 0, 0, 0,
                                0,    0,    0, 0, 0, 0, 0, 0, 1, 0, 32, 0, 0, 0, 0,  0, 0, 0,
                                0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0 };

    int64_t file_size = static_cast<int64_t>(imageWidth) * static_cast<int64_t>(imageHeight) * 4 + 54;
    header[2] = static_cast<unsigned char>(file_size & 0x000000ff);
    header[3] = (file_size >> 8) & 0x000000ff;
    header[4] = (file_size >> 16) & 0x000000ff;
    header[5] = (file_size >> 24) & 0x000000ff;

    int64_t width = imageWidth;
    header[18] = width & 0x000000ff;
    header[19] = (width >> 8) & 0x000000ff;
    header[20] = (width >> 16) & 0x000000ff;
    header[21] = (width >> 24) & 0x000000ff;

    int64_t height = -imageHeight;
    header[22] = height & 0x000000ff;
    header[23] = (height >> 8) & 0x000000ff;
    header[24] = (height >> 16) & 0x000000ff;
    header[25] = (height >> 24) & 0x000000ff;

    char fname_bmp[128];
    snprintf(fname_bmp, 128, "%s.bmp", filename);

    FILE* fp;
    if (!(fp = fopen(fname_bmp, "wb"))) {
        return;
    }

    fwrite(header, sizeof(unsigned char), 54, fp);
    fwrite(image, sizeof(unsigned char), (size_t)(int64_t)imageWidth * imageHeight * 4, fp);
    fclose(fp);
}

void PagInit();

int PagLoadFile(const std::string FileName);


