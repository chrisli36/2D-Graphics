#include "MyTriShader.h"

MyTriShader::MyTriShader(const GPoint p[3], const GColor c[3])
         : 
        lm(computeBasis(p[0], p[1], p[2])), 
        colors(c), 
        opaque(getOpaque()) {
    setContext(GMatrix());
}

const bool MyTriShader::getOpaque() {
    return colors[0].a == 1.f && colors[1].a == 1.f && colors[2].a == 1.f;
}

bool MyTriShader::isOpaque() {
    return opaque;
}

bool MyTriShader::setContext(const GMatrix& ctm) {
    inv = (ctm * lm).invert();
    return inv.has_value();
}

void MyTriShader::shadeRow(int x, int y, int count, GPixel row[]) {
    GPoint startPt = ((*inv) * (GPoint) {x + 0.5f, y + 0.5f});
    const GColor colorDiff20 = colors[2] - colors[0];
    const GColor colorDiff10 = colors[1] - colors[0];

    GColor newC = startPt.x * colorDiff20 + startPt.y * colorDiff10 + colors[0];
    const GColor colorDiff = (*inv)[0] * colorDiff20 + (*inv)[1] * colorDiff10;
    for(int i = 0; i < count; ++i) {
        row[i] = premultiplyScale(newC);
        newC += colorDiff;
    }
}
