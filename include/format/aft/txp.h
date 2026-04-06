//
// Created by Raymond on 4/4/2026.
//

#ifndef IKUYO_TXP_H
#define IKUYO_TXP_H

#include <common/texture.h>
#define TXP_MAGIC 0x54585000

enum TxpMagicType {
    TxpMagicMipmap = 2,
    TxpMagicTextureAtlas = 3,
    TxpMagicTexture = 4,
};

struct TextureArray txpReadBuffer(uint8_t* buffer, size_t size);

#endif //IKUYO_TXP_H