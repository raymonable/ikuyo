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
    DXT1,
    DXT3,
    DXT5,
    BC7
};

enum TextureContainer {
    UnknownContainer = 0,
    DDS,
    UnityAssetBundle,
    UE4
};

struct TextureInformation {
    uint8_t* buffer;
    bool mustFreeBuffer;

    int width;
    int height;

    bool requiresTransformation;

    // NOTE: depth can be inferred from encoding
    enum TextureEncoding format;
};

IKUYO_EXPORT uint8_t* textureDecode(struct TextureInformation);
IKUYO_EXPORT void textureFree(struct TextureInformation);
IKUYO_EXPORT struct TextureInformation textureLoad(enum TextureContainer, uint8_t* data, size_t size);

size_t textureGetSize(struct TextureInformation);
bool textureHasAlpha(struct TextureInformation);

#endif //IKUYO_TEXTURE_H