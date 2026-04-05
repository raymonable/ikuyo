//
// Created by Raymond on 4/5/2026.
//

#include <format/aft/txp.h>
#include <common/bytestream.h>

#include <stdio.h>
struct TextureInformation txpReadBuffer(uint8_t* buffer) {
    // TODO: refactor to not include headerOffset, as that should be handled by the FARC decompressor and NOT the TXP reader
    struct Bytestream bytestream = bytestreamInit(buffer);

    bytestream.offset += 4;
    uint32_t headerOffset = bytestreamReadLong(&bytestream, false);
    bytestream.offset = headerOffset;

    // BEGIN: read texture atlas data
    if (bytestreamReadLong(&bytestream, true) != TXP_MAGIC + TxpMagicTextureAtlas) goto TxpLoadFailure;

    uint32_t mapCount = bytestreamReadLong(&bytestream, false);
    bytestreamReadLong(&bytestream, false);

    // NOTE: txp can contain multiple textures in a single file, so it has to be more complex than a simple TextureInformation
    for (size_t index = 0; index < mapCount; index++) {
        struct Bytestream textureBytestream = bytestreamInit(buffer);
        uint32_t textureFileOffset = bytestreamReadLong(&bytestream, false);;
        textureBytestream.offset = headerOffset + textureFileOffset;

        // BEGIN: read texture data
        if (bytestreamReadLong(&textureBytestream, true) != TXP_MAGIC + TxpMagicTexture) goto TxpLoadFailure;

        uint32_t mipCount = bytestreamReadLong(&textureBytestream, false);
        bytestreamReadLong(&textureBytestream, false);

        for (size_t mipIndex = 0; mipIndex < mipCount; mipIndex++) {
            struct Bytestream mipBytestream = bytestreamInit(buffer);
            mipBytestream.offset = headerOffset + textureFileOffset + bytestreamReadLong(&textureBytestream, false);

            // BEGIN: read mipmap data
            if (bytestreamReadLong(&mipBytestream, true) != TXP_MAGIC + TxpMagicMipmap) goto TxpLoadFailure;

            uint32_t width = bytestreamReadLong(&mipBytestream, false);
            uint32_t height = bytestreamReadLong(&mipBytestream, false);
            uint32_t format = bytestreamReadLong(&mipBytestream, false);
            uint32_t id = bytestreamReadLong(&mipBytestream, false);
            uint32_t bufferSize = bytestreamReadLong(&mipBytestream, false);

            enum TextureEncoding encoding = UnsupportedEncoding;
            switch (format) {
                case 1: encoding = RGB; break;
                case 2: encoding = RGBA; break;
                case 6: case 7: encoding = DXT1; break;
                case 8: encoding = DXT3; break;
                case 9: encoding = DXT5; break;
                // NOTE: our target is AFT, this is redundant as this only appears in MM+ , but it's only an extra statement
                case 15: encoding = BC7; break;
                default: break;
            }

            // NOTE: next is the data... to be implemented
        }
    }


TxpLoadFailure:
    return (struct TextureInformation){0};
};