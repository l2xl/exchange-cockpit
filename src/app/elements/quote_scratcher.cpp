// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#include "quote_scratcher.hpp"
#include "scratch_panel.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkColor.h>
#include <include/core/SkPaint.h>

namespace scratcher::elements {

namespace {

void PaintBuoy(SkCanvas& canvas, uint64_t buoy_duration, uint64_t buoy_ts,
               const BuoyCandleQuotes::candle_t& buoy, const BuoyCandleQuotes::candle_t& prev_buoy,
               ScratchPanel& w)
{
    SkPaint greenPaint;
    greenPaint.setColor(SK_ColorGREEN);
    greenPaint.setStrokeWidth(1.0f);
    greenPaint.setStyle(SkPaint::kStroke_Style);

    SkPaint redPaint = greenPaint;
    redPaint.setColor(SK_ColorRED);

    uint64_t mean_time = buoy_ts + buoy_duration / 2;
    SkPoint maxPt  = SkPoint::Make(w.DataXToWidgetX(mean_time), w.DataYToWidgetY(buoy.max));
    SkPoint meanPt = SkPoint::Make(w.DataXToWidgetX(mean_time), w.DataYToWidgetY(buoy.mean));
    SkPoint minPt  = SkPoint::Make(w.DataXToWidgetX(mean_time), w.DataYToWidgetY(buoy.min));

    if (prev_buoy.max <= buoy.max) canvas.drawLine(maxPt, meanPt, greenPaint);
    if (prev_buoy.min <= buoy.min) canvas.drawLine(minPt, meanPt, greenPaint);

    if (prev_buoy.max > buoy.max)  canvas.drawLine(maxPt, meanPt, redPaint);
    if (prev_buoy.min > buoy.min)  canvas.drawLine(minPt, meanPt, redPaint);

    const SkPaint& meanPaint = (prev_buoy.mean <= buoy.mean) ? greenPaint : redPaint;
    int x_start = w.DataXToWidgetX(buoy_ts);
    int x_end   = w.DataXToWidgetX(buoy_ts + buoy_duration - 1);
    int y_mean  = w.DataYToWidgetY(buoy.mean);
    canvas.drawLine(SkPoint::Make(x_start, y_mean), SkPoint::Make(x_end, y_mean), meanPaint);
}

}

void QuoteScratcher::Paint(cockpit::IChartPanel& w) const
{
    if (!FirstBuoyTimestamp()) return;

    auto& sp = static_cast<ScratchPanel&>(w);
    SkCanvas& canvas = sp.BackingCanvas();

    uint64_t buoy_time = *FirstBuoyTimestamp();
    const auto& quotes = GetQuotes();
    uint64_t duration = BuoyDuration();

    BuoyCandleQuotes::candle_t prev{};
    bool first = true;
    for (const auto& cur : quotes) {
        if (buoy_time >= sp.GetDataViewRect().x_start()
            && buoy_time + duration <= sp.GetDataViewRect().x_end()) {
            PaintBuoy(canvas, duration, buoy_time, cur, first ? cur : prev, sp);
        }
        prev = cur;
        first = false;
        buoy_time += duration;
    }

    BuoyCandleQuotes::candle_t active = GetActiveCandle();
    if (active.volume > 0
        && buoy_time >= sp.GetDataViewRect().x_start()
        && buoy_time + duration < sp.GetDataViewRect().x_end()) {
        BuoyCandleQuotes::candle_t prev_for_active = quotes.empty() ? active : quotes.back();
        PaintBuoy(canvas, duration, buoy_time, active, prev_for_active, sp);
    }
}

}
