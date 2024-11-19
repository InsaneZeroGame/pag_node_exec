// pag_node_exec.cpp : Defines the entry point for the application.
//

#include "pag_node_exec.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <gif_lib.h>

int file_width_;
int file_height_;
std::shared_ptr<pag::PAGFile> pagFile;
std::shared_ptr<pag::PAGSurface> pagSurface;
std::shared_ptr<pag::PAGPlayer> pagPlayer;

int64_t TimeToFrame(int64_t time, float frameRate) {
    return static_cast<int64_t>(floor(time * frameRate / 1000000ll));
}


int saveGif_1(const std::string& filename, const std::vector<std::vector<uint8_t>>& frames, int Width, int Height, int delay) {
    int ColorMapSize;
    GifByteType* RedBuffer = NULL, * GreenBuffer = NULL, * BlueBuffer = NULL, * AlphaBuffer = NULL, * OutputBuffer = NULL;
    ColorMapObject* OutputColorMap = NULL;

    // Open the output GIF file
    int Error;
    GifFileType* GifFile;
    if ((GifFile = EGifOpenFileName(filename.c_str(), false, &Error)) == NULL) {
        printf("EGifOpenFileName Error.\n");
        return -1;
    }

    GifFile->SWidth = Width;
    GifFile->SHeight = Height;
    GifFile->SColorResolution = 8;
    GifFile->SBackGroundColor = 0;
    GifFile->SColorMap = NULL;

    unsigned long Size;
    GifByteType* RedP, * GreenP, * BlueP, * AlphaP;
    GifByteType* Buffer, * BufferP;

    Size = ((long)Width) * Height * sizeof(GifByteType);
    if ((RedBuffer = (GifByteType*)malloc((unsigned int)Size)) == NULL ||
        (GreenBuffer = (GifByteType*)malloc((unsigned int)Size)) == NULL ||
        (BlueBuffer = (GifByteType*)malloc((unsigned int)Size)) == NULL ||
        (AlphaBuffer = (GifByteType*)malloc((unsigned int)Size)) == NULL) {
        return -1;
    }
    if ((Buffer = (GifByteType*)malloc(Width * 4)) == NULL) {
        return -1;
    }

    for (int i = 0; i < frames.size(); i++) {
        ColorMapSize = 256;
        RedP = RedBuffer;
        GreenP = GreenBuffer;
        BlueP = BlueBuffer;
        AlphaP = AlphaBuffer;
        int pointer = 0;
        for (int j = 0; j < Height; j++) {
            int k;
            memcpy(Buffer, frames[i].data() + pointer, Width * 4);
            pointer += Width * 4;
            for (k = 0, BufferP = Buffer; k < Width; k++) {
                *BlueP++ = *BufferP++;
                *GreenP++ = *BufferP++;
                *RedP++ = *BufferP++;
                *AlphaP++ = *BufferP++;
            }
        }
        if ((OutputColorMap = GifMakeMapObject(ColorMapSize, NULL)) == NULL ||
            (OutputBuffer = (GifByteType*)malloc(Width * Height *
                sizeof(GifByteType))) == NULL) {
            printf("Failed to allocate memory required, aborted.\n");
            return -1;
        }

        if (GifQuantizeBuffer(Width, Height, &ColorMapSize,
            RedBuffer, GreenBuffer, BlueBuffer,
            OutputBuffer, OutputColorMap->Colors) == GIF_ERROR) {
            printf("GifQuantizeBuffer Error.\n");
            return -1;
        }

        printf("MakeSavedImage：%d\n", i);
        SavedImage* image = GifMakeSavedImage(GifFile, NULL);

        GifImageDesc* imageDesc = (GifImageDesc*)malloc(sizeof(GifImageDesc));
        imageDesc->Left = 0;
        imageDesc->Top = 0;
        imageDesc->Width = Width;
        imageDesc->Height = Height;
        imageDesc->Interlace = false;
        imageDesc->ColorMap = OutputColorMap;

        image->ImageDesc = *imageDesc;
        image->RasterBits = OutputBuffer;

        GraphicsControlBlock* GCB = (GraphicsControlBlock*)malloc(sizeof(GraphicsControlBlock));
        GCB->DisposalMode = DISPOSE_DO_NOT;
        GCB->DelayTime = delay;
        GCB->UserInputFlag = false;

        // Set the transparent color index based on the alpha channel
        int transparentColorIndex = -1;
        for (int j = 0; j < Width * Height; j++) {
            if (AlphaBuffer[j] < 128) { // Adjust the threshold as needed
                transparentColorIndex = OutputBuffer[j];
                break;
            }
        }
        GCB->TransparentColor = transparentColorIndex;

        printf("GCBToSaved：%d\n", i);
        EGifGCBToSavedExtension(GCB, GifFile, i);
    }
    free((char*)RedBuffer);
    free((char*)GreenBuffer);
    free((char*)BlueBuffer);
    free((char*)AlphaBuffer);

    printf("Output GIF file.\n");
    // Output file
    EGifSpew(GifFile);
    return 0;
}

#include <gif_lib.h>
#include <vector>
#include <algorithm>
#include <cmath>

int saveGif(const std::string& filename, const std::vector<std::vector<uint8_t>>& frames, int Width, int Height, int delay) {
    int Error;
    GifFileType* GifFile = EGifOpenFileName(filename.c_str(), false, &Error);
    if (GifFile == NULL) {
        printf("EGifOpenFileName Error: %d.\n", Error);
        return -1;
    }

    // Set the GIF screen parameters with no global color map
    if (EGifPutScreenDesc(GifFile, Width, Height, 8, 0, NULL) == GIF_ERROR) {
        printf("EGifPutScreenDesc Error: %d.\n", GifFile->Error);
        EGifCloseFile(GifFile, &Error);
        return -1;
    }

    // Loop through each frame
    for (size_t i = 0; i < frames.size(); ++i) {
        const std::vector<uint8_t>& frameData = frames[i];
        std::vector<GifByteType> RedBuffer(Width * Height);
        std::vector<GifByteType> GreenBuffer(Width * Height);
        std::vector<GifByteType> BlueBuffer(Width * Height);
        std::vector<GifByteType> AlphaBuffer(Width * Height);

        // Extract color components
        for (int j = 0; j < Width * Height; ++j) {
            // Assuming BGRA format; adjust if your data is in a different format
            BlueBuffer[j] = frameData[4 * j];
            GreenBuffer[j] = frameData[4 * j + 1];
            RedBuffer[j] = frameData[4 * j + 2];
            AlphaBuffer[j] = frameData[4 * j + 3];
        }

        // Apply dithering
        auto applyDithering = [&](std::vector<GifByteType>& colorBuffer) {
            std::vector<float> errorBuffer(Width * Height, 0.0f);
            for (int y = 0; y < Height; y++) {
                for (int x = 0; x < Width; x++) {
                    int idx = y * Width + x;
                    float oldPixel = colorBuffer[idx] + errorBuffer[idx];
                    uint8_t newPixel = static_cast<uint8_t>(std::round(oldPixel / 51.0f) * 51);
                    colorBuffer[idx] = newPixel;
                    float quantError = oldPixel - newPixel;

                    if (x + 1 < Width)
                        errorBuffer[idx + 1] += quantError * 7 / 16;
                    if (y + 1 < Height) {
                        if (x > 0)
                            errorBuffer[idx + Width - 1] += quantError * 3 / 16;
                        errorBuffer[idx + Width] += quantError * 5 / 16;
                        if (x + 1 < Width)
                            errorBuffer[idx + Width + 1] += quantError * 1 / 16;
                    }
                }
            }
            };

        applyDithering(RedBuffer);
        applyDithering(GreenBuffer);
        applyDithering(BlueBuffer);

        // Quantize the image using GifQuantizeBuffer
        int ColorMapSize = 256;
        ColorMapObject* OutputColorMap = GifMakeMapObject(ColorMapSize, NULL);
        if (OutputColorMap == NULL) {
            printf("GifMakeMapObject Error.\n");
            EGifCloseFile(GifFile, &Error);
            return -1;
        }

        std::vector<GifByteType> OutputBuffer(Width * Height);

        if (GifQuantizeBuffer(Width, Height, &ColorMapSize,
            RedBuffer.data(), GreenBuffer.data(), BlueBuffer.data(),
            OutputBuffer.data(), OutputColorMap->Colors) == GIF_ERROR) {
            printf("GifQuantizeBuffer Error.\n");
            GifFreeMapObject(OutputColorMap);
            EGifCloseFile(GifFile, &Error);
            return -1;
        }

        // Set the transparent color index based on the alpha channel
        int transparentColorIndex = -1;
        for (int j = 0; j < Width * Height; ++j) {
            if (AlphaBuffer[j] < 128) { // Adjust threshold as needed
                transparentColorIndex = OutputBuffer[j];
                break;
            }
        }

        // Write the Graphics Control Extension
        GraphicsControlBlock GCB;
        GCB.DisposalMode = DISPOSE_BACKGROUND;
        GCB.UserInputFlag = false;
        GCB.DelayTime = delay;
        GCB.TransparentColor = transparentColorIndex;

        if (EGifGCBToExtension(&GCB, OutputBuffer.data()) == GIF_ERROR) {
            printf("EGifGCBToExtension Error: %d.\n", GifFile->Error);
            GifFreeMapObject(OutputColorMap);
            EGifCloseFile(GifFile, &Error);
            return -1;
        }

        if (EGifPutExtensionLeader(GifFile, GRAPHICS_EXT_FUNC_CODE) == GIF_ERROR ||
            EGifPutExtensionBlock(GifFile, 4, OutputBuffer.data()) == GIF_ERROR ||
            EGifPutExtensionTrailer(GifFile) == GIF_ERROR) {
            printf("EGifPutExtension Error: %d.\n", GifFile->Error);
            GifFreeMapObject(OutputColorMap);
            EGifCloseFile(GifFile, &Error);
            return -1;
        }

        // Write the image descriptor with a local color map
        if (EGifPutImageDesc(GifFile, 0, 0, Width, Height, false, OutputColorMap) == GIF_ERROR) {
            printf("EGifPutImageDesc Error: %d.\n", GifFile->Error);
            GifFreeMapObject(OutputColorMap);
            EGifCloseFile(GifFile, &Error);
            return -1;
        }

        // Write image data
        if (EGifPutLine(GifFile, OutputBuffer.data(), Width * Height) == GIF_ERROR) {
            printf("EGifPutLine Error: %d.\n", GifFile->Error);
            GifFreeMapObject(OutputColorMap);
            EGifCloseFile(GifFile, &Error);
            return -1;
        }

        GifFreeMapObject(OutputColorMap);
    }

    if (EGifCloseFile(GifFile, &Error) == GIF_ERROR) {
        printf("EGifCloseFile Error: %d.\n", Error);
        return -1;
    }

    printf("Output GIF file.\n");
    return 0;
}


int main(int argc, char* argv[])
{
    PagInit();

    bool retFlag;
    PagLoadFile("D:\\datawash\\1.pag");

    auto totalFrames = TimeToFrame(pagFile->duration(), pagFile->frameRate());
    auto currentFrame = 0;
    const int rowSize = file_width_ * 4;
    int bytesLength = pagFile->width() * pagFile->height() * 4;
    std::vector<uint8_t> data(bytesLength);

    pagFile->replaceImage(0, pag::PAGImage::FromPath("C:\\Users\\luke\\Pictures\\2.jpg"));

    std::vector<std::vector<uint8_t>> frames;

    while (currentFrame <= totalFrames) {
        pagPlayer->setProgress(currentFrame * 1.0 / totalFrames);
        auto status = pagPlayer->flush();
        // PAG 渲染数据读取
        pagSurface->readPixels(pag::ColorType::BGRA_8888, pag::AlphaType::Premultiplied, data.data(),
            rowSize);
        std::string imageName = std::to_string(currentFrame);
        BmpWrite(data.data(), pagFile->width(), pagFile->height(), imageName.c_str());
        currentFrame++;
		frames.push_back(data);
    }
    if (saveGif("gifFilename.gif", frames, file_width_, file_height_, 4) != 0) {
        fprintf(stderr, "Failed to save GIF.\n");
        return -1;
    }

}

int PagLoadFile(const std::string FileName)
{
    pagFile = pag::PAGFile::Load(FileName);
    pagSurface = pag::PAGSurface::MakeOffscreen(pagFile->width(), pagFile->height());
    if (pagSurface == nullptr) {
        printf("---pagSurface is nullptr!!!\n");
        return -1;
    }
    pagPlayer = std::make_shared<pag::PAGPlayer>();
    pagPlayer->setSurface(pagSurface);
    pagPlayer->setComposition(pagFile);
    file_height_ = pagFile->height();
    file_width_ = pagFile->width();
    return 0;
}

void PagInit()
{
    // Register fallback fonts. It should be called only once when the application is being initialized.
    std::vector<std::string> fallbackFontPaths = {};
    fallbackFontPaths.emplace_back("/home/luke/Dev/libpag/resources/font/NotoSerifSC-Regular.otf");
    fallbackFontPaths.emplace_back("/home/luke/Dev/libpag/resources/NotoColorEmoji.ttf");
    std::vector<int> ttcIndices(fallbackFontPaths.size());
    pag::PAGFont::SetFallbackFontPaths(fallbackFontPaths, ttcIndices);
}


//PYBIND11_MODULE(PyPag, m) {
//    m.doc() = "PyPag"; // optional module docstring
//
//    m.def("PagInit", &PagInit, "Init Pag Module");
//}
