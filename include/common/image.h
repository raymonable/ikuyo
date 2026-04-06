//
// Created by Raymond on 4/1/26.
//

#ifndef IKUYO_IMAGE_H
#define IKUYO_IMAGE_H

#include <ikuyo.h>
#include <common/texture.h>

enum ImageContainer {
    NoContainer = 0,
    PNG, WebP, AVIF
};
struct ImageBuffer {
    enum ImageContainer type;
    size_t size;
    uint8_t* buffer;
};

struct ImageBuffer imageBufferInit(enum ImageContainer, uint8_t* data, size_t size);
IKUYO_EXPORT void imageBufferFree(struct ImageBuffer);

struct ImageBuffer avifGenerate(struct TextureInformation);
struct ImageBuffer webpGenerate(struct TextureInformation);
struct ImageBuffer pngGenerate(struct TextureInformation); // NOTE: handled by bridge

IKUYO_EXPORT struct ImageBuffer imageGenerate(enum ImageContainer, struct TextureInformation);

#endif //IKUYO_IMAGE_H