//
// Created by Raymond on 4/1/2026.
//

#include <format/ue4/uexp.h>
#include <common/bytestream.h>

struct UnrealEngineTexture uexpReadBuffer(void* buffer, size_t size) {
    // TODO: make this more robust, it currently relies on very specific values

    struct TextureInformation information = {0};
    struct Bytestream bytestream = bytestreamInit(buffer);

    while (strncmp(bytestreamReadPointer(&bytestream), "PF_", 3) != 0 && bytestream.offset < size) // TODO: find actual upper bound
        bytestream.offset += 1;
    if (bytestream.offset >= size)
        return (struct UnrealEngineTexture){0};

    bytestream.offset -= 0x11;
    information.width = bytestreamReadLong(&bytestream, true);
    information.height = bytestreamReadLong(&bytestream, true);
    bytestream.offset += 0x9;

    const char* formatString = bytestreamReadString(&bytestream);

    if (strcmp(formatString, "PF_R8G8B8A8") == 0) information.format = RGBA;
    if (strcmp(formatString, "PF_B8G8R8A8") == 0) information.format = BGRA;
    if (strcmp(formatString, "PF_DXT1") == 0) information.format = DXT1;
    if (strcmp(formatString, "PF_DXT3") == 0) information.format = DXT3;
    if (strcmp(formatString, "PF_DXT5") == 0) information.format = DXT5;

    struct UnrealEngineTexture texture = {0};
    texture.information = information;
    texture.buffer = (uint8_t*)bytestreamReadPointer(&bytestream) + 0x20;
    return texture;
}