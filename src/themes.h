#pragma once
// 频谱主题注册表
// 添加新主题：1) 建 theme_xxx.h  2) 在此 include + 注册

#include "theme_bars.h"
#include "theme_aura.h"

struct Theme {
    const char* name;
    const char* vert;
    const char* frag;
};

#define REGISTER_THEME(ns) { ns::kName, ns::kVert, ns::kFrag }

static constexpr Theme kThemes[] = {
    REGISTER_THEME(theme_bars),
    REGISTER_THEME(theme_aura),
};

static constexpr int kThemeCount = sizeof(kThemes) / sizeof(kThemes[0]);
