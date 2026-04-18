// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include <include/core/SkRefCnt.h>
#include <include/core/SkSurface.h>

extern "C" { typedef struct _cairo_surface cairo_surface_t; }

namespace scratcher::elements {

class CairoSkiaBridge
{
    int mWidth = 0;
    int mHeight = 0;
    int mStride = 0;
    std::vector<uint8_t> mPixels;
    sk_sp<SkSurface> mSurface;
    cairo_surface_t* mCairoSurface = nullptr;

public:
    CairoSkiaBridge() = default;
    CairoSkiaBridge(const CairoSkiaBridge&) = delete;
    CairoSkiaBridge& operator=(const CairoSkiaBridge&) = delete;
    ~CairoSkiaBridge();

    void Resize(int width, int height);

    int Width() const { return mWidth; }
    int Height() const { return mHeight; }
    int Stride() const { return mStride; }

    SkSurface& Surface() const { return *mSurface; }
    cairo_surface_t* CairoSurface() const { return mCairoSurface; }
};

}
