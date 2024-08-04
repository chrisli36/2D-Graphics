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
    // if the shader exists, then use the shader to blitRow
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
    // otherwise, use the rowProc to do it
    rowProc(l, r, y, src, this->fDevice);
}

void MyCanvas::drawConvexPolygon(const GPoint points[], int count, const GPaint& paint) {
    // get color, blendMode, shader
    const GColor& color = paint.getColor();
    const GBlendMode blendMode = paint.getBlendMode();
    GShader* shader = paint.getShader();
    // if the blendmode is just dst, don't draw
    if(blendMode == GBlendMode::kDst) {return;}
    // if shader exists, set context
    if(shader != nullptr) {
        shader->setContext(stack.back());
    }
    // get the src color, get single color proc or entire row proc
    GPixel src = premultiplyScale(color);
    const BlendProc proc = getBlendProcs(shader)[(int) blendMode];
    BlendRowProc rowProc = getBlendRowProcs(src)[(int) blendMode];
    // if the blendProc returned is dst or the rowProc is dst, don't draw
    if((shader != nullptr && proc == (BlendProc) kDst) || (shader == nullptr && rowProc == (BlendRowProc) kDst)) {
        return;
    }
    // if the single color proc is clear, then shader is opaque, so use faster row proc to clear
    if(proc == (BlendProc) kClear) {
        shader = nullptr;
        rowProc = (BlendRowProc) kClear;
    }
    
    // map points with the matrix
    GPoint newPoints[count];
    stack.back().mapPoints(newPoints, points, count);
    // clip all the edges
    std::vector<Edge>* edgeStorage = new std::vector<Edge>;
    for(int i = 0; i < count - 1; ++i) {
        clipEdge(newPoints[i], newPoints[i + 1], edgeStorage);
    } 
    clipEdge(newPoints[count - 1], newPoints[0], edgeStorage);
    const int numEdges = edgeStorage->size();
    // if edges don't make a polygon don't draw
    if(numEdges < 2) {
        return;
    }
    // make an array of pointers pointing to the edges in storage
    Edge* edgePtrs[numEdges];
    for(int i = 0; i < edgeStorage->size(); ++i) {
        edgePtrs[i] = &edgeStorage->at(i);
    }
    std::sort(edgePtrs, edgePtrs + numEdges, Edge::comparatorY); // sort the edges by top y values

    // pointers pointing to 2 edges at a time, draw ray going through both edges
    int ptr1 = 0; int ptr2 = 1;
    int currY = edgePtrs[0]->top;
    float currRay = currY + 0.5f;
    int left, right;
    while(ptr1 < numEdges && ptr2 < numEdges) {
        // calculate the x values where the ray intersects at y=currRay
        left = GRoundToInt(edgePtrs[ptr1]->eval(currRay));
        right = GRoundToInt(edgePtrs[ptr2]->eval(currRay));
        if(left > right) { // need to swap the x values if not in order from left to right
            std::swap(left, right);
        }

        blitRow(left, right, currY, src, proc, rowProc, shader); // draw the row

        // move currRay down, if a pointer no longer valid, get next pointer
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
    // get color, blendMode, shader
    const GColor& color = paint.getColor();
    const GBlendMode blendMode = paint.getBlendMode();
    GShader* shader = paint.getShader();
    // if the blendmode is just dst, don't draw
    if(blendMode == GBlendMode::kDst) {return;}
    // if shader exists, set context
    if(shader != nullptr) {
        shader->setContext(stack.back());
    }
    // get the src color, get single color proc or entire row proc
    GPixel src = premultiplyScale(color);
    const BlendProc proc = getBlendProcs(shader)[(int) blendMode];
    BlendRowProc rowProc = getBlendRowProcs(src)[(int) blendMode];
    // if the blendProc returned is dst or the rowProc is dst, don't draw
    if((shader != nullptr && proc == (BlendProc) kDst) || (shader == nullptr && rowProc == (BlendRowProc) kDst)) {
        return;
    }
    // if the single color proc is clear, then shader is opaque, so use faster row proc to clear
    if(proc == (BlendProc) kClear) {
        shader = nullptr;
        rowProc = (BlendRowProc) kClear;
    }

    // map path with the matrix
    GPath newPath = path;
    newPath.transform(stack.back());

    // generate edges from path
    std::vector<Edge>* edgeStorage = new std::vector<Edge>;
    GPoint edge[GPath::kMaxNextPoints];
    GPath::Edger edger(newPath);
    std::optional<GPath::Verb> v;
    float numSegs;
    GPoint prevPoint, nextPoint;
    GVector E0, E1;
    const float tolerance = 0.25f; // arbitrary precision value
    while((v = edger.next(edge)).has_value()) {
        switch(v.value()) {
            case GPath::kMove:
                break;
            case GPath::kLine: // just a normal edge
                clipEdge(edge[0], edge[1], edgeStorage);
                break;
            case GPath::kQuad: 
                // calculate number of segments needed based on tolerance
                E0 = (edge[0] - (2 * edge[1]) + edge[2]) * 0.25f;
                numSegs = GCeilToInt(std::sqrt(magnitude(E0) / tolerance)); 
                // compute the points on quad and clip edges
                prevPoint = computeQuad(0.0f, edge[0], edge[1], edge[2]);
                for(int seg = 0; seg < numSegs; ++seg) {
                    nextPoint = computeQuad((seg + 1) / numSegs, edge[0], edge[1], edge[2]);
                    clipEdge(prevPoint, nextPoint, edgeStorage);
                    prevPoint = nextPoint;
                }
                break;
            case GPath::kCubic:
                // calculate number of segments needed based on tolerance
                E0 = edge[0] - (2 * edge[1]) + edge[2];
                E1 = edge[1] - (2 * edge[2]) + edge[3];
                E0 = {std::max(std::abs(E0.x), std::abs(E1.x)), std::max(std::abs(E0.y), std::abs(E1.y))};
                numSegs = GCeilToInt(std::sqrt(0.75f * magnitude(E0) / tolerance));
                // compute the points on cubic and clip edges
                prevPoint = computeCubic(0.0f, edge[0], edge[1], edge[2], edge[3]);
                for(int seg = 0; seg < numSegs; ++seg) {
                    nextPoint = computeCubic((seg + 1) / numSegs, edge[0], edge[1], edge[2], edge[3]);
                    clipEdge(prevPoint, nextPoint, edgeStorage);
                    prevPoint = nextPoint;
                }
                break;
        }
    }
    // if edges don't make a polygon, don't draw
    const int numEdges = edgeStorage->size();
    if(numEdges < 2) {
        return;
    }
    // make a array of edge pointers pointing to edges in storage
    Edge* edgePtrs[numEdges];
    for(int i = 0; i < edgeStorage->size(); ++i) {
        edgePtrs[i] = &edgeStorage->at(i);
    }
    // sort edges by top y, and then by top x value
    std::sort(edgePtrs, edgePtrs + numEdges, Edge::comparatorYX); 

    GIRect boundingRect = newPath.bounds().roundOut(); // get bounds of the path
    int start = 0, i, winding, L;
    float currRay, nextRay = boundingRect.top + 0.5f;
    int x;
    for(int y = boundingRect.top; y < boundingRect.bottom; ++y) {
        i = start; winding = 0; currRay = nextRay; nextRay = y + 1.5f;
        while(i < numEdges && edgePtrs[i]->isValid(currRay)) {
            x = GRoundToInt(edgePtrs[i]->eval(currRay));
            // if winding is 0, start left at current value of x
            if(winding == 0) {
                L = x;
            }
            winding += edgePtrs[i]->w; // update winding
            // if winding is 0, then we are inside the polygon
            if(winding == 0) { 
                blitRow(L, x, y, src, proc, rowProc, shader);
            }
            // "delete" the edge by swapping it with the start of array and iterating start
            if(!edgePtrs[i]->isValid(nextRay)) {
                std::swap(edgePtrs[i], edgePtrs[start]);
                ++start;
            }
            ++i; // move onto the next edge hitting the ray
        }
        assert(winding == 0);
        // include all possible valid edges
        while(i < numEdges && edgePtrs[i]->isValid(nextRay)) {
            ++i;
        }
        // sort all the edges that will hit the next ray
        std::sort(edgePtrs + start, edgePtrs + i, [nextRay](Edge* e0, Edge* e1) {
            return Edge::comparatorX(e0, e1, nextRay);
            });
    }
}

void MyCanvas::drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[], int count, const int indices[], const GPaint& paint) {
    GPoint pts[3];
    GPaint temp;
    GColor c[3]; MyTriShader* triShader;
    MyProxyShader* proxyShader;
    MyComposeShader* composeShader;
    GMatrix P, T, invT;
    if(colors && !texs) { // for each triangle, make a triShader with the points and colors, then draw triangle
        for (int i = 0, n = 0; i < count; ++i, n+=3) {
            pts[0] = verts[indices[n]]; pts[1] = verts[indices[n + 1]]; pts[2] = verts[indices[n + 2]];
            c[0] = colors[indices[n]]; c[1] = colors[indices[n + 1]]; c[2] = colors[indices[n + 2]];

            triShader = new MyTriShader(pts, c);
            temp.setShader(triShader);
            drawConvexPolygon(pts, 3, temp);
        }
    } else if(texs && !colors) { // for each triangle, make a proxyShader with the points and texture, then draw triangle
        for (int i = 0, n = 0; i < count; ++i, n+=3) {
            pts[0] = verts[indices[n]]; pts[1] = verts[indices[n + 1]]; pts[2] = verts[indices[n + 2]];

            P = computeBasis(pts[0], pts[1], pts[2]);
            invT = *(computeBasis(texs[indices[n]], texs[indices[n + 1]], texs[indices[n + 2]]).invert());
            proxyShader = new MyProxyShader(paint.getShader(), P * invT);
            temp.setShader(proxyShader);
            drawConvexPolygon(pts, 3, temp);
        }
    } else if(colors && texs) { // for each triangle, make a triShader and proxyShader, then draw the triangle
        for (int i = 0, n = 0; i < count; ++i, n+=3) {
            pts[0] = verts[indices[n]]; pts[1] = verts[indices[n + 1]]; pts[2] = verts[indices[n + 2]];
            c[0] = colors[indices[n]]; c[1] = colors[indices[n + 1]]; c[2] = colors[indices[n + 2]];

            P = computeBasis(pts[0], pts[1], pts[2]);
            invT = *(computeBasis(texs[indices[n]], texs[indices[n + 1]], texs[indices[n + 2]]).invert());

            triShader = new MyTriShader(pts, c);
            proxyShader = new MyProxyShader(paint.getShader(), P * invT);
            composeShader = new MyComposeShader(triShader, proxyShader);

            temp.setShader(composeShader);
            drawConvexPolygon(pts, 3, temp);
        }
    }
}

void MyCanvas::drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint& paint) {
    const int len = level + 1;
    float u, v;
    const float lenFloat = (float) len;
    const int numPts = (len + 1) * (len + 1);

    int indices[len * len * 6];
    int currIndex;
    int currIndexIndex;

    GPoint vertPts[numPts];
    GColor c[numPts];
    GPoint texPts[numPts];

    if(colors && !texs) {
        for(int j = 0; j < len + 1; ++j) {
            v = (float) j / lenFloat;
            for(int i = 0; i < len + 1; ++i) {
                u = (float) i / lenFloat;
                currIndex = (len + 1) * j + i;

                vertPts[currIndex] = quadrulate(u, v, verts);
                c[currIndex] = quadrulate(u, v, colors);

                if(j == len || i == len) {
                    continue;
                }
                currIndexIndex = (len * j + i) * 6;
                indices[currIndexIndex] = currIndex;
                indices[currIndexIndex + 1] = currIndex + 1;
                indices[currIndexIndex + 2] = currIndex + len + 1;
                indices[currIndexIndex + 3] = currIndex + 1;
                indices[currIndexIndex + 4] = currIndex + len + 1;
                indices[currIndexIndex + 5] = currIndex + len + 2;
            }
        }
        drawMesh(vertPts, c, nullptr, len * len * 2, indices, paint);
    } else if(texs && !colors) {
        for(int j = 0; j < len + 1; ++j) {
            v = (float) j / lenFloat;
            for(int i = 0; i < len + 1; ++i) {
                u = (float) i / lenFloat;
                currIndex = (len + 1) * j + i;
                
                vertPts[currIndex] = quadrulate(u, v, verts);
                texPts[currIndex] = quadrulate(u, v, texs);

                if(j == len || i == len) {
                    continue;
                }
                currIndexIndex = (len * j + i) * 6;
                indices[currIndexIndex] = currIndex;
                indices[currIndexIndex + 1] = currIndex + 1;
                indices[currIndexIndex + 2] = currIndex + len + 1;
                indices[currIndexIndex + 3] = currIndex + 1;
                indices[currIndexIndex + 4] = currIndex + len + 1;
                indices[currIndexIndex + 5] = currIndex + len + 2;
            }
        }
        drawMesh(vertPts, nullptr, texPts, len * len * 2, indices, paint);
    } else if(texs && colors) {
        for(int j = 0; j < len + 1; ++j) {
            v = (float) j / lenFloat;
            for(int i = 0; i < len + 1; ++i) {
                u = (float) i / lenFloat;
                currIndex = (len + 1) * j + i;
                
                vertPts[currIndex] = quadrulate(u, v, verts);
                c[currIndex] = quadrulate(u, v, colors);
                texPts[currIndex] = quadrulate(u, v, texs);

                if(j == len || i == len) {
                    continue;
                }
                currIndexIndex = (len * j + i) * 6;
                indices[currIndexIndex] = currIndex;
                indices[currIndexIndex + 1] = currIndex + 1;
                indices[currIndexIndex + 2] = currIndex + len + 1;
                indices[currIndexIndex + 3] = currIndex + 1;
                indices[currIndexIndex + 4] = currIndex + len + 1;
                indices[currIndexIndex + 5] = currIndex + len + 2;
            }
        }
        drawMesh(vertPts, c, texPts, len * len * 2, indices, paint);
    }
    
}

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    return std::unique_ptr<GCanvas>(new MyCanvas(device));
}

std::string GDrawSomething(GCanvas* canvas, GISize dim) {
    canvas->clear({0.8f, 0.9f, 1.f, 1.f});
    
    GBitmap bm;
    bm.readFromFile("apps/wood2.png");
    std::unique_ptr<GShader> shader = GCreateBitmapShader(bm, GMatrix::Rotate(1.5708f), GTileMode::kMirror);
    GPaint* paint = new GPaint(shader.get());

    GPoint verts[4] = {{20.f, 20.f}, {200.f, 10.f}, {20.f, 250.f}, {150.f, 200.f}};
    GPoint texs[4] = {{0.f, 0.f}, {(float) bm.width(), 0.f}, {(float) bm.width(), (float) bm.height()}, {0.f, (float) bm.height()}};

    canvas->drawQuad(verts, nullptr, texs, 12, *paint);
    
    return "twisty wood";
}
