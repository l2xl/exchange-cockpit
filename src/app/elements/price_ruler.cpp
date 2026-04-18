// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#include "price_ruler.hpp"
#include "scratch_panel.hpp"

#include <string>

#include <include/core/SkCanvas.h>
#include <include/core/SkColor.h>
#include <include/core/SkFontMetrics.h>
#include <include/core/SkPaint.h>
#include <include/core/SkRect.h>
#include <include/core/SkTextBlob.h>

#include "default_typeface.hpp"

namespace scratcher::elements {

PriceRuler::PriceRuler(currency<uint64_t> p)
    : cockpit::PriceRuler(std::move(p))
    , mFont(DefaultTypeface(), 12.0f)
{
    SkFontMetrics fm;
    mFont.getMetrics(&fm);
    mTextHeight = static_cast<int>(fm.fDescent - fm.fAscent);
    SkRect bounds;
    mFont.measureText("O", 1, SkTextEncoding::kUTF8, &bounds);
    mCharWidth = static_cast<int>(bounds.width());
}

void PriceRuler::Paint(cockpit::IChartPanel& w) const
{
    auto& sp = static_cast<ScratchPanel&>(w);
    SkCanvas& canvas = sp.BackingCanvas();

    SkPaint linePaint;
    linePaint.setColor(SK_ColorGRAY);
    linePaint.setStrokeWidth(1.0f);
    linePaint.setStyle(SkPaint::kStroke_Style);

    canvas.drawLine(SkPoint::Make(mAxisX, mAxisTop), SkPoint::Make(mAxisX, mAxisBottom), linePaint);

    SkPaint textPaint;
    textPaint.setColor(SK_ColorGRAY);

    for (uint64_t t : mTicks) {
        int y = sp.DataYToWidgetY(t);
        canvas.drawLine(SkPoint::Make(mTickX1, y), SkPoint::Make(mTickX2, y), linePaint);

        currency<uint64_t> price = mPoint;
        price.set_raw(t);
        std::string label = price.to_string();
        auto blob = SkTextBlob::MakeFromString(label.c_str(), mFont);
        if (blob) canvas.drawTextBlob(blob, static_cast<SkScalar>(mLabelX), static_cast<SkScalar>(y + mTextHeight / 2 - 2), textPaint);
    }
}

}
