//
// Created by Raymond on 4/1/2026.
//

#ifndef IKUYO_UEXP_H
#define IKUYO_UEXP_H

#include <ikuyo.h>
#include <common/texture.h>

struct UnrealEngineTexture {
    struct TextureInformation information;
    void* buffer;
};

struct UnrealEngineTexture uexpReadBuffer(void* buffer);

#endif //IKUYO_UEXP_H