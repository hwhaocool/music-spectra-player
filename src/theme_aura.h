#pragma once
// 主题：流光圆环（连续渐变光晕）

namespace theme_aura {
static constexpr const char* kName = "Aura Ring";

static constexpr const char* kVert = R"glsl(
#version 430 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in float aAngle;
layout(location = 2) in float aMagnitude;

uniform mat4  uProjection;
uniform float uRadius;
uniform float uBarMaxHeight;
uniform float uBarWidth;

out vec2  vUV;
out float vMagnitude;
out float vNormAngle;

void main()
{
    vUV        = aPos;
    vMagnitude = aMagnitude;
    vNormAngle = aAngle / (2.0 * 3.14159265);

    float ang  = aAngle + (aPos.x - 0.5) * uBarWidth;
    float rad  = uRadius + aPos.y * aMagnitude * uBarMaxHeight;
    vec2  pos  = vec2(cos(ang), sin(ang)) * rad;

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
    // 柱间融合 — 淡化个体边界，形成连续光环
    float edge = abs(vUV.x - 0.5) * 2.0;
    float blend = 1.0 - smoothstep(0.0, 1.0, edge);
    blend = pow(blend, 0.6);

    // 径向渐变：中间亮、边缘淡
    float radial = 1.0 - abs(vUV.y - 0.5) * 2.0;
    radial = pow(radial, 1.5);

    // 彩虹色环绕 + 时间流动
    float hue = vNormAngle + uTime * 0.04;
    vec3 color = hsv2rgb(vec3(hue, 0.75, 1.0));

    // 音量控制亮度
    float brightness = 0.25 + vMagnitude * 0.75;
    color *= brightness * blend * radial;

    // 脉动微光
    color *= 0.92 + 0.08 * sin(uTime * 1.8 + vNormAngle * 12.566);

    float alpha = blend * (0.3 + vMagnitude * 0.7);
    FragColor = vec4(color, alpha);
}
)glsl";
} // namespace theme_aura
