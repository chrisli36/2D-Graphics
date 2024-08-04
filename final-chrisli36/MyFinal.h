#ifndef MYFINAL_H
#define MYFINAL_H

#include "include/GFinal.h"
#include "MyCMatrixShader.h"
#include "MySweepShader.h"

class MyFinal : public GFinal {
    std::unique_ptr<GShader> createSweepGradient(GPoint center, float startRadians, const GColor colors[], int count) override;
    std::unique_ptr<GShader> createColorMatrixShader(const GColorMatrix& cMatrix, GShader* realShader) override;
    GPath strokePolygon(const GPoint[], int count, float width, bool isClosed) override;
};

#endif