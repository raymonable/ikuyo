//
// Created by Raymond on 4/21/26.
//

#include <format/unityfs/cab.h>
#include <common/bytestream.h>

struct UnityAssetCollection unityAssetCollectionLoad(uint8_t* buffer, size_t size) {
    struct UnityAssetCollection assetCollection = {0};
    struct Bytestream bytestream = bytestreamInit(buffer);

    bytestream.offset += 8;
    uint32_t version = bytestreamReadLong(&bytestream, true);
    if (version >= 9) goto UnityAssetCollectionLoadFail; // NOTE: only Unity 5.0.0 and above are supported

    return assetCollection;
UnityAssetCollectionLoadFail:
    return assetCollection;
}