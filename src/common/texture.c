//
// Created by Raymond on 3/29/2026.
//

#include <ikuyo.h>
#include <common/texture.h>

#define BCDEC_IMPLEMENTATION
#include <bcdec.h>

void* textureDecode(struct TextureInformation information, void* buffer) {
    if (information.format == UnsupportedEncoding)
        return NULL;
    uint8_t* rgba = malloc(textureGetSize(information));
    memset(rgba, 0, textureGetSize(information));
    uint8_t* src = buffer;

    size_t blockSize = information.format >= (int)DXT1 ? 4 : 1;
    bool formatHasAlpha = !(information.format == RGB || information.format == BGR);

    for (uint32_t y = 0; information.height > y; y += blockSize)
        for (uint32_t x = 0; information.width > x; x += blockSize) {
            uint8_t* dst = rgba + (y * information.width + x) * 4;
            switch (information.format) {
                // Uncompressed formats
                case RGB:
                case RGBA:
                    memcpy(dst, src, 3);
                    // NOTE: Always assume 100% opacity by default if alpha channel is unavailable
                    dst[3] = formatHasAlpha ? src[3] : 255;
                    break;
                case BGR:
                case BGRA:
                    dst[0] = src[2]; dst[1] = src[1]; dst[2] = src[0];
                    // NOTE: Always assume 100% opacity by default if alpha channel is unavailable
                    dst[3] = formatHasAlpha ? src[3] : 255;
                    break;
                // Compressed formats
                case DXT1:
                    bcdec_bc1(src, dst, information.width * 4); break;
                case DXT3:
                    bcdec_bc2(src, dst, information.width * 4); break;
                case DXT5:
                    bcdec_bc3(src, dst, information.width * 4); break;
                case BC7:
                    bcdec_bc7(src, dst, information.width * 4); break;
                default: break;
            }
            int test = (information.format != DXT1 ? (blockSize * (formatHasAlpha ? 4 : 3)) : 8);
            src = src + test;
        }

    if (information.requiresTransformation) {
        char* ref = malloc(textureGetSize(information));
        memcpy(ref, rgba, textureGetSize(information));
        for (uint32_t y = 0; y < information.height; ++y)
            for (uint32_t x = 0; x < information.width; ++x)
                memcpy(
                    rgba + (y * information.width + x) * 4,
                    ref + ((information.height - 1 - y) * information.width + x) * 4,
                    4
                );
        free(ref);
    }

    return rgba;
};


size_t textureGetSize(struct TextureInformation information) {
    return information.width * information.height * 4;
};