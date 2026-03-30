//
// Created by Raymond on 3/29/2026.
//

#include <ikuyo.h>
#include <common/bytestream.h>
#include <format/dds/dds.h>

struct DDS ddsReadBuffer(void* buffer) {
    struct DDS dds = {0};
    dds.buffer = 0;
    struct Bytestream bytestream = bytestreamInit(buffer);

    uint32_t magic = bytestreamReadLong(&bytestream, false);
    if (magic != DDS_MAGIC)
        return dds;

    bytestream.offset = 12;
    dds.information.height = bytestreamReadLong(&bytestream, false);
    dds.information.width = bytestreamReadLong(&bytestream, false);

    bytestream.offset = 80;
    const uint32_t flags = bytestreamReadLong(&bytestream, false);
    const uint32_t format = bytestreamReadLong(&bytestream, false);
    switch (format) {
        case FORMAT_DXT1_MAGIC:
            dds.information.format = DXT1; break;
        case FORMAT_DXT3_MAGIC:
            dds.information.format = DXT3; break;
        case FORMAT_DXT5_MAGIC:
            dds.information.format = DXT5; break;
        default: {
            bytestreamReadLong(&bytestream, false);
            const uint32_t channelR = bytestreamReadLong(&bytestream, false);
            if (flags & 0x40)
                dds.information.format = (flags & 0x1) ?
                    (channelR != 255 ? BGRA : RGBA) : (channelR != 255 ? BGR : RGB);
            break;
        }
    }

    bytestream.offset = 128;
    dds.buffer = bytestreamReadPointer(&bytestream);

    return dds;
}