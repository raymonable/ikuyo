//
// Created by Raymond on 3/29/2026.
//

#include <ikuyo.h>
#include <common/texture.h>

#include <format/dds/dds.h>
#include <format/unityfs/assetbundle.h>
#include <format/ue4/uexp.h>
#include <format/aft/farc.h>
#include <format/aft/txp.h>

#define BCDEC_IMPLEMENTATION
#include <bcdec.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STBIR_DEFAULT_FILTER_UPSAMPLE     STBIR_FILTER_POINT_SAMPLE
#define STBIR_DEFAULT_FILTER_DOWNSAMPLE   STBIR_FILTER_POINT_SAMPLE
#include <stb_image_resize2.h>

/////////////////////////////////////////////////////////////////////////////

void textureLoadImplementationsInit() {
    ddsRegister();
}

/////////////////////////////////////////////////////////////////////////////

struct TextureLoaderImplementation* implementations = NULL;
size_t implementationCount = 0;

void textureLoadImplementationAdd(struct TextureLoaderImplementation implementation) {
    if (implementations != NULL) {
        struct TextureLoaderImplementation* reallocImplementations = realloc(implementations, (implementationCount + 1) * sizeof(struct TextureLoaderImplementation));
        if (!reallocImplementations) return;
        implementations = reallocImplementations;
    } else
        implementations = malloc((implementationCount + 1) * sizeof(struct TextureLoaderImplementation));
    memcpy(implementations + implementationCount, &implementation, sizeof(struct TextureLoaderImplementation));
    implementationCount++;
}
struct TextureArray textureLoad(enum TextureContainer container, uint8_t* buffer, size_t size) {
    for (size_t index = 0; implementationCount > index; index++)
        if (implementations[index].container == container)
            return implementations[index].load(buffer, size);
    for (size_t index = 0; implementationCount > index; index++)
        if (implementations[index].detect != NULL && implementations[index].detect(buffer, size))
            return implementations[index].load(buffer, size);
    return (struct TextureArray){0};
};
enum TextureContainer textureContainerGetFromString(const char* name) {
    for (size_t index = 0; implementationCount > index; index++)
        if (strcmpi(implementations[index].name, name) == 0)
            return implementations[index].container;
    return UnknownContainer;
}

struct TextureInformation* textureResize(struct TextureInformation* information, int w, int h) {
    if (information->format != RGBA) return information;

    uint8_t* buffer = malloc(w * h * 4);

    stbir_resize_uint8_linear(
        information->buffer, information->width, information->height, information->width * 4,
        buffer, w, h, w * 4,
        STBIR_RGBA
    );

    textureFree(information);
    information->buffer = buffer;
    information->width = w;
    information->height = h;

    return information;
}

struct TextureInformation* textureDecode(struct TextureInformation* information) {
    if (information->format == UnsupportedEncoding) return NULL;
    uint8_t* rgba = malloc(textureGetSize(information));
    memset(rgba, 255, textureGetSize(information));
    uint8_t* src = information->buffer;

    size_t blockSize = information->format >= (int)DXT1 ? 4 : 1;
    bool hasAlpha = textureHasAlpha(information);

    if (information->format == RGBA) {
        memcpy(rgba, src, textureGetSize(information));
    } else for (uint32_t y = 0; information->height > (y + blockSize - 1); y += blockSize)
        for (uint32_t x = 0; information->width > (x + blockSize - 1); x += blockSize) {
            uint8_t* dst = rgba + (y * information->width + x) * 4;
            switch (information->format) {
                // Uncompressed formats
                case RGB:
                    memcpy(dst, src, 3);
                    break;
                case BGR:
                case BGRA:
                    dst[0] = src[2]; dst[1] = src[1]; dst[2] = src[0];
                    if (hasAlpha)
                        dst[3] = src[3];
                    break;
                // Compressed formats
                case DXT1: bcdec_bc1(src, dst, information->width * 4); break;
                case DXT3: bcdec_bc2(src, dst, information->width * 4); break;
                case DXT5: bcdec_bc3(src, dst, information->width * 4); break;
                case ATI1: bcdec_bc4(src, dst, information->width); break;
                case ATI2: {
                    uint8_t tempBlock[32];
                    bcdec_bc5(src, tempBlock, 8);
                    uint8_t* ddst = dst;
                    for (uint32_t ii = 0; ii < 4; ii++) {
                        for (uint32_t jj = 0; jj < 4; jj++)
                            memcpy(ddst + (jj * 4), tempBlock + (jj * 2) + (ii * 8), 2);
                        ddst += information->width * 4;
                    }
                    break;
                }
                case BC7: bcdec_bc7(src, dst, information->width * 4); break;
                default: break;
            }
            src += ((information->format != DXT1 && information->format != ATI1) ? (blockSize * (hasAlpha ? 4 : 3)) : 8);
        }

    if (information->requiresTransformation) {
        uint8_t* ref = malloc(textureGetSize(information));
        memcpy(ref, rgba, textureGetSize(information));
        for (uint32_t y = 0; y < information->height; ++y)
            for (uint32_t x = 0; x < information->width; ++x)
                memcpy(
                    rgba + (y * information->width + x) * 4,
                    ref + ((information->height - 1 - y) * information->width + x) * 4,
                    4
                );
        free(ref);
    }

    textureFree(information);

    information->format = RGBA;
    information->buffer = rgba;
    information->allocated = true;

    return information;
};
bool textureHasAlpha(struct TextureInformation* information) {
    return !(information->format == RGB || information->format == BGR);
}
size_t textureGetSize(struct TextureInformation* information) {
    return information->width * information->height * 4;
};
void textureFree(struct TextureInformation* information) {
    if (information->allocated)
        free(information->buffer);
}

void textureArrayAdd(struct TextureArray* array, struct TextureInformation* information) {
    if (array->data != NULL) {
        struct TextureInformation* allocatedData = realloc(array->data, (array->count + 1) * sizeof(struct TextureInformation));
        if (!allocatedData) return textureArrayFree(array);
        array->data = allocatedData;
    } else
        array->data = (struct TextureInformation*)malloc((array->count + 1) * sizeof(struct TextureInformation));
    memcpy(array->data + array->count, information, sizeof(struct TextureInformation));
    array->count++;
};
void textureArrayFree(struct TextureArray* array) {
    if (array->count > 0)
        for (size_t index = 0; index < array->count; index++)
            textureFree(&array->data[index]);
    if (array->data != NULL)
        free(array->data);
    array->data = NULL;
    array->count = 0;
};