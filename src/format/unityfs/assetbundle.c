//
// Created by Raymond on 4/1/26.
//

#include <ikuyo.h>
#include <format/unityfs/assetbundle.h>
#include <common/bytestream.h>

#include <lz4.h> // NOTE: lz4hc appears to be included in here for decompression

bool assetBundleDecompress(enum AssetBundleCompression type, uint8_t* input, size_t inputSize, uint8_t* output, size_t outputSize) {
    switch (type) {
        case AssetBundleNoCompression:
            memcpy(output, input, inputSize); break;
         // TODO: LZMA
        case AssetBundleLZ4HC:
        case AssetBundleLZ4:
            return LZ4_decompress_safe(
                (char*)input, (char*)output,
                inputSize, outputSize
            ) > 0;
        default: break;
    }
    return true;
}

struct AssetBundle assetBundleParse(uint8_t* buffer) {
    struct AssetBundle assetBundle = {0};

    if (strcmp((const char*)buffer, "UnityFS\0") != 0)
        goto AssetBundleLoadFailure;

    struct Bytestream bytestream = bytestreamInit(buffer + 8);

    // BEGIN: read header
    assetBundle.version = bytestreamReadLong(&bytestream, true);
    assetBundle.majorVersion = bytestreamReadString(&bytestream);
    assetBundle.revisionVersion = bytestreamReadString(&bytestream);

    uint64_t size = bytestreamReadLongLong(&bytestream, true);
    uint32_t compressedBlockInfoSize = bytestreamReadLong(&bytestream, true);
    uint32_t decompressedBlockInfoSize = bytestreamReadLong(&bytestream, true);
    uint32_t flags = bytestreamReadLong(&bytestream, true);

    // BEGIN: read block info
    uint8_t* compressedBlockInfo = buffer + size - compressedBlockInfoSize;
    if (!(flags & 0x80)) {
        // NOTE: block info is next up
        compressedBlockInfo = bytestreamReadPointer(&bytestream);
        bytestream.offset += compressedBlockInfoSize;
    }

    uint8_t* decompressedBlockInfo = malloc(decompressedBlockInfoSize);
    memset(decompressedBlockInfo, 0, decompressedBlockInfoSize);
    if (!assetBundleDecompress(
        flags & 0x3F,
        compressedBlockInfo, compressedBlockInfoSize,
        decompressedBlockInfo, decompressedBlockInfoSize
    )) goto AssetBundleLoadFailure;

    struct Bytestream blockInfoBytestream = bytestreamInit(decompressedBlockInfo);
    blockInfoBytestream.offset += 16;

    // BEGIN: decompress block contents
    uint32_t blockInfoCount = bytestreamReadLong(&blockInfoBytestream, true);
    for (size_t index = 0; index < blockInfoCount; index++) {
        uint32_t blockDecompressedSize = bytestreamReadLong(&blockInfoBytestream, true);
        uint32_t blockCompressedSize = bytestreamReadLong(&blockInfoBytestream, true);
        uint32_t blockFlags = bytestreamReadShort(&blockInfoBytestream, true);

        assetBundle.decompressedDataSize += blockDecompressedSize;
        uint8_t* ptr = realloc(assetBundle.decompressedData, assetBundle.decompressedDataSize);
        if (ptr != NULL) {
            assetBundle.decompressedData = ptr;
        } else
            goto AssetBundleLoadFailure;

        if (!assetBundleDecompress(
            blockFlags & 0x3F,
            bytestreamReadPointer(&bytestream), blockCompressedSize,
            assetBundle.decompressedData + assetBundle.decompressedDataSize - blockDecompressedSize,
            blockDecompressedSize
        )) goto AssetBundleLoadFailure;
        bytestream.offset += blockCompressedSize;
    }

    uint32_t blockNodeCount = bytestreamReadLong(&blockInfoBytestream, true);
    for (size_t index = 0; index < blockNodeCount; index++) {
        uint64_t nodeOffset = bytestreamReadLongLong(&blockInfoBytestream, true);
        uint64_t nodeSize = bytestreamReadLongLong(&blockInfoBytestream, true);
        uint32_t nodeFlags = bytestreamReadLong(&blockInfoBytestream, true);
        const char* nodePath = bytestreamReadString(&blockInfoBytestream);
    }

    return assetBundle;
AssetBundleLoadFailure:
    return (struct AssetBundle){0};
}