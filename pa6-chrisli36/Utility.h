#ifndef UTILITY_H
#define UTILITY_H

#include "include/GPoint.h"
#include "include/GPixel.h"

static inline uint32_t GDiv255(unsigned prod) {
    return (prod + 128) * 257 >> 16;
}

static inline const GPixel premultiplyScale(const GColor& color) {
    if(color.a == 1.f) {
        return GPixel_PackARGB(255, 
                                GRoundToInt(color.r * 255),
                                GRoundToInt(color.g * 255),
                                GRoundToInt(color.b * 255));
    }
    return GPixel_PackARGB(GRoundToInt(color.a * 255), 
                            GRoundToInt(color.r * color.a * 255), 
                            GRoundToInt(color.g * color.a * 255), 
                            GRoundToInt(color.b * color.a * 255));
};

static const inline GPoint computeLin(float t, const GPoint& A, const GPoint& B) {
    const float invt = 1 - t;
    return invt * A + t * B;
}

static const inline GPoint computeQuad(float t, const GPoint& A, const GPoint& B, const GPoint& C) {
    const float invt = 1 - t;
    return invt * invt * A + 2 * t * invt * B + t * t * C;
}

static const inline GPoint computeCubic(float t, const GPoint& A, const GPoint& B, const GPoint& C, const GPoint& D) {
    const float invt = 1 - t;
    return invt * invt * invt * A + 3 * invt * invt * t * B + 3 * invt * t * t * C + t * t * t * D;
}

static const inline float magnitude(const GVector& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

static inline GMatrix computeBasis(const GPoint& p0, const GPoint& p1, const GPoint& p2) {
    return GMatrix(p2 - p0, p1 - p0, p0);
}

static inline GPoint quadrulate(const float u, const float v, const GPoint verts[4]) {
    return computeLin(v, computeLin(u, verts[0], verts[1]), computeLin(u, verts[3], verts[2]));
}

static inline GColor quadrulate(const float u, const float v, const GColor colors[4]) {
    return (1.f - v) * ((1.f - u) * colors[0] + u * colors[1]) + v * ((1.f - u) * colors[3] + u * colors[2]);
}

#endif