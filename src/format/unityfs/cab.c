//
// Created by Raymond on 4/21/26.
//

#include <format/unityfs/cab.h>
#include <common/bytestream.h>

struct UnityAssetCollection unityAssetCollectionLoad(uint8_t* buffer, size_t size) {
    struct UnityAssetCollection assetCollection = {0};
    struct Bytestream bytestream = bytestreamInit(buffer);

    // BEGIN: read header
    bytestream.offset += 8;
    uint32_t version = bytestreamReadLong(&bytestream, true);
    if (version < 14) goto UnityAssetCollectionLoadFail; // NOTE: only Unity 5.0.0 (non-alpha) and above are supported
    uint32_t dataOffset = bytestreamReadLong(&bytestream, true);

    uint8_t endianness = *(uint8_t*)bytestreamReadPointer(&bytestream);
    bytestreamReadLong(&bytestream, endianness);

    if (version >= 22) bytestream.offset += 28;

    // BEGIN: read metadata

    const char* unityVersion = bytestreamReadString(&bytestream);
    int32_t targetPlatform = (int32_t)bytestreamReadLong(&bytestream, endianness);
    bool enableTypeTree = bytestreamReadByte(&bytestream);

    int typeCount = (int32_t)bytestreamReadLong(&bytestream, endianness);
    for (size_t index = 0; typeCount > index; index++) {
        int32_t classId = (int32_t)bytestreamReadLong(&bytestream, endianness);
        if (version >= 16)
            bytestreamReadByte(&bytestream);
        if (version >= 17)
            bytestreamReadShort(&bytestream, endianness);

        // NOTE: minimum is version 14, so we can skip a lot of the implementation
        if (version < 16 && (classId < 0 || classId == 114))
            bytestream.offset += 16;
        bytestream.offset += 16;
        if (enableTypeTree) {
            int32_t nodeCount = (int32_t)bytestreamReadLong(&bytestream, endianness);
            int32_t stringSize = (int32_t)bytestreamReadLong(&bytestream, endianness);
            bytestream.offset += stringSize + (nodeCount * (version >= 19 ? 32 : 24));
            if (version >= 21)
                bytestreamReadLong(&bytestream, endianness);
        }
    }

    int32_t objectCount = (int32_t)bytestreamReadLong(&bytestream, endianness);
    for (size_t index = 0; objectCount > index; index++) {
        bytestreamAlign(&bytestream, 4);
        int64_t pathId = (int64_t)bytestreamReadLongLong(&bytestream, endianness);
        int64_t byteStart = (version >= 22) ? (int64_t)bytestreamReadLongLong(&bytestream, endianness)
            : (int64_t)bytestreamReadLong(&bytestream, endianness);
        uint32_t byteSize = bytestreamReadLong(&bytestream, endianness);
        int32_t typeId = (int32_t)bytestreamReadLong(&bytestream, endianness);

        if (version < 16) {
            uint16_t classId = bytestreamReadShort(&bytestream, endianness);
        } // TODO: implement the other end

        if (version < 17) bytestreamReadShort(&bytestream, endianness);
        if (version == 15 || version == 16) bytestreamReadByte(&bytestream);
    }

    return assetCollection;
UnityAssetCollectionLoadFail:
    return assetCollection;
}