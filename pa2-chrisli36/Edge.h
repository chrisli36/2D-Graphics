#ifndef Edge_DEFINED
#define Edge_DEFINED

#include "include/GPoint.h"
#include "include/GMath.h"
#include <iostream>

struct Edge {
    float m, b;
    int top, bottom;

    Edge(GPoint p0, GPoint p1) {
        this->m = computeM(p0, p1);
        this->b = computeB(p0, this->m);
        this->top = GRoundToInt(std::min(p0.y, p1.y));
        this->bottom = GRoundToInt(std::max(p0.y, p1.y));
    }
    Edge(float m, float b, int top, int bottom) {
        this->m = m;
        this->b = b;
        this->top = top;
        this->bottom = bottom;
    }

    friend std::ostream& operator<<(std::ostream& os, const Edge& e) {
        os << "Point 1(" << Edge::eval(e.m, e.b, e.top) << ", " << e.top << ") and ";
        os << "Point 2(" << Edge::eval(e.m, e.b, e.bottom) << "," << e.bottom << ")";
        os << "\t Equation: x = " << e.m << "y + " << e.b;
        return os;
    }
    static const bool comparator(Edge* e0, Edge* e1) {
        return e0->top < e1->top;
    }

    static inline const float computeM(GPoint p0, GPoint p1) {
        return (p1.x - p0.x) / (p1.y - p0.y);
    }
    static inline const float computeB(GPoint p0, float m) {
        return p0.x - m * p0.y;
    }
    static inline const int computeTop(GPoint p0, GPoint p1) {
        return GRoundToInt(std::min(p0.y, p1.y));
    }
    static inline const int computeBottom(GPoint p0, GPoint p1) {
        return GRoundToInt(std::max(p0.y, p1.y));
    }

    float eval(float y) {
        return this->m * y + this->b;
    }
    static float eval(float m, float b, float y) {
        return m * y + b;
    }

    bool isValid(float y) {
        return this->top <= y && y <= this->bottom;
    }
};

#endif