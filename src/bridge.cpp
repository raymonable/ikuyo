//
// Created by Raymond on 3/29/2026.
//

#include <bridge.h>

// Bridge: fpng

#include <fpng.h>
void ikuyo_fpng_encode_image_to_file(const char* pFilename, const void* pImage, uint32_t w, uint32_t h, uint32_t num_chans, uint32_t flags) {
    fpng::fpng_encode_image_to_file(
        pFilename, pImage,
        w, h, num_chans, flags
    );
};