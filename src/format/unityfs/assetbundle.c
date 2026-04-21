//
// Created by Raymond on 4/1/26.
//

#include <ikuyo.h>
#include <format/unityfs/assetbundle.h>
#include <common/bytestream.h>

#include <lz4.h> // NOTE: lz4hc appears to be included in here for decompression
#include <LzmaDec.h>

#include "format/unityfs/cab.h"

static void *SzAlloc(ISzAllocPtr ptr, size_t size) { return malloc(size); }
static void SzFree(ISzAllocPtr ptr, void *address) { free(address); }
static ISzAlloc g_Alloc = { SzAlloc, SzFree };

bool unityAssetBundleDecompress(enum UnityAssetBundleCompression type, uint8_t* input, size_t inputSize, uint8_t* output, size_t outputSize) {
    switch (type) {
        case UnityAssetBundleNoCompression:
            memcpy(output, input, inputSize); break;
        case UnityAssetBundleLZMA: {
            CLzmaDec state;
            LzmaDec_Construct(&state);

            SRes alloc_res = LzmaDec_Allocate(&state, input, LZMA_PROPS_SIZE, &g_Alloc);
            if (alloc_res != SZ_OK) return false;

            LzmaDec_Init(&state);

            SizeT dest_len = outputSize;
            SizeT src_len = inputSize - 5;

            ELzmaStatus status = LZMA_STATUS_NOT_SPECIFIED;
            SRes response = LzmaDec_DecodeToBuf(
                &state,
                output, &dest_len,
                input + 5, &src_len,
                LZMA_FINISH_END, &status
            );

            LzmaDec_Free(&state, &g_Alloc);
            if (response != SZ_OK) return false;
            break;
        }
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

    if (strcmp((const char*)buffer, "UnityFS\0") != 0)
        goto AssetBundleLoadFailure;

    struct Bytestream bytestream = bytestreamInit(buffer + 8);

    // BEGIN: read header
    assetBundle.version = bytestreamReadLong(&bytestream, true);
    assetBundle.majorVersion = bytestreamReadString(&bytestream);
    assetBundle.revisionVersion = bytestreamReadString(&bytestream);

    bytestreamReadLongLong(&bytestream, true);
    uint32_t compressedBlockInfoSize = bytestreamReadLong(&bytestream, true);
    assetBundle.decompressedBlockInfoSize = bytestreamReadLong(&bytestream, true);
    uint32_t flags = bytestreamReadLong(&bytestream, true);

    // BEGIN: read block info
    uint8_t* compressedBlockInfo = buffer + size - compressedBlockInfoSize;
    if (!(flags & 0x80)) {
        // NOTE: block info is next up
        compressedBlockInfo = bytestreamReadPointer(&bytestream);
        bytestream.offset += compressedBlockInfoSize;
    }

    assetBundle.decompressedBlockInfo = malloc(assetBundle.decompressedBlockInfoSize);
    memset(assetBundle.decompressedBlockInfo, 0, assetBundle.decompressedBlockInfoSize);
    if (!unityAssetBundleDecompress(
        flags & 0x3F,
        compressedBlockInfo, compressedBlockInfoSize,
        assetBundle.decompressedBlockInfo, assetBundle.decompressedBlockInfoSize
    )) goto AssetBundleLoadFailure;

    struct Bytestream blockInfoBytestream = bytestreamInit(assetBundle.decompressedBlockInfo);
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

    // NOTE: some AssetBundles keep the assets in the cab but not always
    if (!assetBundle.assetArchiveEntry && assetBundle.cabArchiveEntry != NULL)
        assetBundle.assetArchiveEntry = assetBundle.cabArchiveEntry;

    goto AssetBundleLoadSuccess;
AssetBundleLoadFailure:
    unityAssetBundleFree(&assetBundle);
    memset(&assetBundle, 0, sizeof(struct UnityAssetBundle));
AssetBundleLoadSuccess:
    return assetBundle;
}

void unityAssetBundleFree(struct UnityAssetBundle* assetBundle) {
    if (assetBundle->decompressedData != NULL)
        free(assetBundle->decompressedData);

    if (assetBundle->decompressedBlockInfo != NULL)
        free(assetBundle->decompressedBlockInfo);

    if (assetBundle->entries != NULL)
        free(assetBundle->entries);

    assetBundle->decompressedData = NULL;
    assetBundle->entries = NULL;
    assetBundle->entriesCount = 0;
}

struct TextureArray unityAssetBundleParse(uint8_t* buffer, size_t size) {
    struct UnityAssetBundle assetBundle = unityAssetBundleLoad(buffer, size);
    struct UnityAssetCollection assetCollection = unityAssetCollectionLoad(assetBundle.cabArchiveEntry->data, assetBundle.cabArchiveEntry->size);
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