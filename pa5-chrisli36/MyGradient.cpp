#include "MyGradient.h"

MyGradient::MyGradient(GPoint p0, GPoint p1, const GColor c[], int count, GTileMode m) 
    :   lm(setLM(p0, p1)),
        colorCount(count), 
        opaque(setOpaqueAndColors(c)),
        mode(m) {
    setContext(GMatrix());
}

const GMatrix MyGradient::setLM(GPoint p0, GPoint p1) {
    const float dx = p1.x - p0.x;
    const float dy = p1.y - p0.y;
    return GMatrix(dx, -dy, p0.x, dy, dx, p0.y);
}

const bool MyGradient::setOpaqueAndColors(const GColor c[]) {
    colors = new GColor[colorCount];
    colorDiffs = new GColor[colorCount];
    bool ret = true;
    for(int i = 0; i < colorCount - 1; ++i) {
        ret = ret && (c[i].a == 1.f);
        colors[i] = c[i];
        colorDiffs[i] = c[i + 1] - c[i];
    }
    colors[colorCount - 1] = c[colorCount - 1];
    colorDiffs[colorCount - 1] = {0, 0, 0, 0};
    return ret;
}

bool MyGradient::isOpaque() {
    return opaque;
}

bool MyGradient::setContext(const GMatrix& ctm) {
    inv = (ctm * lm).invert();
    return inv.has_value();
}

void MyGradient::shadeRow(int x, int y, int count, GPixel row[]) {
    if(colorCount == 1) {
        const GPixel px = premultiplyScale(colors[0]);
        for(int i = 0; i < count; ++i) {
            row[i] = px;
        }
        return;
    }
    if(colorCount == 2) {
        float currX = (*inv)[0] * (x + 0.5f) + (*inv)[2] * (y + 0.5f) + (*inv)[4];
        GColor newC; float t;
        for(int i = 0; i < count; ++i) {
            t = getTProcs[(int) mode](currX);
            newC = colors[0] + t * colorDiffs[0];
            row[i] = premultiplyScale(newC);
            currX += (*inv)[0];
        }
        return;
    }
    float currX = (*inv)[0] * (x + 0.5f) + (*inv)[2] * (y + 0.5f) + (*inv)[4];
    float newX; int k; GColor newC; float t;
    for(int i = 0; i < count; ++i) {
        newX = getTProcs[(int) mode](currX) * (colorCount - 1);
        k = GFloorToInt(newX);
        t = newX - k;
        newC = colors[k] + t * colorDiffs[k];
        row[i] = premultiplyScale(newC);
        currX += (*inv)[0];
    }
}

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GTileMode m) {
    return std::unique_ptr<GShader>(new MyGradient(p0, p1, colors, count, m));
};