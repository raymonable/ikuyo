//
// Created by Raymond on 3/29/2026.
//

#include <ikuyo.h>
#include <common/texture.h>

#include <format/dds/dds.h>
#include <format/unityfs/assetbundle.h>
#include <format/ue4/uexp.h>
#include <format/aft/farc.h>
#include <format/aft/txp.h>

#define BCDEC_IMPLEMENTATION
#include <bcdec.h>

struct TextureInformation textureLoad(enum TextureContainer container, uint8_t* buffer, size_t size) {
    switch (container) {
        case DDS:
            return ddsReadBuffer(buffer);
        case UE4:
            return uexpReadBuffer(buffer, size);
        case UnityAssetBundle: {
            struct AssetBundle ab = assetBundleParse(buffer); break;
            // TODO: extract immediate texture2d if only one is available
        }
        case FArC: {
            uint8_t* txpBuffer = farcReadBuffer(buffer);
            struct TextureInformation information = txpReadBuffer(txpBuffer);
            //free(txpBuffer);
            break;
        }
        default: break;
    }
    return (struct TextureInformation){0};
};

void textureFree(struct TextureInformation information) {
    if (information.mustFreeBuffer)
        free(information.buffer);
}

uint8_t* textureDecode(struct TextureInformation information) {
    if (information.format == UnsupportedEncoding)
        return NULL;
    uint8_t* rgba = malloc(textureGetSize(information));
    memset(rgba, 255, textureGetSize(information));
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
                    if (hasAlpha)
                        dst[3] = src[3];
                    break;
                case BGR:
                case BGRA:
                    dst[0] = src[2]; dst[1] = src[1]; dst[2] = src[0];
                    if (hasAlpha)
                        dst[3] = src[3];
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
        uint8_t* ref = malloc(textureGetSize(information));
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