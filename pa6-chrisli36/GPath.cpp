#include "include/GPath.h"
#include "Utility.h"

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

void GPath::addCircle(GPoint center, float radius, Direction dir) {
    const float num = 0.552284749f;
    const int numPoints = 13;
    const GPoint unitCircle[numPoints] = {
        {1, 0}, 
        {1, -num}, 
        {num, -1},
        {0, -1}, 
        {-num, -1}, 
        {-1, -num}, 
        {-1, 0},
        {-1, num},
        {-num, 1},
        {0, 1}, 
        {num, 1}, 
        {1, num}, 
        {1, 0}};
    GPoint circle[numPoints];
    GMatrix transform = GMatrix::Translate(center.x, center.y) * GMatrix::Scale(radius, radius);
    transform.mapPoints(circle, unitCircle, numPoints);
    
    this->moveTo(circle[0]);
    if(dir == kCCW_Direction) {
        for (int i = 1; i <= 10; i += 3) {
            this->cubicTo(circle[i], circle[i + 1], circle[i + 2]);
        }
    } else if(dir == kCW_Direction) {
        for (int i = 9; i >= 0; i -= 3) {
            this->cubicTo(circle[i + 2], circle[i + 1], circle[i]);
        }
    }
}

inline void updateExtremaQuad(float t, std::vector<GPoint>* list, GPoint A, GPoint B, GPoint C) {
    if(t <= 0 || t >= 1) {
        return;
    }
    list->push_back(computeQuad(t, A, B, C));
};

inline void updateExtremaCubic(float t, std::vector<GPoint>* list, GPoint A, GPoint B, GPoint C, GPoint D) {
    if(t <= 0 || t >= 1) {
        return;
    }
    list->push_back(computeCubic(t, A, B, C, D));
};

GRect GPath::bounds() const {
    if(fPts.size() == 0) {
        return {0.f, 0.f, 0.f, 0.f};
    }
    float left  = fPts[0].x, right = fPts[0].x;
    float top  = fPts[0].y, bottom = fPts[0].y;
    GPoint edge[GPath::kMaxNextPoints];
    GPath::Edger edger(*this);
    std::optional<GPath::Verb> v;
    int i, j;
    std::vector<GPoint>* extrema = new std::vector<GPoint>();
    float a, b, c, t;
    while((v = edger.next(edge)).has_value()) {
        extrema->clear();
        switch(v.value()) {
            case GPath::kMove:
                break;
            case GPath::kLine:
                i = 0; j = 1;
                break;
            case GPath::kQuad:
                i = 0; j = 2;
                b = edge[0].x - 2 * edge[1].x + edge[2].x;
                if(b != 0) {
                    c = edge[0].x - edge[1].x;
                    t = c / b;
                    updateExtremaQuad(t, extrema, edge[0], edge[1], edge[2]);
                }
                b = edge[0].y - 2 * edge[1].y + edge[2].y;
                if(b != 0) {
                    c = edge[0].y - edge[1].y;
                    t = c / b;
                    updateExtremaQuad(t, extrema, edge[0], edge[1], edge[2]);
                }
                break;
            case GPath::kCubic:
                i = 0; j = 3;
                a = -edge[0].x + 3 * edge[1].x - 3 * edge[2].x + edge[3].x;
                b = edge[0].x - 2 * edge[1].x + edge[2].x; 
                c = edge[1].x - edge[0].x;
                if(a != 0 && (b * b - a * c) >= 0) {
                    t = (-b + std::sqrt(b * b - a * c)) / a;
                    updateExtremaCubic(t, extrema, edge[0], edge[1], edge[2], edge[3]);
                    t = (-b - std::sqrt(b * b - a * c)) / a;
                    updateExtremaCubic(t, extrema, edge[0], edge[1], edge[2], edge[3]);
                } else if(a == 0 && b != 0) {
                    t = -c / (2 * b);
                    updateExtremaQuad(t, extrema, edge[0], edge[1], edge[2]);
                }
                a = -edge[0].y + 3 * edge[1].y - 3 * edge[2].y + edge[3].y;
                b = edge[0].y - 2 * edge[1].y + edge[2].y;
                c = edge[1].y - edge[0].y;
                if(a != 0 && (b * b - a * c) >= 0) {
                    t = (-b + std::sqrt(b * b - a * c)) / a;
                    updateExtremaCubic(t, extrema, edge[0], edge[1], edge[2], edge[3]);
                    t = (-b - std::sqrt(b * b - a * c)) / a;
                    updateExtremaCubic(t, extrema, edge[0], edge[1], edge[2], edge[3]);
                } else if(a == 0 && b != 0) {
                    t = -c / (2 * b);
                    updateExtremaQuad(t, extrema, edge[0], edge[1], edge[2]);
                }
                break;
        }
        left = std::min(std::min(left, edge[i].x), edge[j].x); 
        right = std::max(std::max(right, edge[i].x), edge[j].x);
        top = std::min(std::min(top, edge[i].y), edge[j].y); 
        bottom = std::max(std::max(bottom, edge[i].y), edge[j].y);
        for(int k = 0; k < extrema->size(); ++k) {
            left = std::min(left, (*extrema)[k].x); 
            right = std::max(right, (*extrema)[k].x);
            top = std::min(top, (*extrema)[k].y); 
            bottom = std::max(bottom, (*extrema)[k].y);
        }
    }
    return GRect::LTRB(left, top, right, bottom);
}

void GPath::transform(const GMatrix& mat) {
    for(int i = 0; i < fPts.size(); ++i) {
        fPts[i] = mat * fPts[i];
    }
}

void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t) {
    const GPoint AB = computeLin(t, src[0], src[1]);
    const GPoint BC = computeLin(t, src[1], src[2]);
    const GPoint pt = computeQuad(t, src[0], src[1], src[2]);
    dst[0] = src[0];
    dst[1] = AB;
    dst[2] = pt;
    dst[3] = BC;
    dst[4] = src[2];
}

void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t) {
    const GPoint AB = computeLin(t, src[0], src[1]);
    const GPoint BC = computeLin(t, src[1], src[2]);
    const GPoint CD = computeLin(t, src[2], src[3]);
    const GPoint ABC = computeLin(t, AB, BC);
    const GPoint BCD = computeLin(t, BC, CD);
    const GPoint pt = computeCubic(t, src[0], src[1], src[2], src[3]);
    dst[0] = src[0];
    dst[1] = AB;
    dst[2] = ABC;
    dst[3] = pt;
    dst[4] = BCD;
    dst[5] = CD;
    dst[6] = src[3];
}