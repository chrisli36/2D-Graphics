#include "MyFinal.h"

std::unique_ptr<GShader> MyFinal::createSweepGradient(GPoint center, float startRadians, const GColor colors[], int count) {
    return std::unique_ptr<MySweepShader>(new MySweepShader(center, startRadians, colors, count));
}

std::unique_ptr<GShader> MyFinal::createColorMatrixShader(const GColorMatrix& cMatrix, GShader* realShader) {
    return std::unique_ptr<MyCMatrixShader>(new MyCMatrixShader(cMatrix, realShader));
}

GPath MyFinal::strokePolygon(const GPoint points[], int count, float width, bool isClosed) {
    GPath path;
    GPoint p0, p1;
    GVector offset;
    float widthDiv2 = width / 2.f;
    float end = isClosed ? count : count - 1;
    for(int i = 0; i < end; ++i) {
        p0 = points[i]; p1 = points[(i + 1) % count];
        offset = (p1 - p0) * (1.f / magnitude(p1 - p0));
        offset = widthDiv2 * ((GVector) {-offset.y, offset.x});

        path.moveTo(p0 + offset);
        path.lineTo(p0 - offset);
        path.lineTo(p1 - offset);
        path.lineTo(p1 + offset);
        path.lineTo(p0 + offset);
        
        path.addCircle(p0, widthDiv2);
        path.addCircle(p1, widthDiv2);
    }
    return path;
}

std::unique_ptr<GFinal> GCreateFinal() {
    return std::unique_ptr<GFinal>(new MyFinal());
}