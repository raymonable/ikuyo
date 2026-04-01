//
// Created by Raymond on 4/1/26.
//

#include <common/image.h>

#include <webp/mux.h>
#include <webp/encode.h>

struct ImageBuffer imageBufferInit(enum ImageContainer type, uint8_t* data, size_t size) {
    struct ImageBuffer imageBuffer;
    imageBuffer.type = type;
    imageBuffer.size = size;
    imageBuffer.buffer = malloc(size);
    memcpy(imageBuffer.buffer, data, imageBuffer.size);
    return imageBuffer;
};
void imageBufferFree(struct ImageBuffer imageBuffer) {
    if (imageBuffer.buffer != NULL)
        free(imageBuffer.buffer);
};

struct ImageBuffer webpGenerate(struct TextureInformation information, uint8_t* rgba) {
    WebPPicture picture = {0};
    WebPConfig config = {0};

    // TODO: add configurable settings for lossless and resize modes
    if (!WebPConfigPreset(&config, WEBP_PRESET_ICON, 75)) goto WebpError;
    if (!WebPValidateConfig(&config)) goto WebpError;

    picture.width = information.width;
    picture.height = information.height;

    picture.use_argb = true;
    WebPPictureAlloc(&picture);

    for (size_t y = 0; information.height > y; y++)
        for (size_t x = 0; information.width > x; x++) {
            size_t offset = y * information.width + x;
            const uint8_t* src = rgba + offset * 4;
            picture.argb[offset] = (255 << 24) | (src[0] << 16) | (src[1] << 8) | src[2];
        }

    WebPMemoryWriter memoryWriter = {0};
    picture.writer = WebPMemoryWrite;
    picture.custom_ptr = (void*)&memoryWriter;

    if (!WebPEncode(&config, &picture)) goto WebpError;

    struct ImageBuffer imageBuffer = imageBufferInit(WebP, memoryWriter.mem, memoryWriter.size);

    WebPMemoryWriterClear(&memoryWriter);
    WebPPictureFree(&picture);

    return imageBuffer;
WebpError:
    return (struct ImageBuffer){0};
};

struct ImageBuffer imageGenerate(enum ImageContainer type, struct TextureInformation information, uint8_t* rgba) {
    switch (type) {
        case PNG: return pngGenerate(information, rgba);
        case WebP: return webpGenerate(information, rgba);
        default: break;
    }
    return (struct ImageBuffer){0};
};