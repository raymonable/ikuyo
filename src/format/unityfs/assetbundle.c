//
// Created by Raymond on 4/1/26.
//

#include <ikuyo.h>
#include <format/unityfs/assetbundle.h>
#include <common/bytestream.h>

#include <lz4.h> // NOTE: lz4hc appears to be included in here for decompression

bool unityAssetBundleDecompress(enum UnityAssetBundleCompression type, uint8_t* input, size_t inputSize, uint8_t* output, size_t outputSize) {
    switch (type) {
        case UnityAssetBundleNoCompression:
            memcpy(output, input, inputSize); break;
         // TODO: LZMA
        case UnityAssetBundleLZ4HC:
        case UnityAssetBundleLZ4:
            return LZ4_decompress_safe(
                (char*)input, (char*)output,
                inputSize, outputSize
            ) > 0;
        default: break;
    }
    return true;
}

struct UnityAssetBundle unityAssetBundleLoad(uint8_t* buffer, size_t size) {
    struct UnityAssetBundle assetBundle = {0};
    uint8_t* decompressedBlockInfo = NULL;

    if (strcmp((const char*)buffer, "UnityFS\0") != 0)
        goto AssetBundleLoadFailure;

    struct Bytestream bytestream = bytestreamInit(buffer + 8);

    // BEGIN: read header
    assetBundle.version = bytestreamReadLong(&bytestream, true);
    assetBundle.majorVersion = bytestreamReadString(&bytestream);
    assetBundle.revisionVersion = bytestreamReadString(&bytestream);

    bytestreamReadLongLong(&bytestream, true);
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

    decompressedBlockInfo = malloc(decompressedBlockInfoSize);
    memset(decompressedBlockInfo, 0, decompressedBlockInfoSize);
    if (!unityAssetBundleDecompress(
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

        if (!unityAssetBundleDecompress(
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

        // NOTE: is this even correct
        assetBundle.entriesCount++;
        if (assetBundle.entries != NULL) {
            struct UnityAssetBundleEntry* ptr = realloc(assetBundle.entries, assetBundle.entriesCount * sizeof(struct UnityAssetBundleEntry));
            if (!ptr) goto AssetBundleLoadFailure;
            assetBundle.entries = ptr;
        } else
            assetBundle.entries = malloc(assetBundle.entriesCount * sizeof(struct UnityAssetBundleEntry));
        struct UnityAssetBundleEntry* entry = assetBundle.entries + assetBundle.entriesCount - 1;
        memset(entry, 0, sizeof(struct UnityAssetBundleEntry));
        entry->name = nodePath;
        entry->data = assetBundle.decompressedData + nodeOffset;
        entry->size = nodeSize;

        switch (nodeFlags) {
            case 0: assetBundle.assetArchiveEntry = entry; break;
            case 4: assetBundle.cabArchiveEntry = entry; break;
            default: break;
        }
    }

    // NOTE: most AssetBundles keep the assets in the cab but not always
    if (!assetBundle.assetArchiveEntry && assetBundle.cabArchiveEntry != NULL)
        assetBundle.assetArchiveEntry = assetBundle.cabArchiveEntry;

    goto AssetBundleLoadSuccess;
AssetBundleLoadFailure:
    unityAssetBundleFree(&assetBundle);
    memset(&assetBundle, 0, sizeof(struct UnityAssetBundle));
AssetBundleLoadSuccess:
    if (decompressedBlockInfo != NULL) free(decompressedBlockInfo);
    return assetBundle;
}

void unityAssetBundleFree(struct UnityAssetBundle* assetBundle) {
    if (assetBundle->decompressedData != NULL)
        free(assetBundle->decompressedData);
    if (assetBundle->entries != NULL)
        free(assetBundle->entries);
    assetBundle->decompressedData = NULL;
    assetBundle->entries = NULL;
    assetBundle->entriesCount = 0;
}

struct TextureArray unityAssetBundleParse(uint8_t* buffer, size_t size) {
    struct UnityAssetBundle assetBundle = unityAssetBundleLoad(buffer, size);

    return (struct TextureArray){0};
}

void unityAssetBundleRegister(struct TextureLoaderImplementations* implementations) {
    struct TextureLoaderImplementation implementation = {0};
    implementation.name = "ab";
    implementation.description = "Unity AssetBundle";
    implementation.container = UnityAssetBundle;
    implementation.load = &unityAssetBundleParse;
    implementation.detect = NULL;
    textureLoadImplementationAdd(implementations, implementation);
}