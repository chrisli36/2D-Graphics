/*
 *  Copyright 2024 <Christopher Li>
 */

#ifndef _g_starter_canvas_h_
#define _g_starter_canvas_h_

#include <vector>

#include "include/GCanvas.h"
#include "include/GRect.h"
#include "include/GColor.h"
#include "include/GBitmap.h"
#include "include/GShader.h"
#include "include/GPath.h"

#include "MyEdge.h"
#include "MyBlendProc.h"
#include "MyBitmapShader.h"
#include "MyGradient.h"
#include "MyTriShader.h"
#include "MyProxyShader.h"
#include "MyComposeShader.h"
#include "Utility.h"

#include <iostream>
#include <random>

class MyCanvas : public GCanvas {
    public:
        MyCanvas(const GBitmap& device) : fDevice(device) {
            stack.push_back(GMatrix());
        }

        const GBitmap getBitmap() {return fDevice;}

        void clear(const GColor& color) override;
        void drawRect(const GRect& rect, const GPaint& paint) override;
        void drawConvexPolygon(const GPoint[], int count, const GPaint&) override;
        void drawPath(const GPath&, const GPaint&) override;
        void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[], 
                      int count, const int indices[], const GPaint&) override;
        virtual void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
                              int level, const GPaint&) override;

        void save() override;
        void restore() override;
        void concat(const GMatrix& matrix) override;

    private:
        static inline const GIRect intersect(const GIRect& rect, const int w, const int h) {
            return GIRect::LTRB(std::max(rect.left, 0), std::max(rect.top, 0), std::min(rect.right, w), std::min(rect.bottom, h));
            }

        static const BlendRowProc* getBlendRowProcs(GPixel src) {
            if(GPixel_GetA(src) == 0) {
                return blendRowProcsZero;
            } else if(GPixel_GetA(src) == 255) {
                return blendRowProcsOne;
            }
            return blendRowProcs;
        };
        static const BlendProc* getBlendProcs(GShader* shader) {
            if(shader != nullptr && shader->isOpaque()) {
                return blendProcsOne;
            }
            return blendProcs;
        };

        template <class T, class S> void blitRow(int l, int r, int y, GPixel& src, T blend, S blendRow, GShader* shader);
        void clipEdge(GPoint p0, GPoint p1, std::vector<Edge>* edges);
        const GBitmap fDevice;
        std::vector<GMatrix> stack;
};

#endif
