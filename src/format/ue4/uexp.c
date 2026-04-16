//
// Created by Raymond on 4/1/2026.
//

#include <format/ue4/uexp.h>
#include <common/bytestream.h>

struct TextureArray uexpLoad(uint8_t* buffer, size_t size) {
    // TODO: make this more robust, it currently relies on very specific values

    struct TextureInformation information = {0};
    struct Bytestream bytestream = bytestreamInit(buffer);

    bytestream.offset = 0x10;
    uint32_t magic = bytestreamReadLong(&bytestream, true);
    if (magic != 0x8000000)
        goto Ue4LoadFailure;

    // NOTE: hacky hacky hacky hacky hacky hacky
    while (strncmp(bytestreamReadPointer(&bytestream), "PF_", 3) != 0 && bytestream.offset < size)
        bytestream.offset += 1;
    if (bytestream.offset >= size)
        goto Ue4LoadFailure;

    bytestream.offset -= 0x11;
    information.width = bytestreamReadLong(&bytestream, true);
    information.height = bytestreamReadLong(&bytestream, true);
    bytestream.offset += 0x9;

    const char* formatString = bytestreamReadString(&bytestream);

    // NOTE: hacky hacky hacky hacky hacky hacky
    if (strcmp(formatString, "PF_R8G8B8A8") == 0) information.format = RGBA;
    if (strcmp(formatString, "PF_B8G8R8A8") == 0) information.format = BGRA;
    if (strcmp(formatString, "PF_DXT1") == 0) information.format = DXT1;
    if (strcmp(formatString, "PF_DXT3") == 0) information.format = DXT3;
    if (strcmp(formatString, "PF_DXT5") == 0) information.format = DXT5;

    information.buffer = (uint8_t*)bytestreamReadPointer(&bytestream) + 0x20;

    if (!textureDecode(&information)) goto Ue4LoadFailure;

    struct TextureArray array = {0};
    textureArrayAdd(&array, &information);

    return array;

Ue4LoadFailure:
    return (struct TextureArray){0};
}

bool uexpDetect(uint8_t* buffer, size_t size) {
    struct Bytestream bytestream = bytestreamInit(buffer);
    bytestream.offset = 0x10;
    return bytestreamReadLong(&bytestream, true) == 0x8000000;
}

void uexpRegister(struct TextureLoaderImplementations* implementations) {
    struct TextureLoaderImplementation implementation = {0};
    implementation.name = "uexp";
    implementation.description = "Unreal Engine 4 Texture Resource (4.19.2)";
    implementation.container = UE4;
    implementation.load = &uexpLoad;
    implementation.detect = &uexpDetect;
    textureLoadImplementationAdd(implementations, implementation);
}