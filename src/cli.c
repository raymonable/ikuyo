//
// Created by Raymond on 3/29/2026.
//

#include <ikuyo.h>
#include <common/image.h>
#include <common/texture.h>

#include <format/dds/dds.h>
#include <format/unityfs/assetbundle.h>
#include <format/ue4/uexp.h>

#include <stdio.h>

const char* displayedSupportedInputContainers[] = {
    "dds (DirectDraw Surface, DirectX)",
    "ab (Unity AssetBundle)"
};

const char* displayedSupportOutputContainers[] = {
    "png (fast, low compression)",
    "webp (slow, high compression)"
};

#ifndef WIN32
#define fopen_s(a,b,c) *(a) = fopen(b,c)
#define _strcmpi strcasecmp

#include <sys/time.h>
long long timeInMilliseconds(void) {
    struct timeval tv;

    gettimeofday(&tv,NULL);
    return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}
#else
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
long long timeInMilliseconds() {
    return GetTickCount64();
}
#endif

int main(const int argc, const char* argv[]) {
    if (argc < 3 || _strcmpi(argv[1], "-h") == 0 || _strcmpi(argv[1], "--help") == 0) {
#ifdef WIN32
        const char* executable = strrchr(argv[0], '\\');
#else
        const char* executable = strrchr(argv[0], '/');
#endif
        if (executable)
            ++executable;
        else
            executable = argv[0];

        printf("Usage:\n\n%s [format] [input file] [output format]\n\nSupported texture input containers:\n", executable);
        for (size_t index = 0; index < sizeof(displayedSupportedInputContainers) / sizeof(displayedSupportedInputContainers[0]); ++index)
            printf("* %s\n", displayedSupportedInputContainers[index]);
        printf("\nSupported image output containers:\n");
        for (size_t index = 0; index < sizeof(displayedSupportOutputContainers) / sizeof(displayedSupportOutputContainers[0]); ++index)
            printf("* %s\n", displayedSupportOutputContainers[index]);

        return 1;
    }

    const char* inputFormatString = argv[1];
    const char* inputFileName = argv[2];
    const char* outputFileFormat = argv[3];

    enum TextureContainer textureContainer = UnknownContainer;
    if (_strcmpi(inputFormatString, "dds") == 0) textureContainer = DDS;
    if (_strcmpi(inputFormatString, "ab") == 0) textureContainer = UnityAssetBundle;
    if (_strcmpi(inputFormatString, "ue4") == 0) textureContainer = UE4;
    if (textureContainer == UnknownContainer) {
        fprintf(stderr, "Unknown texture container format '%s'\n", inputFormatString);
        return 1;
    }

    enum ImageContainer imageFormat = NoContainer;
    if (_strcmpi(outputFileFormat, "png") == 0) imageFormat = PNG;
    if (_strcmpi(outputFileFormat, "webp") == 0) imageFormat = WebP;
    if (imageFormat == NoContainer) {
        fprintf(stderr, "Unknown image container format '%s'\n", inputFormatString);
        return 1;
    }

    // Access input file

    FILE* file = NULL;
    fopen_s(&file, inputFileName, "r");
    if (!file) {
        fprintf(stderr, "Unable to open file '%s'\n", inputFileName);
        return 1;
    }
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t* buffer = malloc(fileSize);
    fread(buffer, 1, fileSize, file);
    fclose(file);

    // Decompress file based on container format

    void* compressedTextureBuffer = NULL;
    struct TextureInformation textureInformation = {0};

    uint64_t startTime = timeInMilliseconds();

    switch (textureContainer) {
        case DDS: {
            struct DDS dds = ddsReadBuffer(buffer);
            textureInformation = dds.information;
            compressedTextureBuffer = dds.buffer;
            break;
        }
        case UE4: {
            struct UnrealEngineTexture ue = uexpReadBuffer(buffer, fileSize);
            textureInformation = ue.information;
            compressedTextureBuffer = ue.buffer;
            break;
        }
        case UnityAssetBundle: {
            struct AssetBundle ab = assetBundleParse(buffer);
            break;
        }
        default: break;
    }

    if (!compressedTextureBuffer) {
        fprintf(stderr, "Unable to read texture\n");
        return 1;
    }

    void* outputBuffer = textureDecode(textureInformation, compressedTextureBuffer);
    if (!outputBuffer) {
        fprintf(stderr, "Unable to decode texture\n");
        return 1;
    }
    printf("Decoded texture\n");

    // TODO: per-format cleanup routines

    char* outputFileName = malloc(strlen(inputFileName) + 8);
    memset(outputFileName, 0, strlen(inputFileName) + 8);
    memcpy(outputFileName, inputFileName, strlen(inputFileName));

    struct ImageBuffer imageBuffer = {0};

    switch (imageFormat) {
        case PNG: memcpy(outputFileName + strlen(inputFileName), ".png", 4); break;
        case WebP: memcpy(outputFileName + strlen(inputFileName), ".webp", 5); break;
        default: break;
    }
    imageBuffer = imageGenerate(imageFormat, textureInformation, outputBuffer);

    if (imageBuffer.buffer != NULL) {
        printf("Decoding & re-encoding took %llu ms\n", timeInMilliseconds() - startTime);

        fopen_s(&file, outputFileName, "wb");
        fwrite(imageBuffer.buffer, imageBuffer.size, 1, file);
        fclose(file);
    } else {
        fprintf(stderr, "Failed to encode image");
        return 1;
    }

    free(outputBuffer);
    free(outputFileName);

    imageBufferFree(imageBuffer);

    return 0;
}