/*
 *  Copyright 2024 <Christopher Li>
 */

#include "MyCanvas.h"

void MyCanvas::save() {
    stack.push_back(stack.back());
}

void MyCanvas::restore() {
    stack.pop_back();
}

void MyCanvas::concat(const GMatrix& matrix) {
    stack.back() = GMatrix::Concat(stack.back(), matrix);
}

void MyCanvas::clear(const GColor& color) {
    const GPixel& coloredPixel = premultiplyAndScale(color);

    GBitmap bm = this->fDevice;
    for(int y = 0; y < bm.height(); y++) {
        for(int x = 0; x < bm.width(); x++) {
            GPixel* pixel = bm.getAddr(x, y);
            *pixel = coloredPixel;
        }
    }
}

void MyCanvas::drawRect(const GRect& rect, const GPaint& paint) {
    if(stack.back() != GMatrix()) {
        GPoint points[4] = {{rect.left, rect.top}, 
                            {rect.right, rect.top},
                            {rect.right, rect.bottom},
                            {rect.left, rect.bottom}};
        drawConvexPolygon(points, 4, paint);
        return;
    }
    const GColor& color = paint.getColor();
    const GBlendMode blendMode = paint.getBlendMode();
    if(blendMode == GBlendMode::kDst) {return;}
    GShader* shader = paint.getShader();
    if(shader != nullptr) {
        shader->setContext(stack.back());
    }

    GPixel src = premultiplyAndScale(color);
    const BlendProc proc = getBlendProcs(shader)[(int) blendMode];
    const BlendRowProc rowProc = getBlendRowProcs(src)[(int) blendMode];
    
    const GIRect& r2 = intersect(rect.round(), this->fDevice.width(), this->fDevice.height());
    for(int y = r2.top; y < r2.bottom; ++y) {
        if(shader != nullptr) {
            blitRowShade(r2.left, r2.right, y, src, proc, shader);
        } else {
            blitRow(r2.left, r2.right, y, src, rowProc);
        }
    }
}

template <class T> void MyCanvas::blitRow(int l, int r, int y, GPixel& src, T proc) {
    proc(l, r, y, src, this->fDevice);
}

template <class T> void MyCanvas::blitRowShade(int l, int r, int y, GPixel& src, T proc, GShader* shader) {
    const int n = r - l;
    GPixel tmp[n];
    shader->shadeRow(l, y, n, tmp);
    GPixel* dst;
    for(int x = l, i = 0; x < r; ++x, ++i) {
        dst = fDevice.getAddr(x, y);
        *dst = proc(tmp[i], *dst);
    }
}

void MyCanvas::drawConvexPolygon(const GPoint points[], int count, const GPaint& paint) {
    const GColor& color = paint.getColor();
    const GBlendMode blendMode = paint.getBlendMode();
    if(blendMode == GBlendMode::kDst) {return;}
    GShader* shader = paint.getShader();
    if(shader != nullptr) {
        shader->setContext(stack.back());
    }

    GPoint newPoints[count];
    stack.back().mapPoints(newPoints, points, count);
    GPixel src = premultiplyAndScale(color);
    const BlendProc proc = getBlendProcs(shader)[(int) blendMode];
    const BlendRowProc rowProc = getBlendRowProcs(src)[(int) blendMode];
    
    std::vector<Edge*>* edges = new std::vector<Edge*>;
    for(int i = 0; i < count - 1; ++i) {
        this->clipEdge(newPoints[i], newPoints[i + 1], edges);
    } 
    this->clipEdge(newPoints[count - 1], newPoints[0], edges);
    if(count < 3 || edges->size() < 2) {
        return;
    }
    std::sort(edges->begin(), edges->end(), Edge::comparator);

    int ptr1 = 0; int ptr2 = 1;
    int currY = edges->at(0)->top;
    float currRay = currY + 0.5f;
    int left, right;
    while(ptr1 < edges->size() && ptr2 < edges->size()) {
        left = GRoundToInt(edges->at(ptr1)->eval(currRay));
        right = GRoundToInt(edges->at(ptr2)->eval(currRay));
        if(left > right) {
            std::swap(left, right);
        }

        if(shader != nullptr) {
            blitRowShade(left, right, currY, src, proc, shader);
        } else {
            blitRow(left, right, currY, src, rowProc);
        }

        currY++; currRay++;
        if(!(edges->at(ptr1)->isValid(currRay))) {
            ptr1 = std::max(ptr1, ptr2) + 1;
        }
        if(!(edges->at(ptr2)->isValid(currRay))) {
            ptr2 = std::max(ptr1, ptr2) + 1;
        }
    }

    for (Edge* edge : *edges) {
        delete edge;
    }
    delete edges;
}

void MyCanvas::clipEdge(GPoint p1, GPoint p2, std::vector<Edge*>* edges) {
    float m = Edge::computeM(p1, p2);
    float b = Edge::computeB(p1, m);
    int top = Edge::computeTop(p1, p2);
    int bottom = Edge::computeBottom(p1, p2);

    if(p1.y > p2.y) {  // make p1 be above, p2 be below
        std::swap(p1, p2);
    } if(top == bottom || p2.y < 0 || p1.y > this->fDevice.height()) { // both horizontal/ OB, skip
        return;
    } 
    if(p1.y < 0) { // if p1 OB above, p2 in, then calculate new p1
        p1.x = Edge::eval(m, b, 0.0);
        p1.y = 0.0;
        top = 0;
    } if(p2.y > this->fDevice.height()) { // if p2 OB below, p1 in, then calculate new p2
        p2.x = Edge::eval(m, b, this->fDevice.height());
        p2.y = this->fDevice.height();
        bottom = this->fDevice.height();
    } if(p1.x > p2.x) { // make p1 on left, p2 on right
        std::swap(p1, p2);
    } 
    // now, adjust for left and right
    if(p2.x < 0) { // if both OB left, project to 0
        m = 0;
        b = 0;
        // p1.x = 0; p2.x = 0;
    } else if(p1.x < 0) { // if p1 OB left, but not p2, then bend
        int newY = GRoundToInt(-b / m);
        int p1yint = GRoundToInt(p1.y);
        Edge* vEdge = new Edge(0, 0, std::min(p1yint, newY), std::max(p1yint, newY));
        edges->push_back(vEdge);
        if(p1yint > newY) {
            bottom = newY;
        } else {
            top = newY;
        }
    } else if(p1.x > this->fDevice.width()) { // if both OB right, project to width
        m = 0;
        b = this->fDevice.width();
    } else if(p2.x > this->fDevice.width()) { // if p2 OB right, but not p1, then bend
        int newY = GRoundToInt((this->fDevice.width() - b) / m);
        int p2yint = GRoundToInt(p2.y);
        Edge* vEdge = new Edge(0, this->fDevice.width(), std::min(p2yint, newY), std::max(p2yint, newY));
        edges->push_back(vEdge);
        if(p2yint > newY) {
            bottom = newY;
        } else {
            top = newY;
        }
    }
    if(top == bottom) {
        return;
    }
    Edge* newEdge = new Edge(m, b, top, bottom);
    edges->push_back(newEdge);
}

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    return std::unique_ptr<GCanvas>(new MyCanvas(device));
}

std::string GDrawSomething(GCanvas* canvas, GISize dim) {
    GBitmap bm;
    bm.readFromFile("meatball.png");

    MyShader shader = MyShader(bm, GMatrix::Scale(0.9f, 0.9f) * GMatrix::Translate(20.f, 20.f));
    // MyShader shader = MyShader(bm, GMatrix());

    GPoint face[5] = {
        (GPoint) {50.0f, 35.0f},
        (GPoint) {200.0f, 35.0f},
        (GPoint) {200.0f, 185.0f},
        (GPoint) {125.0f, 235.0f},
        (GPoint) {50.0f, 185.0f},
    };
    GPaint* paint = new GPaint();
    paint->setShader(&shader);
    paint->setRGBA(0.996f, 0.204f, 0.996f, 0.8f);
    canvas->clear(GColor::RGBA(0.196f, 0.804f, 0.196f, 1.f));
    canvas->drawConvexPolygon(face, 5, *paint);

    GPoint mouth[4] = {
        (GPoint) {100.0f, 150.0f},
        (GPoint) {150.0f, 150.0f},
        (GPoint) {150.0f, 175.0f},
        (GPoint) {100.0f, 175.0f},
    };
    GPaint* paint1 = new GPaint(GColor::RGBA(0.823f, 0.921f, 0.731f, 1.f));
    canvas->drawConvexPolygon(mouth, 4, *paint1);

    GPoint lefteye[3] = {
        (GPoint) {150.0f, 80.0f},
        (GPoint) {160.0f, 95.0f},
        (GPoint) {140.0f, 95.0f},
    };
    GPaint* paint2 = new GPaint(GColor::RGBA(0.723f, 0.321f, 0.831f, 1.f));
    canvas->drawConvexPolygon(lefteye, 3, *paint2);

    GPoint righteye[3] = {
        (GPoint) {100.0f, 80.0f},
        (GPoint) {110.0f, 95.0f},
        (GPoint) {90.0f, 95.0f},
    };
    GPaint paint3 = GPaint(GColor::RGBA(0.623f, 0.621f, 0.931f, 1.f));
    canvas->drawConvexPolygon(righteye, 3, paint3);

    return "a meatball robot";
}
