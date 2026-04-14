//
// Created by Raymond on 3/29/2026.
//

#ifndef IKUYO_TEXTURE_H
#define IKUYO_TEXTURE_H

#include <ikuyo.h>

enum TextureEncoding {
    UnsupportedEncoding = 0,
    RGB, RGBA,
    BGR, BGRA,
    DXT1, DXT3, DXT5,
    ATI1, ATI2,
    BC7
};

enum TextureContainer {
    UnknownContainer = 0,
    DDS,
    UnityAssetBundle,
    UE4, FArC
};

struct TextureInformation {
    uint8_t* buffer;
    bool allocated;

    int width;
    int height;

    bool requiresTransformation;

    // NOTE: depth can be inferred from encoding
    enum TextureEncoding format;
};

struct TextureArray {
    uint32_t count;
    struct TextureInformation* data;
};

struct TextureLoaderImplementation {
    const char* name;
    const char* description;
    enum TextureContainer container;

    bool (*detect)(uint8_t* buffer, size_t size);
    struct TextureArray (*load)(uint8_t* buffer, size_t size);
};

void textureLoadImplementationsInit();

void textureLoadImplementationAdd(struct TextureLoaderImplementation);
IKUYO_EXPORT enum TextureContainer textureContainerGetFromString(const char*);
IKUYO_EXPORT struct TextureArray textureLoad(enum TextureContainer, uint8_t* data, size_t size);

IKUYO_EXPORT void textureArrayAdd(struct TextureArray*, struct TextureInformation*);
IKUYO_EXPORT void textureArrayFree(struct TextureArray*);

IKUYO_EXPORT struct TextureInformation* textureDecode(struct TextureInformation*);
IKUYO_EXPORT void textureFree(struct TextureInformation*);

struct TextureInformation* textureResize(struct TextureInformation*, int w, int h);

size_t textureGetSize(struct TextureInformation*);
bool textureHasAlpha(struct TextureInformation*);

#endif //IKUYO_TEXTURE_H