//
// Created by Raymond on 4/1/2026.
//

#include <format/ue4/uexp.h>
#include <common/bytestream.h>

struct UnrealEngineTexture uexpReadBuffer(void* buffer) {
    // TODO: make this more robust, it currently relies on very specific coincidental values

    struct TextureInformation information = {0};
    struct Bytestream bytestream = bytestreamInit(buffer);

    bytestream.offset = 0x30;

    information.width = bytestreamReadLong(&bytestream, true);
    information.height = bytestreamReadLong(&bytestream, true);

    struct UnrealEngineTexture texture = {0};
    texture.information = information;

    // TODO: determine what the content between actually is instead of praying

    while (strcmp(bytestreamReadPointer(&bytestream), "PF_") != 0 && bytestream.offset < 0x256) // TODO: find actual upper bound
        bytestream.offset += 1;
    if (bytestream.offset >= 0x256)
        return (struct UnrealEngineTexture){0};

    const char* formatString = bytestreamReadString(&bytestream);

    if (strcmp(formatString, "PF_R8G8B8A8") == 0) information.format = RGBA;
    if (strcmp(formatString, "PF_B8G8R8A8") == 0) information.format = BGRA;
    if (strcmp(formatString, "PF_DXT1") == 0) information.format = DXT1;
    if (strcmp(formatString, "PF_DXT3") == 0) information.format = DXT3;
    if (strcmp(formatString, "PF_DXT5") == 0) information.format = DXT5;

    texture.buffer = (uint8_t*)bytestreamReadPointer(&bytestream) + 0x20;
    return texture;
}