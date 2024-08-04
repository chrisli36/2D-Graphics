/*
 *  Copyright 2018 Mike Reed
 */

#include "../include/GPath.h"
#include "../include/GMatrix.h"

GPath::GPath() {}
GPath::~GPath() {}

GPath& GPath::operator=(const GPath& src) {
    if (this != &src) {
        fPts = src.fPts;
        fVbs = src.fVbs;
    }
    return *this;
}

void GPath::reset() {
    fPts.clear();
    fVbs.clear();
}

void GPath::dump() const {
    Iter iter(*this);
    GPoint pts[2];
    while (auto v = iter.next(pts)) {
        switch (*v) {
            case kMove: printf("M %g %g\n", pts[0].x, pts[0].y); break;
            case kLine: printf("L %g %g\n", pts[1].x, pts[1].y); break;
        }
    }
}

/////////////////////////////////////////////////////////////////

GPath::Iter::Iter(const GPath& path) {
    fPrevMove = nullptr;
    fCurrPt = path.fPts.data();
    fCurrVb = path.fVbs.data();
    fStopVb = fCurrVb + path.fVbs.size();
}

std::optional<GPath::Verb> GPath::Iter::next(GPoint pts[]) {
    assert(fCurrVb <= fStopVb);
    if (fCurrVb == fStopVb) {
        return {};
    }
    Verb v = *fCurrVb++;
    switch (v) {
        case kMove:
            fPrevMove = fCurrPt;
            pts[0] = *fCurrPt++;
            break;
        case kLine:
            pts[0] = fCurrPt[-1];
            pts[1] = *fCurrPt++;
            break;
    }
    return v;
}

GPath::Edger::Edger(const GPath& path) {
    fPrevMove = nullptr;
    fCurrPt = path.fPts.data();
    fCurrVb = path.fVbs.data();
    fStopVb = fCurrVb + path.fVbs.size();
    fPrevVerb = GPath::Verb(-1);
}

std::optional<GPath::Verb> GPath::Edger::next(GPoint pts[]) {
    assert(fCurrVb <= fStopVb);
    bool do_return = false;
    while (fCurrVb < fStopVb) {
        switch (*fCurrVb++) {
            case kMove:
                if (fPrevVerb == kLine) {
                    pts[0] = fCurrPt[-1];
                    pts[1] = *fPrevMove;
                    do_return = true;
                }
                fPrevMove = fCurrPt++;
                fPrevVerb = kMove;
                break;
            case kLine:
                pts[0] = fCurrPt[-1];
                pts[1] = *fCurrPt++;
                do_return = true;
                fPrevVerb = kLine;
                break;
        }
        if (do_return) {
            return kLine;
        }
    }
    if (fPrevVerb == kLine) {
        pts[0] = fCurrPt[-1];
        pts[1] = *fPrevMove;
        fPrevVerb = GPath::Verb(-1);
        return kLine;
    } else {
        return {};
    }
}
