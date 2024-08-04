/*
 *  Copyright 2024 <Christopher Li>
 */

#ifndef _g_starter_canvas_h_
#define _g_starter_canvas_h_

#include "include/GCanvas.h"
#include "include/GRect.h"
#include "include/GColor.h"
#include "include/GBitmap.h"

#include "Edge.h"
#include "BlendProc.h"

class MyCanvas : public GCanvas {
public:
    MyCanvas(const GBitmap& device) : fDevice(device) {}

    void clear(const GColor& color) override;
    void drawRect(const GRect& rect, const GPaint& paint) override;
    void drawConvexPolygon(const GPoint[], int count, const GPaint&) override;

private:
    static inline const GIRect intersect(const GIRect& rect, const int w, const int h) {
        return GIRect::LTRB(std::max(rect.left, 0), std::max(rect.top, 0), std::min(rect.right, w), std::min(rect.bottom, h));
    }
    static inline const GColor premultiply(const GColor& color) {
        return GColor {color.a * color.r, color.a * color.g, color.a * color.b, color.a};
    }
    static inline const GPixel scale(const GColor& color) {
        return GPixel_PackARGB(GRoundToInt(color.a * 255), GRoundToInt(color.r * 255), GRoundToInt(color.g * 255), GRoundToInt(color.b * 255));
    }
    static inline const GPixel premultiplyAndScale(const GColor& color) {
        return GPixel_PackARGB(GRoundToInt(color.a * 255), 
                                GRoundToInt(color.r * color.a * 255), 
                                GRoundToInt(color.g * color.a * 255), 
                                GRoundToInt(color.b * color.a * 255));
    }
    static inline uint32_t GDiv255(unsigned prod) {
        return (prod + 128) * 257 >> 16;
    }

    template <class T> void bitRow(int l, int r, int y, GPixel& src, T blend);
    void clipEdge(GPoint p0, GPoint p1, std::vector<Edge*>* edges);

    const GBitmap fDevice;
};

#endif
