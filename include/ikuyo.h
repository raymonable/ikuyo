//
// Created by Raymond on 3/29/2026.
//

#ifndef IKUYO_H
#define IKUYO_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <string.h>
#include <math.h>

#if WIN32
    #define IKUYO_EXPORT __declspec(dllexport)
#else
    #define IKUYO_EXPORT __attribute__((visibility("default")))
#endif

#endif //IKUYO_H