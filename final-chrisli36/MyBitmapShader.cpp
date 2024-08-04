#include "MyBitmapShader.h"

bool MyBitmapShader::isOpaque() {
    return bm.isOpaque();
}

bool MyBitmapShader::setContext(const GMatrix& ctm) {
    inv = (ctm * lm).invert();
    if(!inv.has_value()) {
        return false;
    }
    // scale the inv matrix so everything gets mapped between 0 and 1
    inv = GMatrix::Scale(invW, invH) * *(inv);
    return true;
}

void MyBitmapShader::shadeRow(int x, int y, int count, GPixel row[]) {
    GPoint startPt = ((*inv) * (GPoint) {x + 0.5f, y + 0.5f});
    float currX = startPt.x; 
    float currY = startPt.y;
    int ix, iy;

    if((*inv)[1] == 0) {
        // iy only needs to be calculated once
        iy = getTProcs[(int) mode](currY, bm.height());
        for(int i = 0; i < count; ++i) {
            // using clamp, repeat, or mirror tiling, get ix
            ix = getTProcs[(int) mode](currX, bm.width());
            row[i] = *(bm.getAddr(ix, iy));
            currX += (*inv)[0]; // update currX
        }
        return;
    }
    for(int i = 0; i < count; ++i) {
        // using clamp, repeat, or mirror tiling, get ix, iy
        ix = getTProcs[(int) mode](currX, bm.width()); 
        iy = getTProcs[(int) mode](currY, bm.height());
        row[i] = *(bm.getAddr(ix, iy));
        currX += (*inv)[0]; currY += (*inv)[1]; // update currX, currY
    }
}

std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& bitmap, const GMatrix& localMatrix, GTileMode m) {
    return std::unique_ptr<GShader>(new MyBitmapShader(bitmap, localMatrix, m));
};