//
// Created by Raymond on 3/29/2026.
//

#ifndef IKUYO_BYTESTREAM_H
#define IKUYO_BYTESTREAM_H

#include <ikuyo.h>

struct Bytestream {
    uint8_t* data;
    uint32_t offset;
};

struct Bytestream bytestreamInit(void* start);
void bytestreamAlign(struct Bytestream*, int alignment);

uint64_t bytestreamReadLongLong(struct Bytestream*, bool bigEndian);
uint32_t bytestreamReadLong(struct Bytestream*, bool bigEndian);
uint16_t bytestreamReadShort(struct Bytestream*, bool bigEndian);
uint8_t bytestreamReadByte(struct Bytestream*);

void* bytestreamReadPointer(struct Bytestream*);
char* bytestreamReadString(struct Bytestream*);

#endif //IKUYO_BYTESTREAM_H