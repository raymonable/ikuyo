//
// Created by Raymond on 4/17/26.
//

#include <common/texture.h>
#include <common/image.h>

#include <jni.h>

struct TextureLoaderImplementations implementations = {0};

///////////////////// JNI initialization functions ///////////////////////////////

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    if (implementations.count != 0) return JNI_VERSION_21;
    textureLoadImplementationsInit(&implementations);
    return JNI_VERSION_21;
};

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {
    textureLoadImplementationsFree(&implementations);
};

/////////////////////////// Handler functions ////////////////////////////////////

jint jniGetEnum(JNIEnv* env, jobject parent, char* propertyName, char* enumType) {
    jobject formatInputEnum = (*env)->GetObjectField(env, parent,
        (*env)->GetFieldID(env, (*env)->GetObjectClass(env, parent), propertyName, enumType)
    );
    return (*env)->GetIntField(env, formatInputEnum,
        (*env)->GetFieldID(env, (*env)->GetObjectClass(env, formatInputEnum), "value", "I")
    );
}

JNIEXPORT jobject JNICALL Java_ikuyo_Ikuyo_process(JNIEnv* env, jobject this, jobject input, jobject settings) {
    struct TextureArray array = {0};

    if (!(*env)->CallIntMethod(env, input,
        (*env)->GetMethodID(env, (*env)->GetObjectClass(env, input), "isDirect", "()Z")
    )) goto IkuyoProcessFail;

    // BEGIN: read input and settings

    const enum TextureContainer formatInput = jniGetEnum(env, settings, "formatInput", "Likuyo/IkuyoTextureContainer;");
    const enum ImageContainer formatOutput = jniGetEnum(env, settings, "formatOutput", "Likuyo/IkuyoImageContainer;");

    jobject nullableWidth = (*env)->GetObjectField(env, settings, (*env)->GetFieldID(env, (*env)->GetObjectClass(env, settings), "width", "Ljava/lang/Integer;"));
    jobject nullableHeight = (*env)->GetObjectField(env, settings, (*env)->GetFieldID(env, (*env)->GetObjectClass(env, settings), "height", "Ljava/lang/Integer;"));

    jobject preferredImageIndex = (*env)->GetObjectField(env, settings, (*env)->GetFieldID(env, (*env)->GetObjectClass(env, settings), "preferredImageIndex", "Ljava/lang/Integer;"));

    int preferredQuality = (*env)->GetIntField(env, settings,
        (*env)->GetFieldID(env, (*env)->GetObjectClass(env, settings), "preferredQuality", "I")
    );

    void* data = (*env)->GetDirectBufferAddress(env, input);
    size_t size = (*env)->GetDirectBufferCapacity(env, input);

    // BEGIN: process input

    array = textureLoad(&implementations, formatInput, data, size);

    jclass byteBufferClass = (*env)->FindClass(env, "java/nio/ByteBuffer");
    jobjectArray byteBufferArray = (*env)->NewObjectArray(env, preferredImageIndex != NULL ? 1 : array.count, byteBufferClass, NULL);

    for (uint32_t index = 0; array.count > index; index++) {
        if (preferredImageIndex != NULL)
            if ((*env)->CallIntMethod(env, preferredImageIndex,
                (*env)->GetMethodID(env, (*env)->FindClass(env, "java/lang/Integer"), "intValue", "()I")
            ) != index) continue;

        if (nullableWidth != NULL || nullableHeight != NULL) {
            jint w = nullableWidth != NULL ? (*env)->CallIntMethod(env, nullableWidth,
                (*env)->GetMethodID(env, (*env)->FindClass(env, "java/lang/Integer"), "intValue", "()I")) : 0;
            jint h = nullableHeight != NULL ? (*env)->CallIntMethod(env, nullableHeight,
                (*env)->GetMethodID(env, (*env)->FindClass(env, "java/lang/Integer"), "intValue", "()I")) : 0;

            struct TextureInformation* informationPtr = array.data + index;

            if (w <= 0) w = (jint)(((float)informationPtr->width / (float)informationPtr->height) * (float)h);
            if (h <= 0) h = (jint)(((float)informationPtr->height / (float)informationPtr->width) * (float)w);

            textureResize(array.data + index, w, h);
        }

        struct TextureInformation information = array.data[index];
        const struct ImageBuffer imageBuffer = imageGenerate(formatOutput, information, preferredQuality);

        if (imageBuffer.buffer != NULL) {
            jobject imageByteBuffer = (*env)->CallStaticObjectMethod(
                env, byteBufferClass,
                (*env)->GetStaticMethodID(env, byteBufferClass, "allocateDirect", "(I)Ljava/nio/ByteBuffer;"),
                imageBuffer.size
            );
            memcpy((*env)->GetDirectBufferAddress(env, imageByteBuffer), imageBuffer.buffer, imageBuffer.size);
            (*env)->SetObjectArrayElement(env, byteBufferArray, preferredImageIndex != NULL ? 0 : index, imageByteBuffer);
        } else goto IkuyoProcessFail;
        imageBufferFree(imageBuffer);
    }

    textureArrayFree(&array);
    return byteBufferArray;

IkuyoProcessFail:
    textureArrayFree(&array);
    return NULL;
}