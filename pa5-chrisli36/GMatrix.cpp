#include "include/GMatrix.h"

GMatrix::GMatrix() {
    fMat[0] = 1.f;    fMat[2] = 0.f;    fMat[4] = 0.f;
    fMat[1] = 0.f;    fMat[3] = 1.f;    fMat[5] = 0.f;
}

GMatrix GMatrix::Translate(float tx, float ty) {
    return GMatrix(1.f, 0.f, tx, 0.f, 1.f, ty);
}

GMatrix GMatrix::Scale(float sx, float sy) {
    return GMatrix(sx, 0.f, 0.f, 0.f, sy, 0.f);
}

GMatrix GMatrix::Rotate(float radians) {
    const float c = cos(radians);
    const float s = sin(radians);
    return GMatrix(c, -s, 0.f, s, c, 0.f);
}

GMatrix GMatrix::Concat(const GMatrix& a, const GMatrix& b) {
    return GMatrix(a[0] * b[0] + a[2] * b[1], a[0] * b[2] + a[2] * b[3], a[0] * b[4] + a[2] * b[5] + a[4],
                    a[1] * b[0] + a[3] * b[1], a[1] * b[2] + a[3] * b[3], a[1] * b[4] + a[3] * b[5] + a[5]);
}

std::optional<GMatrix> GMatrix::invert() const {
    const float adbc = fMat[0] * fMat[3] - fMat[1] * fMat[2];
    if(adbc == 0) {
        return {};
    }
    return GMatrix(fMat[3] / adbc, -fMat[2] / adbc, (fMat[2] * fMat[5] - fMat[3] * fMat[4]) / adbc,
                    -fMat[1] / adbc, fMat[0] / adbc, (fMat[1] * fMat[4] - fMat[0] * fMat[5]) / adbc);
}

void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
    if(dst == src) {
        float srcX, srcY;
        for(int i = 0; i < count; i++) {
            srcX = src[i].x; srcY = src[i].y;
            dst[i].x = fMat[0] * srcX + fMat[2] * srcY + fMat[4];
            dst[i].y = fMat[1] * srcX + fMat[3] * srcY + fMat[5];
        }
        return;
    }
    for(int i = 0; i < count; i++) {
        dst[i].x = fMat[0] * src[i].x + fMat[2] * src[i].y + fMat[4];
        dst[i].y = fMat[1] * src[i].x + fMat[3] * src[i].y + fMat[5];
    }
}