#pragma once
// 频谱主题注册表
// 添加新主题：1) 建 theme_xxx.h  2) 在此 include + 注册
//
// Java 式继承：每个主题 struct 继承 ThemeDefaults，可选覆写 kParticleVert/kParticleFrag；
// 未覆写则自动沿用 shaders_embed.h 中的默认粒子着色器。

#include "shaders_embed.h"

// 默认粒子着色器 — 主题继承此基类即可获得默认实现
struct ThemeDefaults {
    static constexpr const char* kParticleVert = kDefaultParticleVert;
    static constexpr const char* kParticleFrag = kDefaultParticleFrag;
};

#include "theme_v/theme_bars.h"
#include "theme_v/theme_aura.h"
#include "theme_v/theme_gpt.h"

struct Theme {
    const char* name;
    const char* vert;
    const char* frag;
    const char* particleVert;
    const char* particleFrag;
};

#define REGISTER_THEME(ns) { ns::kName, ns::kVert, ns::kFrag, ns::kParticleVert, ns::kParticleFrag }

static constexpr Theme kThemes[] = {
    REGISTER_THEME(theme_gpt),
    REGISTER_THEME(theme_bars),
    REGISTER_THEME(theme_aura),
};

static constexpr int kThemeCount = sizeof(kThemes) / sizeof(kThemes[0]);
