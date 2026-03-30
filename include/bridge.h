//
// Created by Raymond on 3/29/2026.
//

#ifndef IKUYO_BRIDGE_H
#define IKUYO_BRIDGE_H

#include <ikuyo.h>

#ifdef __cplusplus
extern "C" {
#endif

    void ikuyo_fpng_encode_image_to_file(const char* pFilename, const void* pImage, uint32_t w, uint32_t h, uint32_t num_chans, uint32_t flags);

#ifdef __cplusplus
}
#endif

#endif //IKUYO_BRIDGE_H