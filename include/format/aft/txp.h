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

void txpRegister(struct TextureLoaderImplementations* implementations);
struct TextureArray txpLoad(uint8_t *, size_t); // NOTE: exported for farc package

#endif //IKUYO_TXP_H