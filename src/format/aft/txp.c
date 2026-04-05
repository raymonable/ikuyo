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
        }
    }


TxpLoadFailure:
    return (struct TextureInformation){0};
};