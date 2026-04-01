//
// Created by Raymond on 4/1/26.
//

#ifndef IKUYO_UNITY_H
#define IKUYO_UNITY_H

#include <ikuyo.h>

enum AssetBundleCompression {
    AssetBundleNoCompression,
    AssetBundleLZMA,
    AssetBundleLZ4,
    AssetBundleLZ4HC
};
struct AssetBundle {
    uint32_t version;
    char* majorVersion;
    char* revisionVersion;


};

struct AssetBundle assetBundleParse(uint8_t* buffer);

#endif //IKUYO_UNITY_H