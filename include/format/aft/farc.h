//
// Created by Raymond on 4/4/2026.
//

#ifndef IKUYO_FARC_H
#define IKUYO_FARC_H

#include <ikuyo.h>

#define FARC_MAGIC 0x46417243

// NOTE: remember to free() the returned buffer!
uint8_t* farcReadBuffer(uint8_t* buffer);

#endif //IKUYO_FARC_H