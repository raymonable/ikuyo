//
// Created by Raymond on 4/1/26.
//

#ifndef IKUYO_UNITY_H
#define IKUYO_UNITY_H

#include <ikuyo.h>
#include <common/texture.h>

enum UnityAssetBundleCompression {
    UnityAssetBundleNoCompression,
    UnityAssetBundleLZMA,
    UnityAssetBundleLZ4,
    UnityAssetBundleLZ4HC
};
struct UnityAssetBundleEntry {
    const char* name;
    uint8_t* data;
    size_t size;
};
struct UnityAssetBundle {
    uint32_t version;
    char* majorVersion;
    char* revisionVersion;

    uint8_t* decompressedData;
    size_t decompressedDataSize;
    uint8_t* decompressedBlockInfo;
    size_t decompressedBlockInfoSize;

    struct UnityAssetBundleEntry* entries;
    size_t entriesCount;

    struct UnityAssetBundleEntry* cabArchiveEntry;
    struct UnityAssetBundleEntry* assetArchiveEntry; // Often kept the same as CAB.
};

void unityAssetBundleFree(struct UnityAssetBundle*);

struct UnityAssetBundle unityAssetBundleLoad(uint8_t* buffer, size_t size);
void unityAssetBundleRegister(struct TextureLoaderImplementations* implementations);

#endif //IKUYO_UNITY_H