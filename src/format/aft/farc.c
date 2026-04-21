//
// Created by Raymond on 4/4/2026.
//

#include <format/aft/farc.h>
#include <format/aft/txp.h>

#include <common/texture.h>
#include <common/bytestream.h>

#include <libdeflate.h>

uint8_t* farcAccess(uint8_t* buffer, size_t* size) {
    struct Bytestream bytestream = bytestreamInit(buffer);

    if (bytestreamReadLong(&bytestream, true) != FARC_MAGIC) goto FarcLoadFailure;

    uint32_t version = bytestreamReadLong(&bytestream, true);
    uint32_t entries = bytestreamReadLong(&bytestream, true);

    // NOTE: i cannot find any FArCs that use more than one entry, so only one will be handled
    if (entries != 1) goto FarcLoadFailure;

    char* entryName = bytestreamReadString(&bytestream);
    uint32_t entryType = bytestreamReadLong(&bytestream, true);
    uint32_t entryCompressedSize = bytestreamReadLong(&bytestream, true);
    uint32_t entryDecompressedSize = bytestreamReadLong(&bytestream, true);

    uint8_t* decompressedBuffer = malloc(entryDecompressedSize);

    // NOTE: this specific FArC format (compressed, note the lowercase r in the magic) is compressed and not encrypted
    struct libdeflate_decompressor* decompressor = libdeflate_alloc_decompressor();

    size_t decompressedBytes = 0;
    enum libdeflate_result result = libdeflate_gzip_decompress(
        decompressor,
        bytestreamReadPointer(&bytestream), entryCompressedSize,
        decompressedBuffer, entryDecompressedSize, &decompressedBytes
    );
    libdeflate_free_decompressor(decompressor);

    if (result != LIBDEFLATE_SUCCESS) goto FarcLoadFailure;
    if (size != NULL) *size = decompressedBytes;

    return decompressedBuffer;
FarcLoadFailure: return NULL;
}

bool farcDetect(uint8_t* buffer, size_t size) {
    struct Bytestream bytestream = bytestreamInit(buffer);
    return bytestreamReadLong(&bytestream, true) == FARC_MAGIC;
}

struct TextureArray farcLoad(uint8_t* buffer, size_t size) {
    struct TextureArray array = {0};
    size_t farcSize = 0;

    uint8_t* farc = farcAccess(buffer, &farcSize);
    if (!farc) goto FarcLoadFailure;
    array = txpLoad(farc, farcSize);

FarcLoadFailure:
    free(farc);
    return array;
}

void farcRegister(struct TextureLoaderImplementations* implementations) {
    struct TextureLoaderImplementation implementation = {0};
    implementation.name = "farc";
    implementation.description = "Project DIVA Arcade Future Tone Package Format";
    implementation.container = PDAFT_FArC;
    implementation.load = &farcLoad;
    implementation.detect = &farcDetect;
    textureLoadImplementationAdd(implementations, implementation);
}