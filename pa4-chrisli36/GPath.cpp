#include "include/GPath.h"

void GPath::addRect(const GRect& rect, Direction dir) {
    this->moveTo(rect.left, rect.top);
    if(dir == kCW_Direction) {
        this->lineTo(rect.right, rect.top);
        this->lineTo(rect.right, rect.bottom);
        this->lineTo(rect.left, rect.bottom);
    } else if(dir == kCCW_Direction) {
        this->lineTo(rect.left, rect.bottom);
        this->lineTo(rect.right, rect.bottom);
        this->lineTo(rect.right, rect.top);
    }
}

void GPath::addPolygon(const GPoint pts[], int count) {
    this->moveTo(pts[0]);
    for(int i = 1; i < count; ++i) {
        this->lineTo(pts[i]);
    }
}

GRect GPath::bounds() const {
    if(fPts.size() == 0) {
        return {0.f, 0.f, 0.f, 0.f};
    }
    float left  = fPts[0].x, right = fPts[0].x;
    float top  = fPts[0].y, bottom = fPts[0].y;
    for(int i = 1; i < fPts.size(); ++i) {
        left = std::min(left, fPts[i].x);
        top = std::min(top, fPts[i].y);
        right = std::max(right, fPts[i].x);
        bottom = std::max(bottom, fPts[i].y);
    }
    return GRect::LTRB(left, top, right, bottom);
}

void GPath::transform(const GMatrix& mat) {
    for(int i = 0; i < fPts.size(); ++i) {
        fPts.at(i) = {
            fPts[i].x * mat[0] + fPts[i].y * mat[2] + mat[4],
            fPts[i].y * mat[1] + fPts[i].y * mat[3] + mat[5]
        };
    }
}