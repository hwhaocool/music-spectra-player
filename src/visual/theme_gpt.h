#pragma once
// 主题：流光圆环（连续渐变光晕）

namespace theme_gpt {
static constexpr const char* kName = "GPT";

static constexpr const char* kVert = R"glsl(
#version 430 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in float aAngle;
layout(location = 2) in float aMagnitude;

uniform mat4  uProjection;
uniform float uRadius;
uniform float uBarMaxHeight;
uniform float uBarWidth;

uniform float uTime;
uniform float uPulseStrength;

out vec2  vUV;
out float vMagnitude;
out float vNormAngle;

void main()
{
    vUV        = aPos;
    vMagnitude = aMagnitude;
    vNormAngle = aAngle / (2.0 * 3.14159265);

    // 微弱角度扭曲
    float warp = sin(aAngle * 6.0 + uTime * 2.5) * 0.015 * aMagnitude;

    // 条形在圆上的角度分布
    float ang  = aAngle + (aPos.x - 0.5) * uBarWidth + warp;

    // 脉冲动画
    float pulse = sin(uTime * 4.0 + aAngle * 8.0) * uPulseStrength;
    float height =  aMagnitude *  (1.0 + pulse);

    // 极坐标定位
    float rad  = uRadius + aPos.y * height * uBarMaxHeight;

    //vRadialPos = rad;

    vec2  pos  = vec2(cos(ang), sin(ang)) * rad;

    // 投影输出
    gl_Position = uProjection * vec4(pos, 0.0, 1.0);
}
)glsl";

static constexpr const char* kFrag = R"glsl(
#version 430 core

in vec2  vUV;
in float vMagnitude;
in float vNormAngle;

out vec4 FragColor;

uniform float uTime;

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    // ========================================================
    // [NEW]
    // 动态 hue 流动
    // ========================================================

    float hue =
        0.55 +
        vNormAngle * 0.45 +
        sin(uTime * 0.5 + vNormAngle * 8.0) * 0.03;

    vec3 neon =
        hsv2rgb(
            vec3(
                fract(hue),
                0.95,
                2.5
            )
        );

    // ========================================================
    // [NEW]
    // 亮度随音量变化
    // ========================================================

    neon *=  0.5 +  vMagnitude * 1.2;

    // ========================================================
    // [NEW]
    // 中心辉光
    // ========================================================

    float centerGlow =
        pow(
            1.0 - abs(vUV.x - 0.5) * 2.0,
            4.0
        );

    neon +=  neon * centerGlow * 1.5;

    // ========================================================
    // [NEW]
    // 外边缘 bloom
    // ========================================================

    float edgeGlow =  smoothstep(0.6, 1.0, vUV.y);

    neon +=  neon * edgeGlow * 0.9;

    // ========================================================
    // [NEW]
    // 动态 flicker
    // ========================================================

    float flicker =
        sin(
            uTime * 14.0 +
            vNormAngle * 30.0
        ) * 0.04;

    neon *= 1.0 + flicker;

    // ========================================================
    // [OPT]
    // alpha 柔化
    // ========================================================

    float alpha =
        smoothstep(0.0, 0.08, vUV.x) *
        smoothstep(1.0, 0.92, vUV.x);

    alpha *=
        smoothstep(0.0, 0.08, vUV.y) *
        smoothstep(1.0, 0.95, vUV.y);

    alpha *=  0.2 +  vMagnitude;

    FragColor = vec4(neon, alpha);
}
)glsl";
} // namespace theme_gpt
