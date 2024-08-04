#ifndef MYSWEEPMATRIX_H
#define MYSWEEPMATRIX_H

#include "include/GShader.h"
#include "include/GFinal.h"
#include "Utility.h"
#include <iostream>

class MySweepShader : public GShader {
public:
    MySweepShader(GPoint cen, float s, const GColor c[], int count) : center(cen), numColors(count), startRadians(s) {
        colors = new GColor[numColors];
        for(int i = 0; i < numColors; ++i) {
            colors[i] = c[i];
        }
    };
    bool isOpaque() override { return false; };
    virtual bool setContext(const GMatrix& ctm) override {
        inv = ctm.invert();
        if(!inv.has_value()) {
            return false;
        }
        return true;
    };
    virtual void shadeRow(int x, int y, int count, GPixel row[]) override {
        GPoint startPt = ((*inv) * (GPoint) {x + 0.5f, y + 0.5f});
        float currX = startPt.x; float currY = startPt.y;
        GVector vec;
        GColor newC;
        float angle;
        int k;
        float t;

        for(int i = 0; i < count; ++i) {
            vec = (GPoint) {currX, currY} - center;
            angle = std::atan2(vec.y, vec.x) - startRadians;
            while(angle < 0.f) {
                angle += M_1_PI;
            }
            angle *= numColors - 1;
            k = GFloorToInt(angle);
            t = angle - k;
            newC = colors[k % numColors] * (1 - t) + colors[(k + 1) % numColors] * t;

            row[i] = premultiplyScale(newC);
            currX += (*inv)[0]; currY += (*inv)[1];
        }
    };

private:
    GPoint center;
    GColor* colors;
    std::optional<GMatrix> inv;
    int numColors;
    float startRadians;
};

#endif