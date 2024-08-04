#ifndef MYTRISHADER_H
#define MYTRISHADER_H

#include "include/GShader.h"
#include "include/GMatrix.h"
#include "include/GMath.h"
#include "Utility.h"

class MyTriShader : public GShader {
public:
    MyTriShader(const GPoint p[3], const GColor c[3]);
    bool isOpaque() override;
    virtual bool setContext(const GMatrix& ctm) override;
    virtual void shadeRow(int x, int y, int count, GPixel row[]) override;

private:
    const bool getOpaque();

    const GMatrix lm;
    std::optional<GMatrix> inv;
    const GColor* colors;
    const bool opaque;
};

#endif