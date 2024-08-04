#ifndef MYGRADIENT_H
#define MYGRADIENT_H

#include "include/GShader.h"
#include "include/GMatrix.h"
#include "include/GMath.h"

class MyGradient : public GShader {
public:
    MyGradient(GPoint p0, GPoint p1, const GColor c[], int count, GTileMode m);
    bool isOpaque() override;
    virtual bool setContext(const GMatrix& ctm) override;
    virtual void shadeRow(int x, int y, int count, GPixel row[]) override;

private:
    const GMatrix setLM(GPoint p0, GPoint p1);
    const bool setOpaqueAndColors(const GColor c[]);
    static inline const GPixel premultiplyScale(GColor& color) {
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