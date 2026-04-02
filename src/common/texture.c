//
// Created by Raymond on 3/29/2026.
//

#include <ikuyo.h>
#include <common/texture.h>

#define BCDEC_IMPLEMENTATION
#include <bcdec.h>

uint8_t* textureDecode(struct TextureInformation information) {
    if (information.format == UnsupportedEncoding)
        return NULL;
    uint8_t* rgba = malloc(textureGetSize(information));
    memset(rgba, 0, textureGetSize(information));
    uint8_t* src = information.buffer;

    size_t blockSize = information.format >= (int)DXT1 ? 4 : 1;
    bool hasAlpha = textureHasAlpha(information);

    for (uint32_t y = 0; information.height > (y + blockSize - 1); y += blockSize)
        for (uint32_t x = 0; information.width > (x + blockSize - 1); x += blockSize) {
            uint8_t* dst = rgba + (y * information.width + x) * 4;
            switch (information.format) {
                // Uncompressed formats
                case RGB:
                case RGBA:
                    memcpy(dst, src, 3);
                    // NOTE: Always assume 100% opacity by default if alpha channel is unavailable
                    dst[3] = hasAlpha ? src[3] : 255;
                    break;
                case BGR:
                case BGRA:
                    dst[0] = src[2]; dst[1] = src[1]; dst[2] = src[0];
                    // NOTE: Always assume 100% opacity by default if alpha channel is unavailable
                    dst[3] = hasAlpha ? src[3] : 255;
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
            src += (information.format != DXT1 ? (blockSize * (hasAlpha ? 4 : 3)) : 8);
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
bool textureHasAlpha(struct TextureInformation information) {
    return !(information.format == RGB || information.format == BGR);
}
size_t textureGetSize(struct TextureInformation information) {
    return information.width * information.height * 4;
};