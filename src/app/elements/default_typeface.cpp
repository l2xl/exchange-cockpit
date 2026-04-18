// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#include "default_typeface.hpp"

#include <include/core/SkFontMgr.h>
#include <include/core/SkFontStyle.h>
#include <include/ports/SkFontMgr_fontconfig.h>
#include <include/ports/SkFontScanner_FreeType.h>

namespace scratcher::elements {

sk_sp<SkTypeface> DefaultTypeface()
{
    static sk_sp<SkTypeface> tf = []() -> sk_sp<SkTypeface> {
        auto mgr = SkFontMgr_New_FontConfig(nullptr, SkFontScanner_Make_FreeType());
        if (!mgr) return SkTypeface::MakeEmpty();
        sk_sp<SkTypeface> face(mgr->matchFamilyStyle(nullptr, SkFontStyle()));
        if (!face) return SkTypeface::MakeEmpty();
        return face;
    }();
    return tf;
}

}
