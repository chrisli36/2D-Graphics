#ifndef MYCOMPOSESHADER_H
#define MYCOMPOSESHADER_H

#include "include/GShader.h"
#include "include/GMatrix.h"
#include "Utility.h"

class MyComposeShader : public GShader {
public:
    MyComposeShader(GShader* sh1, GShader* sh2);
    bool isOpaque() override;
    virtual bool setContext(const GMatrix& ctm) override;
    virtual void shadeRow(int x, int y, int count, GPixel row[]) override;

private:
    GShader* sh1;
    GShader* sh2;
    inline GPixel mul(GPixel px1, GPixel px2) {
        return GPixel_PackARGB(GDiv255(GPixel_GetA(px1) * GPixel_GetA(px2)),
                                GDiv255(GPixel_GetR(px1) * GPixel_GetR(px2)),
                                GDiv255(GPixel_GetG(px1) * GPixel_GetG(px2)),
                                GDiv255(GPixel_GetB(px1) * GPixel_GetB(px2)));
    };
};

#endif