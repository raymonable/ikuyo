//
// Created by Raymond on 4/1/26.
//

#ifndef IKUYO_IMAGE_H
#define IKUYO_IMAGE_H

#include <ikuyo.h>
#include <common/texture.h>

enum ImageContainer {
    NoContainer = 0,
    PNG, WebP
};
struct ImageBuffer {
    enum ImageContainer type;
    size_t size;
    uint8_t* buffer;
};

struct TextureInformation imageResize(struct TextureInformation, int w, int h);

struct ImageBuffer imageBufferInit(enum ImageContainer, uint8_t* data, size_t size);
void imageBufferFree(struct ImageBuffer);

struct ImageBuffer webpGenerate(struct TextureInformation, uint8_t* rgba);
struct ImageBuffer pngGenerate(struct TextureInformation, uint8_t* rgba); // NOTE: handled by bridge

IKUYO_EXPORT struct ImageBuffer imageGenerate(enum ImageContainer, struct TextureInformation, uint8_t* rgba);

#endif //IKUYO_IMAGE_H