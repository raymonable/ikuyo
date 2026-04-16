//
// Created by Raymond on 3/29/2026.
//

#include <ikuyo.h>
#include <common/bytestream.h>
#include <format/dds/dds.h>

bool ddsDetect(uint8_t* buffer, size_t size) {
    struct Bytestream bytestream = bytestreamInit(buffer);
    uint32_t magic = bytestreamReadLong(&bytestream, false);
    return magic == DDS_MAGIC;
}

struct TextureArray ddsLoad(uint8_t* buffer, size_t size) {
    struct TextureInformation information = {0};
    struct Bytestream bytestream = bytestreamInit(buffer);

    uint32_t magic = bytestreamReadLong(&bytestream, false);
    if (magic != DDS_MAGIC)
        goto DdsLoadFailure;

    bytestream.offset = 12;
    information.height = bytestreamReadLong(&bytestream, false);
    information.width = bytestreamReadLong(&bytestream, false);

    bytestream.offset = 80;
    const uint32_t flags = bytestreamReadLong(&bytestream, false);
    const uint32_t format = bytestreamReadLong(&bytestream, false);
    switch (format) {
        case FORMAT_DXT1_MAGIC:
            information.format = DXT1; break;
        case FORMAT_DXT3_MAGIC:
            information.format = DXT3; break;
        case FORMAT_DXT5_MAGIC:
            information.format = DXT5; break;
        default: {
            bytestreamReadLong(&bytestream, false);
            const uint32_t channelR = bytestreamReadLong(&bytestream, false);
            if (flags & 0x40)
                information.format = (flags & 0x1) ?
                    (channelR != 255 ? BGRA : RGBA) : (channelR != 255 ? BGR : RGB);
            break;
        }
    }

    bytestream.offset = 128;
    information.buffer = bytestreamReadPointer(&bytestream);

    if (!textureDecode(&information)) goto DdsLoadFailure;

    struct TextureArray array = {0};
    textureArrayAdd(&array, &information);

    return array;
DdsLoadFailure:
    return (struct TextureArray){0};
}

void ddsRegister(struct TextureLoaderImplementations* implementations) {
    struct TextureLoaderImplementation implementation = {0};
    implementation.name = "dds";
    implementation.description = "DirectDraw Surface";
    implementation.container = DDS;
    implementation.load = &ddsLoad;
    implementation.detect = &ddsDetect;
    textureLoadImplementationAdd(implementations, implementation);
}