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

#include "Edge.h"
#include "BlendProc.h"
#include "MyShader.h"

#include <iostream>

class MyCanvas : public GCanvas {
    public:
        MyCanvas(const GBitmap& device) : fDevice(device) {
            stack.push_back(GMatrix());
        }

        const GBitmap getBitmap() {return fDevice;}

        void clear(const GColor& color) override;
        void drawRect(const GRect& rect, const GPaint& paint) override;
        void drawConvexPolygon(const GPoint[], int count, const GPaint&) override;

        void save() override;
        void restore() override;
        void concat(const GMatrix& matrix) override;

    private:
        static inline const GIRect intersect(const GIRect& rect, const int w, const int h) {
            return GIRect::LTRB(std::max(rect.left, 0), std::max(rect.top, 0), std::min(rect.right, w), std::min(rect.bottom, h));
        }
        static inline const GPixel premultiplyAndScale(const GColor& color) {
            return GPixel_PackARGB(GRoundToInt(color.a * 255), 
                                    GRoundToInt(color.r * color.a * 255), 
                                    GRoundToInt(color.g * color.a * 255), 
                                    GRoundToInt(color.b * color.a * 255));
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

        template <class T> void blitRow(int l, int r, int y, GPixel& src, T blend);
        template <class T> void blitRowShade(int l, int r, int y, GPixel& src, T blend, GShader* shader);
        void clipEdge(GPoint p0, GPoint p1, std::vector<Edge*>* edges);

        const GBitmap fDevice;
        std::vector<GMatrix> stack;
};

#endif
