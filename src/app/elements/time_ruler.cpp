// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#include "time_ruler.hpp"
#include "scratch_panel.hpp"

#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

#include <include/core/SkCanvas.h>
#include <include/core/SkColor.h>
#include <include/core/SkFontMetrics.h>
#include <include/core/SkPaint.h>
#include <include/core/SkTextBlob.h>

#include "default_typeface.hpp"

namespace scratcher::elements {

namespace {

std::string FormatDate(uint64_t ms)
{
    std::time_t t = static_cast<std::time_t>(ms / 1000);
    std::tm tm_buf{};
    gmtime_r(&t, &tm_buf);
    std::ostringstream out;
    out << std::put_time(&tm_buf, "%Y-%m-%d");
    return out.str();
}

}

TimeRuler::TimeRuler()
    : mFont(DefaultTypeface(), 12.0f)
{
    SkFontMetrics fm;
    mFont.getMetrics(&fm);
    mTextHeight = static_cast<int>(fm.fDescent - fm.fAscent);
}

void TimeRuler::Paint(cockpit::IChartPanel& w) const
{
    auto& sp = static_cast<ScratchPanel&>(w);
    SkCanvas& canvas = sp.BackingCanvas();

    SkPaint linePaint;
    linePaint.setColor(SK_ColorGRAY);
    linePaint.setStrokeWidth(1.0f);
    linePaint.setStyle(SkPaint::kStroke_Style);

    canvas.drawLine(SkPoint::Make(mAxisLeft, mAxisY), SkPoint::Make(mAxisRight, mAxisY), linePaint);

    for (uint64_t t : mTicks) {
        int x = sp.DataXToWidgetX(t);
        canvas.drawLine(SkPoint::Make(x, mTickY1), SkPoint::Make(x, mTickY2), linePaint);
    }

    SkPaint textPaint;
    textPaint.setColor(SK_ColorGRAY);
    std::string label = FormatDate(sp.GetDataViewRect().x);
    auto blob = SkTextBlob::MakeFromString(label.c_str(), mFont);
    if (blob) canvas.drawTextBlob(blob, 5.0f, static_cast<SkScalar>(sp.GetClientRect().bottom + mTextHeight * 2), textPaint);
}

}
