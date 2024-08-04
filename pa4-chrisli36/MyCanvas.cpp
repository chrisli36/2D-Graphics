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
    const GPixel& coloredPixel = premultiplyScale(color);

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
    GShader* shader = paint.getShader();
    if(shader != nullptr) {
        shader->setContext(stack.back());
    }

    GPixel src = premultiplyScale(color);
    const BlendProc proc = getBlendProcs(shader)[(int) blendMode];
    BlendRowProc rowProc = getBlendRowProcs(src)[(int) blendMode];
    if((shader != nullptr && proc == (BlendProc) kDst) || (shader == nullptr && rowProc == (BlendRowProc) kDst)) {
        return;
    }
    if(proc == (BlendProc) kClear) {
        shader = nullptr;
        rowProc = (BlendRowProc) kClear;
    }

    const GIRect& r2 = intersect(rect.round(), fDevice.width(), fDevice.height());
    for(int y = r2.top; y < r2.bottom; ++y) {
        blitRow(r2.left, r2.right, y, src, proc, rowProc, shader);
    }
}

template <class T, class S> void MyCanvas::blitRow(int l, int r, int y, GPixel& src, T proc, S rowProc, GShader* shader) {
    if(shader != nullptr) {
        const int n = r - l;
        GPixel tmp[n];
        shader->shadeRow(l, y, n, tmp);
        GPixel* dst;
        for(int x = l, i = 0; x < r; ++x, ++i) {
            dst = fDevice.getAddr(x, y);
            *dst = proc(tmp[i], *dst);
        }
        return;
    }
    rowProc(l, r, y, src, this->fDevice);
}

void MyCanvas::drawConvexPolygon(const GPoint points[], int count, const GPaint& paint) {
    const GColor& color = paint.getColor();
    const GBlendMode blendMode = paint.getBlendMode();
    GShader* shader = paint.getShader();
    if(blendMode == GBlendMode::kDst) {return;}
    if(shader != nullptr) {
        shader->setContext(stack.back());
    }
    GPixel src = premultiplyScale(color);
    const BlendProc proc = getBlendProcs(shader)[(int) blendMode];
    BlendRowProc rowProc = getBlendRowProcs(src)[(int) blendMode];
    if((shader != nullptr && proc == (BlendProc) kDst) || (shader == nullptr && rowProc == (BlendRowProc) kDst)) {
        return;
    }
    if(proc == (BlendProc) kClear) {
        shader = nullptr;
        rowProc = (BlendRowProc) kClear;
    }
    
    GPoint newPoints[count];
    stack.back().mapPoints(newPoints, points, count);
    std::vector<Edge>* edgeStorage = new std::vector<Edge>;
    for(int i = 0; i < count - 1; ++i) {
        clipEdge(newPoints[i], newPoints[i + 1], edgeStorage);
    } 
    clipEdge(newPoints[count - 1], newPoints[0], edgeStorage);
    const int numEdges = edgeStorage->size();
    if(numEdges < 2) {
        return;
    }
    Edge* edgePtrs[numEdges];
    for(int i = 0; i < edgeStorage->size(); ++i) {
        edgePtrs[i] = &edgeStorage->at(i);
    }
    std::sort(edgePtrs, edgePtrs + numEdges, Edge::comparatorY);

    int ptr1 = 0; int ptr2 = 1;
    int currY = edgePtrs[0]->top;
    float currRay = currY + 0.5f;
    int left, right;
    while(ptr1 < numEdges && ptr2 < numEdges) {
        left = GRoundToInt(edgePtrs[ptr1]->eval(currRay));
        right = GRoundToInt(edgePtrs[ptr2]->eval(currRay));
        if(left > right) {
            std::swap(left, right);
        }

        blitRow(left, right, currY, src, proc, rowProc, shader);

        currY++; currRay++;
        if(!(edgePtrs[ptr1]->isValid(currRay))) {
            ptr1 = std::max(ptr1, ptr2) + 1;
        }
        if(!(edgePtrs[ptr2]->isValid(currRay))) {
            ptr2 = std::max(ptr1, ptr2) + 1;
        }
    }
}

void MyCanvas::clipEdge(GPoint p1, GPoint p2, std::vector<Edge>* edges) {
    float m = Edge::computeM(p1, p2);
    float b = Edge::computeB(p1, m);
    int top = Edge::computeTop(p1, p2);
    int bottom = Edge::computeBottom(p1, p2);
    const int w = Edge::computeWinding(p1, p2);

    if(p1.y > p2.y) {  // make p1 be above, p2 be below
        std::swap(p1, p2);
    } 
    if(top == bottom || p2.y < 0 || p1.y > this->fDevice.height()) { // both horizontal/ OB, skip
        return;
    } 
    if(p1.y < 0) { // if p1 OB above, p2 in, then calculate new p1
        // p1.x = Edge::eval(m, b, 0.0);
        p1.x = p1.x + m * (-p1.y);
        p1.y = 0.0;
        top = 0;
    } 
    if(p2.y > fDevice.height()) { // if p2 OB below, p1 in, then calculate new p2
        // p2.x = Edge::eval(m, b, this->fDevice.height());
        p2.x = p1.x + m * (fDevice.height() - p1.y);
        p2.y = fDevice.height();
        bottom = fDevice.height();
    } 
    if(p1.x > p2.x) { // make p1 on left, p2 on right
        std::swap(p1, p2);
    } 
    if(p2.x < 0) { // if both OB left, project to 0
        m = 0;
        b = 0;
        edges->push_back(Edge(m, b, top, bottom, w));
        return;
    }
    if(p1.x > fDevice.width()) { // if both OB right, project to width
        m = 0;
        b = fDevice.width();
        edges->push_back(Edge(m, b, top, bottom, w));
        return;
    } 
    if(p1.x < 0) { // if p1 OB left, then bend
        int newY = GRoundToInt(-b / m);
        int p1yint = GRoundToInt(p1.y);
        if(newY != p1yint) {
            edges->push_back(Edge(0, 0, std::min(p1yint, newY), std::max(p1yint, newY), w));
        }
        bottom = (p1yint > newY) ? newY : bottom;
        top = (p1yint < newY) ? newY : top;
    }
    if(p2.x > fDevice.width()) { // if p2 OB right, then bend
        int newY = GRoundToInt((fDevice.width() - b) / m);
        int p2yint = GRoundToInt(p2.y);
        if(newY != p2yint) {
            edges->push_back(Edge(0, fDevice.width(), std::min(p2yint, newY), std::max(p2yint, newY), w));
        }
        bottom = (p2yint > newY) ? newY : bottom;
        top = (p2yint < newY) ? newY : top;
    }
    if(top == bottom) {
        return;
    }
    edges->push_back(Edge(m, b, top, bottom, w));
}

void MyCanvas::drawPath(const GPath& path, const GPaint& paint) {
    const GColor& color = paint.getColor();
    const GBlendMode blendMode = paint.getBlendMode();
    GShader* shader = paint.getShader();
    if(blendMode == GBlendMode::kDst) {return;}
    if(shader != nullptr) {
        shader->setContext(stack.back());
    }
    GPixel src = premultiplyScale(color);
    const BlendProc proc = getBlendProcs(shader)[(int) blendMode];
    BlendRowProc rowProc = getBlendRowProcs(src)[(int) blendMode];
    if((shader != nullptr && proc == (BlendProc) kDst) || (shader == nullptr && rowProc == (BlendRowProc) kDst)) {
        return;
    }
    if(proc == (BlendProc) kClear) {
        shader = nullptr;
        rowProc = (BlendRowProc) kClear;
    }

    // what's the point of having path.transform?
    std::vector<Edge>* edgeStorage = new std::vector<Edge>;
    GPoint edge[GPath::kMaxNextPoints];
    GPath::Edger edger(path);
    std::optional<GPath::Verb> v;
    while((v = edger.next(edge)).has_value()) {
        clipEdge(mapPoint(edge[0]), mapPoint(edge[1]), edgeStorage);
    }
    const int numEdges = edgeStorage->size();
    if(numEdges < 2) {
        return;
    }
    Edge* edgePtrs[numEdges];
    for(int i = 0; i < edgeStorage->size(); ++i) {
        edgePtrs[i] = &edgeStorage->at(i);
    }
    std::sort(edgePtrs, edgePtrs + numEdges, Edge::comparatorYX);

    int yMin = edgePtrs[0]->top, yMax = edgePtrs[0]->bottom;
    for(int i = 1; i < numEdges; ++i) {
        yMin = std::min(yMin, edgePtrs[i]->top);
        yMax = std::max(yMax, edgePtrs[i]->bottom);
    }
    int start = 0, i, winding, L;
    float currRay, nextRay = yMin + 0.5f;
    int x;
    for(int y = yMin; y < yMax; ++y) {
        i = start; winding = 0; currRay = nextRay; nextRay = y + 1.5f;
        while(i < numEdges && edgePtrs[i]->isValid(currRay)) {
            x = GRoundToInt(edgePtrs[i]->eval(currRay));
            if(winding == 0) {
                L = x;
            }
            winding += edgePtrs[i]->w;
            if(winding == 0) {
                blitRow(L, x, y, src, proc, rowProc, shader);
            }
            if(!edgePtrs[i]->isValid(nextRay)) {
                std::swap(edgePtrs[i], edgePtrs[start]);
                ++start;
            }
            ++i;
        }
        assert(winding == 0);
        while(i < numEdges && edgePtrs[i]->isValid(nextRay)) {
            ++i;
        }
        std::sort(edgePtrs + start, edgePtrs + i, [nextRay](Edge* e0, Edge* e1) {
            return Edge::comparatorX(e0, e1, nextRay);
            });
    }
}

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    return std::unique_ptr<GCanvas>(new MyCanvas(device));
}

std::string GDrawSomething(GCanvas* canvas, GISize dim) {
    GShader* shader;
    GPaint* paint = new GPaint();
    canvas->clear(GColor());

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float x, y, w, h;
    float a1, r1, g1, b1;
    float a2, r2, g2, b2;
    for(int i = 0; i < 1000; ++i) {
        a1 = dist(gen) * 0.5f; r1 = dist(gen); g1 = dist(gen); b1 = dist(gen);
        a2 = dist(gen) * 0.5f; r2 = dist(gen); g2 = dist(gen); b2 = dist(gen);
        const GColor c[2] = {{r1, g1, b1, a1}, {r2, g2, b2, a2}};
        x = dist(gen) * dim.width;
        y = dist(gen) * dim.height;
        w = dist(gen) * (dim.width - x);
        h = dist(gen) * (dim.height - y);
        shader = new MyGradient((GPoint) {x, y}, (GPoint) {x + w, y + h}, c, 2);
        paint->setShader(shader);
        canvas->drawRect(GRect::XYWH(x, y, w, h), *paint);
    }
    return "a bunch of gradients";
}
