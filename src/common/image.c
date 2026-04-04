//
// Created by Raymond on 4/1/26.
//

#include <common/image.h>

#include <webp/mux.h>
#include <webp/encode.h>
#include <avif/avif.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STBIR_DEFAULT_FILTER_UPSAMPLE     STBIR_FILTER_POINT_SAMPLE
#define STBIR_DEFAULT_FILTER_DOWNSAMPLE   STBIR_FILTER_POINT_SAMPLE
#include <stb_image_resize2.h>

struct TextureInformation imageResize(struct TextureInformation information, int w, int h) {
    struct TextureInformation resizedInformation = {0};
    resizedInformation.buffer = malloc(w * h * 4);
    resizedInformation.mustFreeBuffer = true;
    resizedInformation.width = w;
    resizedInformation.height = h;
    resizedInformation.format = RGBA;

    stbir_resize_uint8_linear(
        information.buffer,
        information.width, information.height, information.width * 4,
        resizedInformation.buffer,
        w, h, w * 4,
        STBIR_RGBA
    );

    if (information.mustFreeBuffer)
        free(information.buffer);

    return resizedInformation;
}

struct ImageBuffer imageBufferInit(enum ImageContainer type, uint8_t* data, size_t size) {
    struct ImageBuffer imageBuffer;
    imageBuffer.type = type;
    imageBuffer.size = size;
    imageBuffer.buffer = malloc(size);
    memcpy(imageBuffer.buffer, data, imageBuffer.size);
    return imageBuffer;
};
void imageBufferFree(struct ImageBuffer imageBuffer) {
    if (imageBuffer.buffer != NULL)
        free(imageBuffer.buffer);
};

struct ImageBuffer webpGenerate(struct TextureInformation information, uint8_t* rgba) {
    WebPPicture picture = {0};
    WebPConfig config = {0};

    // TODO: add configurable settings for lossless and resize modes
    if (!WebPConfigPreset(&config, WEBP_PRESET_ICON, 75)) goto WebpError;
    if (!WebPValidateConfig(&config)) goto WebpError;

    picture.width = information.width;
    picture.height = information.height;

    picture.use_argb = true;
    WebPPictureAlloc(&picture);

    for (size_t y = 0; information.height > y; y++)
        for (size_t x = 0; information.width > x; x++) {
            size_t offset = y * information.width + x;
            const uint8_t* src = rgba + offset * 4;
            picture.argb[offset] = (src[3] << 24) | (src[0] << 16) | (src[1] << 8) | src[2];
        }

    WebPMemoryWriter memoryWriter = {0};
    picture.writer = WebPMemoryWrite;
    picture.custom_ptr = (void*)&memoryWriter;

    if (!WebPEncode(&config, &picture)) goto WebpError;

    struct ImageBuffer imageBuffer = imageBufferInit(WebP, memoryWriter.mem, memoryWriter.size);

    WebPMemoryWriterClear(&memoryWriter);
    WebPPictureFree(&picture);

    return imageBuffer;
WebpError:
    return (struct ImageBuffer){0};
};

struct ImageBuffer avifGenerate(struct TextureInformation information, uint8_t* rgba) {
    avifRWData avifOutput = AVIF_DATA_EMPTY;
    avifRGBImage rgb = {0};

    avifImage* image = avifImageCreate(
        information.width, information.height,
        8, AVIF_PIXEL_FORMAT_YUV420
    );
    if (!image)
        goto AvifError;

    avifRGBImageSetDefaults(&rgb, image);
    if (avifRGBImageAllocatePixels(&rgb) != AVIF_RESULT_OK) goto AvifError;

    memcpy(rgb.pixels, rgba, rgb.rowBytes * image->height);
    if (avifImageRGBToYUV(image, &rgb) != AVIF_RESULT_OK)goto AvifError;

    avifEncoder* encoder = avifEncoderCreate();
    if (!encoder) goto AvifError;

    encoder->quality = 50;
    encoder->qualityAlpha = AVIF_QUALITY_BEST;

    if (avifEncoderAddImage(encoder, image, 1, AVIF_ADD_IMAGE_FLAG_SINGLE) != AVIF_RESULT_OK) goto AvifError;
    if (avifEncoderFinish(encoder, &avifOutput) != AVIF_RESULT_OK) goto AvifError;

    struct ImageBuffer imageBuffer = imageBufferInit(AVIF, avifOutput.data, avifOutput.size);

    avifImageDestroy(image);
    avifEncoderDestroy(encoder);
    avifRWDataFree(&avifOutput);
    avifRGBImageFreePixels(&rgb);

    return imageBuffer;
AvifError:
    return (struct ImageBuffer){0};
}

struct ImageBuffer imageGenerate(enum ImageContainer type, struct TextureInformation information, uint8_t* rgba) {
    switch (type) {
        case PNG: return pngGenerate(information, rgba);
        case WebP: return webpGenerate(information, rgba);
        case AVIF: return avifGenerate(information, rgba);
        default: break;
    }
    return (struct ImageBuffer){0};
};