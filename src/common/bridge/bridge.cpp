//
// Created by Raymond on 3/29/2026.
//

// Bridge: `fpng`

#include <fpng.h>
extern "C" {
    #include <common/image.h>
}

ImageBuffer pngGenerate(TextureInformation information) {
    std::vector<uint8_t> buffer;
    fpng::fpng_encode_image_to_memory(
        information.buffer, information.width, information.height, 4,
        buffer, 0
    );
    ImageBuffer imageBuffer = imageBufferInit(PNG, buffer.data(), buffer.size());
    buffer.clear();
    return imageBuffer;
};