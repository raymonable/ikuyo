//
// Created by Raymond on 4/21/26.
//

#ifndef IKUYO_CAB_H
#define IKUYO_CAB_H

#include <ikuyo.h>
#include <common/texture.h>

struct UnityAssetCollectionAsset {
    uint32_t offset;
    size_t size;
};
struct UnityAssetCollection {
    struct UnityAssetCollectionAsset* entries;
    size_t count;
};

struct UnityAssetCollection unityAssetCollectionLoad(uint8_t* buffer, size_t size);

#endif //IKUYO_CAB_H