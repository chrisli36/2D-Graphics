#ifndef UTILITY_H
#define UTILITY_H

#include "include/GPoint.h"

static const inline GPoint computeLin(float t, GPoint A, GPoint B) {
    const float invt = 1 - t;
    return invt * A + t * B;
}

static const inline GPoint computeQuad(float t, GPoint A, GPoint B, GPoint C) {
    const float invt = 1 - t;
    return invt * invt * A + 2 * t * invt * B + t * t * C;
}

static const inline GPoint computeCubic(float t, GPoint A, GPoint B, GPoint C, GPoint D) {
    const float invt = 1 - t;
    return invt * invt * invt * A + 3 * invt * invt * t * B + 3 * invt * t * t * C + t * t * t * D;
}

static const inline float magnitude(GVector v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

#endif