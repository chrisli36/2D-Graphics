#ifndef Edge_DEFINED
#define Edge_DEFINED

#include "include/GPoint.h"
#include "include/GMath.h"
#include <iostream>

struct Edge {
    float m, b;
    int top, bottom;
    int w;

    Edge() {}
    Edge(GPoint p0, GPoint p1) {
        Edge(p0, p1, computeWinding(p0, p1));
    }
    Edge(GPoint p0, GPoint p1, int w) {
        this->m = computeM(p0, p1);
        this->b = computeB(p0, this->m);
        this->top = computeTop(p0, p1);
        this->bottom = computeBottom(p0, p1);
        this->w = w;
    }
    Edge(float m, float b, int top, int bottom, int w) {
        this->m = m;
        this->b = b;
        this->top = top;
        this->bottom = bottom;
        this->w = w;
    }

    static const bool comparatorY(Edge* e0, Edge* e1) {
        return e0->top < e1->top;
    }
    static const bool comparatorYX(Edge* e0, Edge* e1) {
        if(e0->top == e1->top) {
            return e0->eval(e0->top + 0.5f) < e1->eval(e1->top + 0.5f);
        }
        return e0->top < e1->top;
    }
    static const bool comparatorX(Edge* e0, Edge* e1, float y) {
        return e0->eval(y) < e1->eval(y);
    }

    static inline const float computeM(GPoint& p0, GPoint& p1) {
        return (p1.x - p0.x) / (p1.y - p0.y);
    }
    static inline const float computeB(GPoint& p0, float m) {
        return p0.x - m * p0.y;
    }
    static inline const int computeTop(GPoint& p0, GPoint& p1) {
        return GRoundToInt(std::min(p0.y, p1.y));
    }
    static inline const int computeBottom(GPoint& p0, GPoint& p1) {
        return GRoundToInt(std::max(p0.y, p1.y));
    }
    static inline const int computeWinding(GPoint& p0, GPoint& p1) {
        return (p1.y - p0.y > 0) ? 1 : -1;
    }

    float eval(float y) {
        return this->m * y + this->b;
    }
    static float eval(float m, float b, float y) {
        return m * y + b;
    }
    bool isValid(float y) {
        return this->top <= y && y < this->bottom;
    }
};

#endif