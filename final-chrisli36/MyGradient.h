#ifndef MYGRADIENT_H
#define MYGRADIENT_H

#include "include/GShader.h"
#include "include/GMatrix.h"
#include "include/GMath.h"
#include "Utility.h"

class MyGradient : public GShader {
public:
    MyGradient(GPoint p0, GPoint p1, const GColor c[], int count, GTileMode m);
    bool isOpaque() override;
    virtual bool setContext(const GMatrix& ctm) override;
    virtual void shadeRow(int x, int y, int count, GPixel row[]) override;

private:
    const GMatrix setLM(GPoint p0, GPoint p1);
    const bool setOpaqueAndColors(const GColor c[]);
    static const float clamp(float x) {
        return std::min(std::max(x, 0.f), 1.f);
    };
    static const float repeat(float x) {
        return x - GFloorToInt(x);
    };
    static const float mirror(float x) {
        x = x - 2 * GFloorToInt(x * 0.5f);
        if(x > 1.f) {
            x = 2.f - x;
        }
        return x;
    };

    const GMatrix lm;
    std::optional<GMatrix> inv;
    GColor* colors;
    GColor* colorDiffs;
    const int colorCount;
    const bool opaque;
    const GTileMode mode;
    const float (*getTProcs[3])(float) = {clamp, repeat, mirror};
};

#endif