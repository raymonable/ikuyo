//
// Created by Raymond on 3/29/2026.
//

#include <common/bytestream.h>

struct Bytestream bytestreamInit(void* start) {
    struct Bytestream bytestream = {0};
    bytestream.data = start;
    return bytestream;
}

void bytestreamAlign(struct Bytestream* bytestream, int alignment) {
    const long mod = bytestream->offset % alignment;
    if (mod != 0)
        bytestream->offset = bytestream->offset + (alignment - mod);
}

uint64_t bytestreamReadLongLong(struct Bytestream* bytestream, bool bigEndian) {
    // TODO: determine host endianness
    uint64_t value = *(uint64_t*)(bytestream->offset + bytestream->data);
    bytestream->offset = bytestream->offset + sizeof(uint64_t);

    if (bigEndian) {
        uint32_t result = 0;
        for (int i = 0; i < sizeof(uint64_t); i++)
            result |= ((uint8_t*)(&value))[(sizeof(uint64_t) - 1) - i] << (8 * i);
        return result;
    }

    return value;
};
uint32_t bytestreamReadLong(struct Bytestream* bytestream, bool bigEndian) {
    // TODO: determine host endianness
    uint32_t value = *(uint32_t*)(bytestream->offset + bytestream->data);
    bytestream->offset = bytestream->offset + sizeof(uint32_t);

    if (bigEndian) {
        uint32_t result = 0;
        for (int i = 0; i < sizeof(uint32_t); i++)
            result |= ((uint8_t*)(&value))[(sizeof(uint32_t) - 1) - i] << (8 * i);
        return result;
    }

    return value;
};
uint16_t bytestreamReadShort(struct Bytestream* bytestream, bool bigEndian) {
    // TODO: determine host endianness
    uint16_t value = *(uint16_t*)(bytestream->offset + bytestream->data);
    bytestream->offset = bytestream->offset + sizeof(uint16_t);

    if (bigEndian) {
        uint16_t result = 0;
        for (int i = 0; i < sizeof(uint16_t); i++)
            result |= ((uint8_t*)(&value))[(sizeof(uint16_t) - 1) - i] << (8 * i);
        return result;
    }

    return value;
};
uint8_t bytestreamReadByte(struct Bytestream* bytestream) {
    uint8_t value = *(bytestream->offset + bytestream->data);
    bytestream->offset = bytestream->offset + sizeof(uint8_t);
    return value;
};
char* bytestreamReadString(struct Bytestream* bytestream) {
    char* start = (char*)bytestream->data + bytestream->offset;
    char* end = start;

    while (*end != 0)
        end++;
    bytestream->offset += (end - start) + 1;

    return start;
};
void* bytestreamReadPointer(struct Bytestream* bytestream) {
    return bytestream->data + bytestream->offset;
};