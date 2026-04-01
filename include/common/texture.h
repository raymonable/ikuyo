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
    UnityAssetBundle
};

struct TextureInformation {
    unsigned int width;
    unsigned int height;

    bool requiresTransformation;

    // NOTE: depth can be inferred from encoding
    enum TextureEncoding format;
};

void* textureDecode(struct TextureInformation, void*);
size_t textureGetSize(struct TextureInformation);
bool textureHasAlpha(struct TextureInformation);

#endif //IKUYO_TEXTURE_H