#ifndef MYSHADER_H
#define MYSHADER_H

#include "include/GShader.h"
#include "include/GMatrix.h"
#include "include/GBitmap.h"
#include "include/GMath.h"

class MyShader : public GShader {
    public:
        MyShader(const GBitmap& device, const GMatrix& localMatrix, GTileMode m) : 
            bm(device), 
            lm(localMatrix), 
            invW(1.f / (float) (device.width())),
            invH(1.f / (float) (device.height())),
            mode(m)
            {};
        bool isOpaque() override;
        virtual bool setContext(const GMatrix& ctm) override;
        virtual void shadeRow(int x, int y, int count, GPixel row[]) override;
        
    private:
        static const int clamp(float x, int len) {
            return std::min(GFloorToInt(std::min(std::max(x, 0.f), 1.f) * len), len - 1);
        };
        static const int repeat(float x, int len) {
            return std::min(GFloorToInt((x - GFloorToInt(x)) * len), len - 1);
        };
        static const int mirror(float x, int len) {
            x = x - 2 * GFloorToInt(x * 0.5f);
            if(x > 1.f) {
                x = 2.f - x;
            }
            return std::min(GFloorToInt(x * len), len - 1);
        };
        
        const GBitmap bm;
        const GMatrix lm;
        std::optional<GMatrix> inv;
        const float invW;
        const float invH;
        const GTileMode mode;
        const int (*getTProcs[3])(float, int) = {clamp, repeat, mirror};
};

#endif