#ifndef MYSHADER_H
#define MYSHADER_H

#include "include/GShader.h"
#include "include/GMatrix.h"
#include "include/GBitmap.h"
#include "include/GMath.h"

class MyShader : public GShader {
    public:
        MyShader(const GBitmap& device, const GMatrix& localMatrix) : bm(device), lm(localMatrix){};
        bool isOpaque() override;
        virtual bool setContext(const GMatrix& ctm) override;
        virtual void shadeRow(int x, int y, int count, GPixel row[]) override;

    private:
        const GBitmap bm;
        const GMatrix lm;
        std::optional<GMatrix> inv;
};

#endif