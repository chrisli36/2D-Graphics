#include "MyShader.h"

bool MyShader::isOpaque() {
    return bm.isOpaque();
}

bool MyShader::setContext(const GMatrix& ctm) {
    inv = (ctm * lm).invert();
    return inv.has_value();
}

void MyShader::shadeRow(int x, int y, int count, GPixel row[]) {
    float currX = (*inv)[0] * (x + 0.5f) + (*inv)[2] * (y + 0.5f) + (*inv)[4];
    float currY = (*inv)[1] * (x + 0.5f) + (*inv)[3] * (y + 0.5f) + (*inv)[5];
    int ix, iy;

    if((*inv)[1] == 0) {
        iy = std::min(std::max(0, GFloorToInt(currY)), bm.height() - 1);
        for(int i = 0; i < count; ++i) {
            ix = std::min(std::max(0, GFloorToInt(currX)), bm.width() - 1);
            row[i] = *(bm.getAddr(ix, iy));
            currX += (*inv)[0];
        }
        return;
    }
    for(int i = 0; i < count; ++i) {
        ix = std::min(std::max(0, GFloorToInt(currX)), bm.width() - 1);
        iy = std::min(std::max(0, GFloorToInt(currY)), bm.height() - 1);
        row[i] = *(bm.getAddr(ix, iy));
        currX += (*inv)[0]; currY += (*inv)[1];
    }
}

std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& bitmap, const GMatrix& localMatrix) {
    return std::unique_ptr<GShader>(new MyShader(bitmap, localMatrix));
};