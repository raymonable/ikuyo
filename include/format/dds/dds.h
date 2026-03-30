//
// Created by Raymond on 3/29/2026.
//

#ifndef IKUYO_DDS_H
#define IKUYO_DDS_H

#include <ikuyo.h>
#include <common/texture.h>

#define DDS_MAGIC 0x20534444

#define FORMAT_DXT1_MAGIC 0x31545844
#define FORMAT_DXT3_MAGIC 0x33545844
#define FORMAT_DXT5_MAGIC 0x35545844

struct DDS {
    struct TextureInformation information;
    void* buffer;
};

struct DDS ddsReadBuffer(void* buffer);

#endif //IKUYO_DDS_H