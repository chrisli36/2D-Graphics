#ifndef BlendProc_DEFINED
#define BlendProc_DEFINED

#include "include/GPixel.h"
#include "include/GBitmap.h"
#include "Utility.h"

typedef const GPixel (*BlendProc)(GPixel&, GPixel&);

static const GPixel kClear(GPixel& src, GPixel& dst) { // 0
    return GPixel_PackARGB(0, 0, 0, 0);
    }
static const GPixel kSrc(GPixel& src, GPixel& dst) { // S
    return src;
    }
static const GPixel kDst(GPixel& src, GPixel& dst) { // D
    return dst;
    }
static const GPixel kSrcOver(GPixel& src, GPixel& dst) { // S + (1 - Sa)*D
    const int srcACmp = 255 - GPixel_GetA(src);
    return GPixel_PackARGB(GPixel_GetA(src) + GDiv255(srcACmp * GPixel_GetA(dst)),
                            GPixel_GetR(src) + GDiv255(srcACmp * GPixel_GetR(dst)),
                            GPixel_GetG(src) + GDiv255(srcACmp * GPixel_GetG(dst)),
                            GPixel_GetB(src) + GDiv255(srcACmp * GPixel_GetB(dst)));
    }
static const GPixel kDstOver(GPixel& src, GPixel& dst) { // D + (1 - Da)*S
    if(GPixel_GetA(dst) == 255) {
        return dst;
    }
    const int dstACmp = 255 - GPixel_GetA(dst);
    return GPixel_PackARGB(GPixel_GetA(dst) + GDiv255(dstACmp * GPixel_GetA(src)),
                            GPixel_GetR(dst) + GDiv255(dstACmp * GPixel_GetR(src)),
                            GPixel_GetG(dst) + GDiv255(dstACmp * GPixel_GetG(src)),
                            GPixel_GetB(dst) + GDiv255(dstACmp * GPixel_GetB(src)));
    }
static const GPixel kSrcIn(GPixel& src, GPixel& dst) { // Da * S
    if(GPixel_GetA(dst) == 0) {
        return GPixel_PackARGB(0, 0, 0, 0);
    } else if (GPixel_GetA(dst) == 255) {
        return src;
    }
    return GPixel_PackARGB(GDiv255(GPixel_GetA(dst) * GPixel_GetA(src)),
                            GDiv255(GPixel_GetA(dst) * GPixel_GetR(src)),
                            GDiv255(GPixel_GetA(dst) * GPixel_GetG(src)), 
                            GDiv255(GPixel_GetA(dst) * GPixel_GetB(src)));
    }
static const GPixel kDstIn(GPixel& src, GPixel& dst) { // Sa * D
    if(GPixel_GetA(src) == 0) {
        return GPixel_PackARGB(0, 0, 0, 0);
    }
    return GPixel_PackARGB(GDiv255(GPixel_GetA(src) * GPixel_GetA(dst)),
                            GDiv255(GPixel_GetA(src) * GPixel_GetR(dst)),
                            GDiv255(GPixel_GetA(src) * GPixel_GetG(dst)),
                            GDiv255(GPixel_GetA(src) * GPixel_GetB(dst)));
    }
static const GPixel kSrcOut(GPixel& src, GPixel& dst) { // (1 - Da)*S
    if(GPixel_GetA(dst) == 255) {
        return GPixel_PackARGB(0, 0, 0, 0);
    } else if(GPixel_GetA(dst) == 0) {
        return src;
    }
    const int dstACmp = 255 - GPixel_GetA(dst);
    return GPixel_PackARGB(GDiv255(dstACmp * GPixel_GetA(src)),
                            GDiv255(dstACmp * GPixel_GetR(src)),
                            GDiv255(dstACmp * GPixel_GetG(src)),
                            GDiv255(dstACmp * GPixel_GetB(src)));
    }
static const GPixel kDstOut(GPixel& src, GPixel& dst) { // (1 - Sa)*D
    if(GPixel_GetA(src) == 0) {
        return dst;
    }
    const int srcACmp = 255 - GPixel_GetA(src);
    return GPixel_PackARGB(GDiv255(srcACmp * GPixel_GetA(dst)),
                            GDiv255(srcACmp * GPixel_GetR(dst)),
                            GDiv255(srcACmp * GPixel_GetG(dst)),
                            GDiv255(srcACmp * GPixel_GetB(dst)));
}
static const GPixel kSrcATop(GPixel& src, GPixel& dst) { // Da*S + (1 - Sa)*D
    const int srcACmp = 255 - GPixel_GetA(src);
    const int dstA = GPixel_GetA(dst);
    if(dstA == 0) {
        return kDstOut(src, dst);
    } else if(dstA == 255) {
        return kSrcOver(src, dst);
    }
    return GPixel_PackARGB(GDiv255(dstA * GPixel_GetA(src) + srcACmp * GPixel_GetA(dst)),
                            GDiv255(dstA * GPixel_GetR(src)+ srcACmp * GPixel_GetR(dst)),
                            GDiv255(dstA * GPixel_GetG(src) + srcACmp * GPixel_GetG(dst)),
                            GDiv255(dstA * GPixel_GetB(src) + srcACmp * GPixel_GetB(dst)));
}
static const GPixel kDstATop(GPixel& src, GPixel& dst) { // Sa*D + (1 - Da)*S
    const int dstACmp = 255 - GPixel_GetA(dst);
    const int srcA = GPixel_GetA(src);
    if(srcA == 0) {
        return kSrcOut(src, dst);
    } else if(dstACmp == 0) {
        return kDstIn(src, dst);
    }
    return GPixel_PackARGB(GDiv255(srcA * GPixel_GetA(dst) + dstACmp * GPixel_GetA(src)),
                            GDiv255(srcA * GPixel_GetR(dst)+ dstACmp * GPixel_GetR(src)),
                            GDiv255(srcA * GPixel_GetG(dst) + dstACmp * GPixel_GetG(src)),
                            GDiv255(srcA * GPixel_GetB(dst) + dstACmp * GPixel_GetB(src)));
}
static const GPixel kXor(GPixel& src, GPixel& dst) { // (1 - Sa)*D + (1 - Da)*S
        const int srcACmp = 255 - GPixel_GetA(src);
        if(srcACmp == 255) {
            return kDstOver(src, dst);
        }
        const int dstACmp = 255 - GPixel_GetA(dst);
        if(dstACmp == 0) {
            return kDstOut(src, dst);
        } else if(dstACmp == 255) {
            return kSrcOver(src, dst);
        }
        return GPixel_PackARGB(GDiv255(srcACmp * GPixel_GetA(dst) + dstACmp * GPixel_GetA(src)),
                                GDiv255(srcACmp * GPixel_GetR(dst)+ dstACmp * GPixel_GetR(src)),
                                GDiv255(srcACmp * GPixel_GetG(dst) + dstACmp * GPixel_GetG(src)),
                                GDiv255(srcACmp * GPixel_GetB(dst) + dstACmp * GPixel_GetB(src)));
    }  

const BlendProc blendProcs[] = {
    kClear, 
    kSrc,
    kDst,
    kSrcOver,
    kDstOver,
    kSrcIn,
    kDstIn,
    kSrcOut,
    kDstOut,
    kSrcATop,
    kDstATop,
    kXor
};
const BlendProc blendProcsOne[] = {
    kClear,
    kSrc,
    kDst,
    kSrc,
    kDstOver,
    kSrcIn,
    kDst,
    kSrcOut,
    kClear,
    kSrcIn,
    kDstOver,
    kSrcOut,
};

typedef void (*BlendRowProc)(int, int, int, GPixel&, const GBitmap&);

static void kClear(int left, int right, int y, GPixel& src, const GBitmap& fDevice) { // 0
    for(int x = left; x < right; ++x) {
        *fDevice.getAddr(x, y) = GPixel_PackARGB(0, 0, 0, 0);
    }
}
static void kSrc(int left, int right, int y, GPixel& src, const GBitmap& fDevice) { // S
    for(int x = left; x < right; ++x) {
        *fDevice.getAddr(x, y) = src;
    }
}
static void kDst(int left, int right, int y, GPixel& src, const GBitmap& fDevice) { // D
    return;
}
static void kSrcOver(int left, int right, int y, GPixel& src, const GBitmap& fDevice) { // S + (1 - Sa)*D
    GPixel* dst;
    const int srcACmp = 255 - GPixel_GetA(src);
    for(int x = left; x < right; ++x) {
        dst = fDevice.getAddr(x, y);
        *dst = GPixel_PackARGB(GPixel_GetA(src) + GDiv255(srcACmp * GPixel_GetA(*dst)), 
                                GPixel_GetR(src) + GDiv255(srcACmp * GPixel_GetR(*dst)),
                                GPixel_GetG(src) + GDiv255(srcACmp * GPixel_GetG(*dst)),
                                GPixel_GetB(src) + GDiv255(srcACmp * GPixel_GetB(*dst)));
    }
}
static void kDstOver(int left, int right, int y, GPixel& src, const GBitmap& fDevice) { // D + (1 - Da)*S
    int dstACmp;
    GPixel* dst;
    for(int x = left; x < right; ++x) {
        dst = fDevice.getAddr(x, y);
        // if(GPixel_GetA(*dst) == 255) {
        // } else if(GPixel_GetA(*dst) == 0) {
        //     dst = fDevice.getAddr(x, y);
        //     *dst = GPixel_PackARGB(GPixel_GetA(src) + GPixel_GetA(*dst), 
        //                             GPixel_GetR(src) + GPixel_GetR(*dst),
        //                             GPixel_GetG(src) + GPixel_GetG(*dst),
        //                             GPixel_GetB(src) + GPixel_GetB(*dst));
        // } else {
        dstACmp = 255 - GPixel_GetA(*dst);
        *dst = GPixel_PackARGB(GPixel_GetA(*dst) + GDiv255(dstACmp * GPixel_GetA(src)),
                                GPixel_GetR(*dst) + GDiv255(dstACmp * GPixel_GetR(src)), 
                                GPixel_GetG(*dst) + GDiv255(dstACmp * GPixel_GetG(src)), 
                                GPixel_GetB(*dst) + GDiv255(dstACmp * GPixel_GetB(src)));
        // }
    }
}
static void kSrcIn(int left, int right, int y, GPixel& src, const GBitmap& fDevice) { // Da * S
    GPixel* dst;
    for(int x = left; x < right; ++x) {
        dst = fDevice.getAddr(x, y);
        // if(GPixel_GetA(*dst) == 0) {
        //     *dst = GPixel_PackARGB(0, 0, 0, 0);
        // } else if(GPixel_GetA(*dst) == 255) {
        //     *dst = src;
        // } else {
        *dst = GPixel_PackARGB(GDiv255(GPixel_GetA(*dst) * GPixel_GetA(src)), 
                                GDiv255(GPixel_GetA(*dst) * GPixel_GetR(src)), 
                                GDiv255(GPixel_GetA(*dst) * GPixel_GetG(src)), 
                                GDiv255(GPixel_GetA(*dst) * GPixel_GetB(src)));
        // }
    }
}
static void kDstIn(int left, int right, int y, GPixel& src, const GBitmap& fDevice) { // Sa * D
    GPixel* dst;
    for(int x = left; x < right; ++x) {
        dst = fDevice.getAddr(x, y);
        *dst = GPixel_PackARGB(GDiv255(GPixel_GetA(src) * GPixel_GetA(*dst)), 
                                GDiv255(GPixel_GetA(src) * GPixel_GetR(*dst)),
                                GDiv255(GPixel_GetA(src) * GPixel_GetG(*dst)),
                                GDiv255(GPixel_GetA(src) * GPixel_GetB(*dst)));
    }
}
static void kSrcOut(int left, int right, int y, GPixel& src, const GBitmap& fDevice) { // (1 - Da)*S
    int dstACmp;
    GPixel* dst;
    for(int x = left; x < right; ++x) {
        dst = fDevice.getAddr(x, y);
        // if(GPixel_GetA(*dst) == 255) {
        //     *dst = GPixel_PackARGB(0, 0, 0, 0);
        // } else if(GPixel_GetA(*dst) == 0){
        //     *dst = src;
        // } else {
            dstACmp = 255 - GPixel_GetA(*dst);
            *dst = GPixel_PackARGB(GDiv255(dstACmp * GPixel_GetA(src)), 
                                    GDiv255(dstACmp * GPixel_GetR(src)), 
                                    GDiv255(dstACmp * GPixel_GetG(src)), 
                                    GDiv255(dstACmp * GPixel_GetB(src)));
        // }
    }
}
static void kDstOut(int left, int right, int y, GPixel& src, const GBitmap& fDevice) {  // (1 - Sa)*D
    GPixel* dst;
    const int srcACmp = 255 - GPixel_GetA(src);
    for(int x = left; x < right; ++x) {
        dst = fDevice.getAddr(x, y);
        *dst = GPixel_PackARGB(GDiv255(srcACmp * GPixel_GetA(*dst)), 
                                GDiv255(srcACmp * GPixel_GetR(*dst)), 
                                GDiv255(srcACmp * GPixel_GetG(*dst)), 
                                GDiv255(srcACmp * GPixel_GetB(*dst)));
    }
}
static void kSrcATop(int left, int right, int y, GPixel& src, const GBitmap& fDevice) { // Da*S + (1 - Sa)*D
    GPixel* dst;
    const int srcACmp = 255 - GPixel_GetA(src);
    int dstA;
    for(int x = left; x < right; ++x) {
        dst = fDevice.getAddr(x, y);
        dstA = GPixel_GetA(*dst);
        // if(dstA == 0) {
        //     *dst = GPixel_PackARGB(GDiv255(srcACmp * GPixel_GetA(*dst)), 
        //                             GDiv255(srcACmp * GPixel_GetR(*dst)), 
        //                             GDiv255(srcACmp * GPixel_GetG(*dst)), 
        //                             GDiv255(srcACmp * GPixel_GetB(*dst)));
        // } else {
            *dst = GPixel_PackARGB(GDiv255(dstA * GPixel_GetA(src) + srcACmp * GPixel_GetA(*dst)), 
                                    GDiv255(dstA * GPixel_GetR(src)+ srcACmp * GPixel_GetR(*dst)), 
                                    GDiv255(dstA * GPixel_GetG(src) + srcACmp * GPixel_GetG(*dst)), 
                                    GDiv255(dstA * GPixel_GetB(src) + srcACmp * GPixel_GetB(*dst)));
        // }
    }
}
static void kDstATop(int left, int right, int y, GPixel& src, const GBitmap& fDevice) { // Sa*D + (1 - Da)*S
    GPixel* dst;
    const int srcA = GPixel_GetA(src);
    int dstACmp;
    for(int x = left; x < right; ++x) {
        dst = fDevice.getAddr(x, y);
        dstACmp = 255 - GPixel_GetA(*dst);
        if(dstACmp == 0) {
            *dst = GPixel_PackARGB(GDiv255(GPixel_GetA(src) * GPixel_GetA(*dst)), 
                                    GDiv255(GPixel_GetA(src) * GPixel_GetR(*dst)), 
                                    GDiv255(GPixel_GetA(src) * GPixel_GetG(*dst)), 
                                    GDiv255(GPixel_GetA(src) * GPixel_GetB(*dst)));
        } else {
            *dst = GPixel_PackARGB(GDiv255(srcA * GPixel_GetA(*dst) + dstACmp * GPixel_GetA(src)), 
                                    GDiv255(srcA * GPixel_GetR(*dst)+ dstACmp * GPixel_GetR(src)), 
                                    GDiv255(srcA * GPixel_GetG(*dst) + dstACmp * GPixel_GetG(src)), 
                                    GDiv255(srcA * GPixel_GetB(*dst) + dstACmp * GPixel_GetB(src)));
        }
    }
}
static void kXor(int left, int right, int y, GPixel& src, const GBitmap& fDevice) { // (1 - Sa)*D + (1 - Da)*S
    GPixel* dst;
    int dstACmp;
    const int srcACmp = 255 - GPixel_GetA(src);
    for(int x = left; x < right; ++x) {
        dst = fDevice.getAddr(x, y);
        dstACmp = 255 - GPixel_GetA(*dst);
        *dst = GPixel_PackARGB(GDiv255(srcACmp * GPixel_GetA(*dst) + dstACmp * GPixel_GetA(src)), 
                                GDiv255(srcACmp * GPixel_GetR(*dst)+ dstACmp * GPixel_GetR(src)), 
                                GDiv255(srcACmp * GPixel_GetG(*dst) + dstACmp * GPixel_GetG(src)), 
                                GDiv255(srcACmp * GPixel_GetB(*dst) + dstACmp * GPixel_GetB(src)));
    }
}

const BlendRowProc blendRowProcs[] = {
    kClear, 
    kSrc,
    kDst,
    kSrcOver,
    kDstOver,
    kSrcIn,
    kDstIn,
    kSrcOut,
    kDstOut,
    kSrcATop,
    kDstATop,
    kXor
};
const BlendRowProc blendRowProcsZero[] = {
    kClear,
    kClear,
    kDst,
    kDst,
    kDst,
    kClear,
    kClear,
    kClear,
    kDst,
    kDst,
    kClear,
    kDst,
};
const BlendRowProc blendRowProcsOne[] = {
    kClear,
    kSrc,
    kDst,
    kSrc,
    kDstOver,
    kSrcIn,
    kDst,
    kSrcOut,
    kClear,
    kSrcIn,
    kDstOver,
    kSrcOut,
};

#endif