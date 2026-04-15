//
// Created by Raymond on 4/5/2026.
//

#include <format/aft/txp.h>
#include <common/bytestream.h>

float clamp(float val) {
    return val > 255 ? 255 : (val < 0 ? 0 : val);
}

struct TextureInformation txpGetTextureInformationFromMip(uint8_t* mip) {
    struct Bytestream bytestream = bytestreamInit(mip);

    // BEGIN: read mipmap data
    if (bytestreamReadLong(&bytestream, true) != TXP_MAGIC + TxpMagicMipmap)
        return (struct TextureInformation){0};

    struct TextureInformation information = {0};

    information.width = (int)bytestreamReadLong(&bytestream, false);
    information.height = (int)bytestreamReadLong(&bytestream, false);
    information.format = UnsupportedEncoding;
    switch (bytestreamReadLong(&bytestream, false)) {
        case 1: information.format = RGB; break;
        case 2: information.format = RGBA; break;
        case 6: case 7: information.format = DXT1; break;
        case 8: information.format = DXT3; break;
        case 9: information.format = DXT5; break;
        case 10: information.format = ATI1; break;
        case 11: information.format = ATI2; break;
        case 15: information.format = BC7; break;
        default: break;
    }
    bytestream.offset += 8;

    information.buffer = bytestreamReadPointer(&bytestream);
    information.requiresTransformation = true;

    return information;
}

bool txpDetect(uint8_t* buffer, size_t size) {
    struct Bytestream bytestream = bytestreamInit(buffer);
    uint8_t* bufferOffset = buffer;
    if (bytestreamReadLong(&bytestream, true) != TXP_MAGIC + TxpMagicTextureAtlas) {
        uint32_t headerOffset = bytestreamReadLong(&bytestream, false);
        if (headerOffset > size) return false;

        bufferOffset += headerOffset;
        bytestream.data = bufferOffset;
        bytestream.offset = 0;

        if (bytestreamReadLong(&bytestream, true) != TXP_MAGIC + TxpMagicTextureAtlas) return false;
    }
    return true;
}

struct TextureArray txpLoad(uint8_t* buffer, size_t size) {
    struct Bytestream bytestream = bytestreamInit(buffer);

    // BEGIN: verify contents, both cleaned txp and extracted are supported
    uint8_t* bufferOffset = buffer;
    if (bytestreamReadLong(&bytestream, true) != TXP_MAGIC + TxpMagicTextureAtlas) {
        uint32_t headerOffset = bytestreamReadLong(&bytestream, false);;
        if (headerOffset > size)
            goto TxpLoadFailure;

        bufferOffset += headerOffset;
        bytestream.data = bufferOffset;
        bytestream.offset = 0;

        if (bytestreamReadLong(&bytestream, true) != TXP_MAGIC + TxpMagicTextureAtlas)
            goto TxpLoadFailure;
    }

    // BEGIN: read texture atlas data

    uint32_t mapCount = bytestreamReadLong(&bytestream, false);
    bytestreamReadLong(&bytestream, false);

    struct TextureArray array = {0};

    for (size_t index = 0; index < mapCount; index++) {
        uint8_t* bufferTextureOffset = bufferOffset + bytestreamReadLong(&bytestream, false);
        struct Bytestream textureBytestream = bytestreamInit(bufferTextureOffset);

        // BEGIN: read texture data
        if (bytestreamReadLong(&textureBytestream, true) != TXP_MAGIC + TxpMagicTexture) goto TxpLoadFailure;

        uint32_t mipCount = bytestreamReadLong(&textureBytestream, false);
        bytestreamReadLong(&textureBytestream, false);

        struct TextureInformation textureInformation = txpGetTextureInformationFromMip(
            bufferTextureOffset + bytestreamReadLong(&textureBytestream, false)
        );
        if (textureInformation.format == ATI2 && mipCount > 1) {
            // NOTE: the texture is AY/UV, 2 buffer image
            struct TextureInformation alternateTextureInformation = txpGetTextureInformationFromMip(
                bufferTextureOffset + bytestreamReadLong(&textureBytestream, false)
            );
            if (textureInformation.buffer != NULL && alternateTextureInformation.buffer != NULL) {
                // NOTE: the BC6 (ATI2) has to be decoded before we can process it
                textureDecode(&textureInformation);
                textureDecode(&alternateTextureInformation);

                uint8_t* rgba = malloc(textureGetSize(&textureInformation));

                // BEGIN: convert from separate AY / UV channels into a single RGBA
                for (uint32_t y = 0; y < textureInformation.height; y++) {
                    for (uint32_t x = 0; x < textureInformation.width; x++) {
                        uint32_t offsetUV = ((y / 2) * alternateTextureInformation.width + (x / 2)) * 4;
                        uint32_t offsetAY = (y * textureInformation.width + x) * 4;
                        uint8_t* dst = rgba + offsetAY;

                        const float cy = textureInformation.buffer[offsetAY + 0];
                        const float cb = alternateTextureInformation.buffer[offsetUV + 0] - 128;
                        const float cr = alternateTextureInformation.buffer[offsetUV + 1] - 128;

                        dst[0] = (uint8_t)(clamp(cy + (cr * 1.403f)));
                        dst[1] = (uint8_t)(clamp(cy - (cb * 0.344f) - (cr * 0.714f)));
                        dst[2] = (uint8_t)(clamp(cy + (cb * 1.770f)));
                        dst[3] = textureInformation.buffer[offsetAY + 1];
                    }
                }

                struct TextureInformation information = {0};
                information.format = RGBA;
                information.width = textureInformation.width;
                information.height = textureInformation.height;
                information.buffer = rgba;
                information.allocated = true;

                textureArrayAdd(&array, &information);

                textureFree(&textureInformation);
                textureFree(&alternateTextureInformation);
            } else
                goto TxpLoadFailure;
        } else {
            // NOTE: typical texture
            if (!textureInformation.buffer) goto TxpLoadFailure;
            textureDecode(&textureInformation);
            textureArrayAdd(&array, &textureInformation);
        }
    }

    return array;

TxpLoadFailure:
    return (struct TextureArray){0};
};

void txpRegister() {
    struct TextureLoaderImplementation implementation = {0};
    implementation.name = "txp";
    implementation.description = "Project DIVA Arcade Future Tone Texture Format";
    implementation.container = PDAFT_TXP;
    implementation.load = &txpLoad;
    implementation.detect = &txpDetect;
    textureLoadImplementationAdd(implementation);
}