#ifndef MYCMATRIXSHADER_H
#define MYCMATRIXSHADER_H

#include "include/GShader.h"
#include "include/GFinal.h"
#include "Utility.h"
#include <iostream>

class MyCMatrixShader : public GShader {
public:
    MyCMatrixShader(const GColorMatrix& c, GShader* s) : shader(s), M(c) {};
    bool isOpaque() override { return false; };
    virtual bool setContext(const GMatrix& ctm) override { return shader->setContext(ctm); };
    virtual void shadeRow(int x, int y, int count, GPixel row[]) override {
        shader->shadeRow(x, y, count, row);
        GPixel pixel;
        GColor old, newColor;
        for(int i = 0; i < count; ++i) {
            pixel = row[i];
            if(GPixel_GetA(pixel) == 0) {
                row[i] = GPixel_PackARGB(0, 0, 0, 0);
                continue;
            }
            
            old.a = (float) GPixel_GetA(pixel) / 255.f;
            old.r = (float) GPixel_GetR(pixel) / (255.f * old.a);
            old.g = (float) GPixel_GetG(pixel) / (255.f * old.a);
            old.b = (float) GPixel_GetB(pixel) / (255.f * old.a);

            newColor.r = M[0] * old.r + M[4] * old.g + M[8] * old.b + M[12] * old.a + M[16];
            newColor.g = M[1] * old.r + M[5] * old.g + M[9] * old.b + M[13] * old.a + M[17];
            newColor.b = M[2] * old.r + M[6] * old.g + M[10] * old.b + M[14] * old.a + M[18];
            newColor.a = M[3] * old.r + M[7] * old.g + M[11] * old.b + M[15] * old.a + M[19];

            newColor.r = std::max(0.f, std::min(1.f, newColor.r));
            newColor.g = std::max(0.f, std::min(1.f, newColor.g));
            newColor.b = std::max(0.f, std::min(1.f, newColor.b));
            newColor.a = std::max(0.f, std::min(1.f, newColor.a));

            row[i] = premultiplyScale(newColor);
        }
    };

private:
    GShader* shader;
    const GColorMatrix M;
};

#endif