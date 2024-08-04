#ifndef BlendProc_DEFINED
#define BlendProc_DEFINED

#include "include/GPixel.h"

typedef void (*BlendProc)(int, int, int, GPixel&, const GBitmap&);

static inline uint32_t GDiv255(unsigned prod) {
    return (prod + 128) * 257 >> 16;
}
static void kClear(int left, int right, int y, GPixel& src, const GBitmap& fDevice) {
    for(int x = left; x < right; ++x) {
        GPixel* dst = fDevice.getAddr(x, y);
        *dst = GPixel_PackARGB(0, 0, 0, 0);
    }
}
static void kSrc(int left, int right, int y, GPixel& src, const GBitmap& fDevice) {
    for(int x = left; x < right; ++x) {
        GPixel* dst = fDevice.getAddr(x, y);
        *dst = src;
    }
}
static void kDst(int left, int right, int y, GPixel& src, const GBitmap& fDevice) {
    return;
}
static void kSrcOver(int left, int right, int y, GPixel& src, const GBitmap& fDevice) {
    if(GPixel_GetA(src) == 255) {
        for(int x = left; x < right; ++x) {
            GPixel* dst = fDevice.getAddr(x, y);
            *dst = src;
        }
        return;
    } else if(GPixel_GetA(src) == 0) {
        for(int x = left; x < right; ++x) {
            GPixel* dst = fDevice.getAddr(x, y);
            *dst = GPixel_PackARGB(GPixel_GetA(src) + GPixel_GetA(*dst), 
                                    GPixel_GetR(src) + GPixel_GetR(*dst),
                                    GPixel_GetG(src) + GPixel_GetG(*dst),
                                    GPixel_GetB(src) + GPixel_GetB(*dst));
        }
        return;
    }
    int srcACmp = 255 - GPixel_GetA(src);
    for(int x = left; x < right; ++x) {
        GPixel* dst = fDevice.getAddr(x, y);
        *dst = GPixel_PackARGB(GPixel_GetA(src) + GDiv255(srcACmp * GPixel_GetA(*dst)), 
                                GPixel_GetR(src) + GDiv255(srcACmp * GPixel_GetR(*dst)),
                                GPixel_GetG(src) + GDiv255(srcACmp * GPixel_GetG(*dst)),
                                GPixel_GetB(src) + GDiv255(srcACmp * GPixel_GetB(*dst)));
    }
}
static void kDstOver(int left, int right, int y, GPixel& src, const GBitmap& fDevice) {
    int dstACmp;
    for(int x = left; x < right; ++x) {
        GPixel* dst = fDevice.getAddr(x, y);
        if(GPixel_GetA(*dst) == 255) {
            continue;
        } else if(GPixel_GetA(*dst) == 0) {
            GPixel* dst = fDevice.getAddr(x, y);
            *dst = GPixel_PackARGB(GPixel_GetA(src) + GPixel_GetA(*dst), 
                                    GPixel_GetR(src) + GPixel_GetR(*dst),
                                    GPixel_GetG(src) + GPixel_GetG(*dst),
                                    GPixel_GetB(src) + GPixel_GetB(*dst));
            continue;
        }
        dstACmp = 255 - GPixel_GetA(*dst);
        *dst = GPixel_PackARGB(GPixel_GetA(*dst) + GDiv255(dstACmp * GPixel_GetA(src)),
                                GPixel_GetR(*dst) + GDiv255(dstACmp * GPixel_GetR(src)), 
                                GPixel_GetG(*dst) + GDiv255(dstACmp * GPixel_GetG(src)), 
                                GPixel_GetB(*dst) + GDiv255(dstACmp * GPixel_GetB(src)));
    }
}
static void kSrcIn(int left, int right, int y, GPixel& src, const GBitmap& fDevice) {
    for(int x = left; x < right; ++x) {
        GPixel* dst = fDevice.getAddr(x, y);
        if(GPixel_GetA(*dst) == 0) {
            *dst = GPixel_PackARGB(0, 0, 0, 0);
            continue;
        } else if(GPixel_GetA(*dst) == 255) {
            *dst = src;
            continue;
        }
        *dst = GPixel_PackARGB(GDiv255(GPixel_GetA(*dst) * GPixel_GetA(src)), 
                                GDiv255(GPixel_GetA(*dst) * GPixel_GetR(src)), 
                                GDiv255(GPixel_GetA(*dst) * GPixel_GetG(src)), 
                                GDiv255(GPixel_GetA(*dst) * GPixel_GetB(src)));
    }
}
static void kDstIn(int left, int right, int y, GPixel& src, const GBitmap& fDevice) {
    if(GPixel_GetA(src) == 0) {
        for(int x = left; x < right; ++x) {
            GPixel* dst = fDevice.getAddr(x, y);
            *dst = GPixel_PackARGB(0, 0, 0, 0);
        }
        return;
    } else if(GPixel_GetA(src) == 255) {
        return;
    }
    for(int x = left; x < right; ++x) {
        GPixel* dst = fDevice.getAddr(x, y);
        *dst = GPixel_PackARGB(GDiv255(GPixel_GetA(src) * GPixel_GetA(*dst)), 
                GDiv255(GPixel_GetA(src) * GPixel_GetR(*dst)),
                GDiv255(GPixel_GetA(src) * GPixel_GetG(*dst)),
                GDiv255(GPixel_GetA(src) * GPixel_GetB(*dst)));
    }
}
static void kSrcOut(int left, int right, int y, GPixel& src, const GBitmap& fDevice) {
    int dstACmp;
    for(int x = left; x < right; ++x) {
        GPixel* dst = fDevice.getAddr(x, y);
        if(GPixel_GetA(*dst) == 255) {
            *dst = GPixel_PackARGB(0, 0, 0, 0);
        } else if(GPixel_GetA(*dst) == 0){
            *dst = src;
        }else {
            dstACmp = 255 - GPixel_GetA(*dst);
            *dst = GPixel_PackARGB(GDiv255(dstACmp * GPixel_GetA(src)), 
                                    GDiv255(dstACmp * GPixel_GetR(src)), 
                                    GDiv255(dstACmp * GPixel_GetG(src)), 
                                    GDiv255(dstACmp * GPixel_GetB(src)));
        }
    }
}
static void kDstOut(int left, int right, int y, GPixel& src, const GBitmap& fDevice) {
    if(GPixel_GetA(src) == 255) {
        for(int x = left; x < right; ++x) {
            GPixel* dst = fDevice.getAddr(x, y);
            *dst = GPixel_PackARGB(0, 0, 0, 0);
        }
        return;
    } else if(GPixel_GetA(src) == 0) {
        return;
    }
    int srcACmp = 255 - GPixel_GetA(src);
    for(int x = left; x < right; ++x) {
        GPixel* dst = fDevice.getAddr(x, y);
        *dst = GPixel_PackARGB(GDiv255(srcACmp * GPixel_GetA(*dst)), 
                GDiv255(srcACmp * GPixel_GetR(*dst)), 
                GDiv255(srcACmp * GPixel_GetG(*dst)), 
                GDiv255(srcACmp * GPixel_GetB(*dst)));
    }
}
static void kSrcATop(int left, int right, int y, GPixel& src, const GBitmap& fDevice) {
    if(GPixel_GetA(src) == 255) {
        for(int x = left; x < right; ++x) {
            GPixel* dst = fDevice.getAddr(x, y);
            if(GPixel_GetA(*dst) == 0) {
                *dst = GPixel_PackARGB(0, 0, 0, 0);
                continue;
            } else if(GPixel_GetA(*dst) == 255) {
                *dst = src;
                continue;
            }
            *dst = GPixel_PackARGB(GDiv255(GPixel_GetA(*dst) * GPixel_GetA(src)), 
                                    GDiv255(GPixel_GetA(*dst) * GPixel_GetR(src)), 
                                    GDiv255(GPixel_GetA(*dst) * GPixel_GetG(src)), 
                                    GDiv255(GPixel_GetA(*dst) * GPixel_GetB(src)));
        }
        return;
    }
    const int srcACmp = 255 - GPixel_GetA(src);
    int dstA;
    for(int x = left; x < right; ++x) {
        GPixel* dst = fDevice.getAddr(x, y);
        dstA = GPixel_GetA(*dst);
        if(dstA == 0) {
            *dst = GPixel_PackARGB(GDiv255(srcACmp * GPixel_GetA(*dst)), 
                                    GDiv255(srcACmp * GPixel_GetR(*dst)), 
                                    GDiv255(srcACmp * GPixel_GetG(*dst)), 
                                    GDiv255(srcACmp * GPixel_GetB(*dst)));
        } else {
            *dst = GPixel_PackARGB(GDiv255(dstA * GPixel_GetA(src) + srcACmp * GPixel_GetA(*dst)), 
                                    GDiv255(dstA * GPixel_GetR(src)+ srcACmp * GPixel_GetR(*dst)), 
                                    GDiv255(dstA * GPixel_GetG(src) + srcACmp * GPixel_GetG(*dst)), 
                                    GDiv255(dstA * GPixel_GetB(src) + srcACmp * GPixel_GetB(*dst)));
        }
    }
}
static void kDstATop(int left, int right, int y, GPixel& src, const GBitmap& fDevice) {
    const int srcA = GPixel_GetA(src);
    int dstACmp;
    if(srcA == 0) {
        int dstACmp;
        for(int x = left; x < right; ++x) {
            GPixel* dst = fDevice.getAddr(x, y);
            if(GPixel_GetA(*dst) == 255) {
                *dst = GPixel_PackARGB(0, 0, 0, 0);
            } else if(GPixel_GetA(*dst) == 0){
                *dst = src;
            }else {
                dstACmp = 255 - GPixel_GetA(*dst);
                *dst = GPixel_PackARGB(GDiv255(dstACmp * GPixel_GetA(src)), 
                                        GDiv255(dstACmp * GPixel_GetR(src)), 
                                        GDiv255(dstACmp * GPixel_GetG(src)), 
                                        GDiv255(dstACmp * GPixel_GetB(src)));
            }
        }
        return;
    } else if(srcA == 255) {
        int dstACmp;
        for(int x = left; x < right; ++x) {
            GPixel* dst = fDevice.getAddr(x, y);
            if(GPixel_GetA(*dst) == 255) {
                continue;
            } else if(GPixel_GetA(*dst) == 0) {
                GPixel* dst = fDevice.getAddr(x, y);
                *dst = GPixel_PackARGB(GPixel_GetA(src) + GPixel_GetA(*dst), 
                                        GPixel_GetR(src) + GPixel_GetR(*dst),
                                        GPixel_GetG(src) + GPixel_GetG(*dst),
                                        GPixel_GetB(src) + GPixel_GetB(*dst));
                continue;
            }
            dstACmp = 255 - GPixel_GetA(*dst);
            *dst = GPixel_PackARGB(GPixel_GetA(*dst) + GDiv255(dstACmp * GPixel_GetA(src)),
                                    GPixel_GetR(*dst) + GDiv255(dstACmp * GPixel_GetR(src)), 
                                    GPixel_GetG(*dst) + GDiv255(dstACmp * GPixel_GetG(src)), 
                                    GPixel_GetB(*dst) + GDiv255(dstACmp * GPixel_GetB(src)));
        }
        return;
    }
    for(int x = left; x < right; ++x) {
        GPixel* dst = fDevice.getAddr(x, y);
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
static void kXor(int left, int right, int y, GPixel& src, const GBitmap& fDevice) {
    if(GPixel_GetA(src) == 0) {
        kDstOver(left, right, y, src, fDevice);
        return;
    } else if(GPixel_GetA(src) == 255) {
        kSrcOut(left, right, y, src, fDevice);
        return;
    }
    int dstACmp;
    const int srcACmp = 255 - GPixel_GetA(src);
    for(int x = left; x < right; ++x) {
        GPixel* dst = fDevice.getAddr(x, y);
        dstACmp = 255 - GPixel_GetA(*dst);
        *dst = GPixel_PackARGB(GDiv255(srcACmp * GPixel_GetA(*dst) + dstACmp * GPixel_GetA(src)), 
                                GDiv255(srcACmp * GPixel_GetR(*dst)+ dstACmp * GPixel_GetR(src)), 
                                GDiv255(srcACmp * GPixel_GetG(*dst) + dstACmp * GPixel_GetG(src)), 
                                GDiv255(srcACmp * GPixel_GetB(*dst) + dstACmp * GPixel_GetB(src)));
    }
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
    kXor};

#endif