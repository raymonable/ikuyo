//
// Created by Raymond on 4/4/2026.
//

#include <format/aft/farc.h>
#include <common/bytestream.h>

#include <libdeflate.h>

uint8_t* farcReadBuffer(uint8_t* buffer) {
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

    return decompressedBuffer;
FarcLoadFailure:
    return NULL;
}