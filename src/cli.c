//
// Created by Raymond on 3/29/2026.
//

#include <ikuyo.h>
#include <bridge.h>
#include <common/texture.h>

#include <format/dds/dds.h>

#include <stdio.h>

const char* displayedSupportedInputContainers[] = {
    "dds (DirectDraw Surface, DirectX)",
    "ab (Unity AssetBundle)"
};

const char* displayedSupportOutputContainers[] = {
    "png", "webp"
};

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

    switch (textureContainer) {
        case DDS:
            struct DDS dds = ddsReadBuffer(buffer);
            textureInformation = dds.information;
            compressedTextureBuffer = dds.buffer;
            break;
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

    // TODO: migrate to a centralized function for generating file buffers
    switch (imageFormat) {
        case PNG:
            memcpy(outputFileName + strlen(inputFileName), ".png", 4);
            ikuyo_fpng_encode_image_to_file(outputFileName, outputBuffer, textureInformation.width, textureInformation.height, 4, 0);
            break;
        default: break;
    }

    free(outputBuffer);
    free(outputFileName);

    return 0;
}