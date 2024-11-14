// pag_node_exec.cpp : Defines the entry point for the application.
//

#include "pag_node_exec.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int file_width_;
int file_height_;

int64_t TimeToFrame(int64_t time, float frameRate) {
    return static_cast<int64_t>(floor(time * frameRate / 1000000ll));
}

void BmpWrite(unsigned char* image, int imageWidth, int imageHeight, const char* filename) {
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

int main(int argc, char* argv[])
{
    // Register fallback fonts. It should be called only once when the application is being initialized.
    std::vector<std::string> fallbackFontPaths = {};
    fallbackFontPaths.emplace_back("/home/luke/Dev/libpag/resources/font/NotoSerifSC-Regular.otf");
    fallbackFontPaths.emplace_back("/home/luke/Dev/libpag/resources/NotoColorEmoji.ttf");
    std::vector<int> ttcIndices(fallbackFontPaths.size());
    pag::PAGFont::SetFallbackFontPaths(fallbackFontPaths, ttcIndices);

    auto pagFile = pag::PAGFile::Load("/home/luke/Dev/assets/test.pag");
    auto pagSurface = pag::PAGSurface::MakeOffscreen(pagFile->width(), pagFile->height());
    if (pagSurface == nullptr) {
        printf("---pagSurface is nullptr!!!\n");
        return -1;
    }
    auto pagPlayer = new pag::PAGPlayer();
    pagPlayer->setSurface(pagSurface);
    pagPlayer->setComposition(pagFile);

    auto totalFrames = TimeToFrame(pagFile->duration(), pagFile->frameRate());
    auto currentFrame = 0;
    const int rowSize = file_width_ * 4;
    int bytesLength = pagFile->width() * pagFile->height() * 4;
    std::vector<uint8_t> data(bytesLength);

    pagFile->replaceImage(0, pag::PAGImage::FromPath("/home/luke/Dev/assets/gif/ComfyUI_temp_ajsff_00001_.png"));

    while (currentFrame <= totalFrames) {
        pagPlayer->setProgress(currentFrame * 1.0 / totalFrames);
        auto status = pagPlayer->flush();
        // PAG 渲染数据读取
        pagSurface->readPixels(pag::ColorType::BGRA_8888, pag::AlphaType::Premultiplied, data.data(),
            rowSize);
        std::string imageName = std::to_string(currentFrame);
        BmpWrite(data.data(), pagFile->width(), pagFile->height(), imageName.c_str());
        currentFrame++;
    }
    delete pagPlayer;
}
