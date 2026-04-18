// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#include "price_indicator.hpp"
#include "quote_scratcher.hpp"
#include "scratch_panel.hpp"

#include <string>

#include <include/core/SkCanvas.h>
#include <include/core/SkColor.h>
#include <include/core/SkFontMetrics.h>
#include <include/core/SkPaint.h>
#include <include/core/SkPathEffect.h>
#include <include/core/SkRect.h>
#include <include/core/SkTextBlob.h>
#include <include/effects/SkDashPathEffect.h>

#include "default_typeface.hpp"

namespace scratcher::elements {

PriceIndicator::PriceIndicator(currency<uint64_t> p)
    : cockpit::PriceIndicator(std::move(p))
    , mFont(DefaultTypeface(), 12.0f)
{
    SkFontMetrics fm;
    mFont.getMetrics(&fm);
    mTextHeight = fm.fDescent - fm.fAscent;
    SkRect bounds;
    mFont.measureText("0", 1, SkTextEncoding::kUTF8, &bounds);
    mCharWidth = bounds.width();
}

void PriceIndicator::Paint(cockpit::IChartPanel& w) const
{
    auto& sp = static_cast<ScratchPanel&>(w);
    auto lock = sp.LockTradeCache();
    if (sp.TradeCache().empty()) return;

    auto last = mPoint;
    last.set_raw(sp.TradeCache().back().price_points);
    int y = sp.DataYToWidgetY(last.raw());

    SkCanvas& canvas = sp.BackingCanvas();
    const cockpit::PixelRect& client = sp.GetClientRect();

    SkPaint linePaint;
    linePaint.setColor(SK_ColorGRAY);
    linePaint.setStyle(SkPaint::kStroke_Style);
    linePaint.setStrokeWidth(1.0f);
    SkScalar dashes[] = {3.0f, 3.0f};
    linePaint.setPathEffect(SkDashPathEffect::Make(SkSpan<const SkScalar>(dashes, 2), 0));
    canvas.drawLine(SkPoint::Make(client.left, y), SkPoint::Make(client.right, y), linePaint);

    auto active = sp.Quote() ? sp.Quote()->GetActiveCandle() : BuoyCandleQuotes::candle_t{};
    bool bullish = (active.volume == 0) || (last.raw() >= active.mean);

    std::string label = last.to_string();
    SkRect text_bounds;
    mFont.measureText(label.c_str(), label.size(), SkTextEncoding::kUTF8, &text_bounds);

    int char_w = static_cast<int>(mCharWidth);
    int label_x = client.right + 1 + char_w;
    SkRect box = SkRect::MakeXYWH(static_cast<SkScalar>(label_x),
        static_cast<SkScalar>(y - text_bounds.height() / 2 - 2),
        text_bounds.width() + char_w, text_bounds.height() + 4);

    SkPaint boxPaint;
    boxPaint.setStyle(SkPaint::kFill_Style);
    boxPaint.setColor(bullish ? SK_ColorGREEN : SK_ColorRED);
    canvas.drawRect(box, boxPaint);

    SkPaint textPaint;
    textPaint.setColor(SK_ColorWHITE);
    auto blob = SkTextBlob::MakeFromString(label.c_str(), mFont);
    if (blob) canvas.drawTextBlob(blob, box.fLeft + char_w / 2.0f, box.fTop + text_bounds.height() + 1, textPaint);
}

}
