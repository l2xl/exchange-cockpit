// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#include "cairo_skia_bridge.hpp"

#include <cairo/cairo.h>

#include <include/core/SkColorSpace.h>
#include <include/core/SkColorType.h>
#include <include/core/SkImageInfo.h>

namespace scratcher::elements {

CairoSkiaBridge::~CairoSkiaBridge()
{
    if (mCairoSurface) cairo_surface_destroy(mCairoSurface);
}

void CairoSkiaBridge::Resize(int width, int height)
{
    if (width <= 0 || height <= 0) return;
    if (width == mWidth && height == mHeight) return;

    mWidth = width;
    mHeight = height;
    mStride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
    mPixels.assign(static_cast<size_t>(mStride) * height, 0);

    auto info = SkImageInfo::Make(width, height, kBGRA_8888_SkColorType, kPremul_SkAlphaType);
    mSurface = SkSurfaces::WrapPixels(info, mPixels.data(), mStride);

    if (mCairoSurface) cairo_surface_destroy(mCairoSurface);
    mCairoSurface = cairo_image_surface_create_for_data(mPixels.data(), CAIRO_FORMAT_ARGB32, width, height, mStride);
}

}
