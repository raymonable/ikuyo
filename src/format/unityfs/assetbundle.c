//
// Created by Raymond on 4/1/26.
//

#include <ikuyo.h>
#include <format/unityfs/assetbundle.h>
#include <common/bytestream.h>

#include <lz4.h> // NOTE: lz4hc appears to be included in here for decompression

void assetBundleDecompress(enum AssetBundleCompression type, uint8_t* input, size_t inputSize, uint8_t* output, size_t outputSize) {
    switch (type) {
        case AssetBundleNoCompression:
            memcpy(output, input, inputSize); break;
         // TODO: LZMA
        case AssetBundleLZ4HC:
        case AssetBundleLZ4:
            LZ4_decompress_safe((char*)input, (char*)output, inputSize, outputSize);
            break;
        default: break;
    }
}

struct AssetBundle assetBundleParse(uint8_t* buffer) {
    struct AssetBundle assetBundle = {0};

    if (strcmp((const char*)buffer, "UnityFS\0") != 0)
        goto AssetBundleFailure;

    struct Bytestream bytestream = bytestreamInit(buffer + 8);

    // NOTE: begin read header
    assetBundle.version = bytestreamReadLong(&bytestream, true);
    assetBundle.majorVersion = bytestreamReadString(&bytestream);
    assetBundle.revisionVersion = bytestreamReadString(&bytestream);

    uint64_t size = bytestreamReadLongLong(&bytestream, true);
    uint32_t compressedBlockInfoSize = bytestreamReadLong(&bytestream, true);
    uint32_t decompressedBlockInfoSize = bytestreamReadLong(&bytestream, true);
    uint32_t flags = bytestreamReadLong(&bytestream, true);

    // NOTE: begin read block info
    uint8_t* compressedBlockInfo = buffer + size - compressedBlockInfoSize;
    if (!(flags & 0x80)) {
        // NOTE: block info is next up
        compressedBlockInfo = bytestreamReadPointer(&bytestream);
        bytestream.offset += compressedBlockInfoSize;
    }

    uint8_t* decompressedBlockInfo = malloc(decompressedBlockInfoSize);
    memset(decompressedBlockInfo, 0, decompressedBlockInfoSize);
    assetBundleDecompress(
        flags & 0x3F,
        compressedBlockInfo, compressedBlockInfoSize,
        decompressedBlockInfo, decompressedBlockInfoSize
    );

    return assetBundle;
AssetBundleFailure:
    return (struct AssetBundle){0};
}