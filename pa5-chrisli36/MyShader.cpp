#include "MyShader.h"

bool MyShader::isOpaque() {
    return bm.isOpaque();
}

bool MyShader::setContext(const GMatrix& ctm) {
    inv = (ctm * lm).invert();
    if(!inv.has_value()) {
        return false;
    }
    inv = GMatrix::Scale(invW, invH) * *(inv);
    return true;
}

void MyShader::shadeRow(int x, int y, int count, GPixel row[]) {
    GPoint startPt = ((*inv) * (GPoint) {x + 0.5f, y + 0.5f});
    float currX = startPt.x; 
    float currY = startPt.y;
    int ix, iy;

    if((*inv)[1] == 0) {
        iy = getTProcs[(int) mode](currY, bm.height());
        for(int i = 0; i < count; ++i) {
            ix = getTProcs[(int) mode](currX, bm.width());
            row[i] = *(bm.getAddr(ix, iy));
            currX += (*inv)[0];
        }
        return;
    }
    for(int i = 0; i < count; ++i) {
        ix = getTProcs[(int) mode](currX, bm.width());
        iy = getTProcs[(int) mode](currY, bm.height());
        row[i] = *(bm.getAddr(ix, iy));
        currX += (*inv)[0]; currY += (*inv)[1];
    }
}

std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& bitmap, const GMatrix& localMatrix, GTileMode m) {
    return std::unique_ptr<GShader>(new MyShader(bitmap, localMatrix, m));
};