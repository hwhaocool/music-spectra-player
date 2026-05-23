#pragma once
// 主题：霓虹柱（当前默认样式）

struct theme_bars : ThemeDefaults {
static constexpr const char* kName = "Neon Bars";

// 顶点着色器
static constexpr const char* kVert = R"glsl(
#version 430 core

// ── 逐顶点 ──
layout(location = 0) in vec2 aPos;        // 单元四边形 (0,0)→(1,1)

// ── 逐实例 ──
layout(location = 1) in float aAngle;     // 弧度
layout(location = 2) in float aMagnitude; // 0‑1

uniform mat4  uProjection;
uniform float uRadius;        // 内圆半径
uniform float uBarMaxHeight;  // 柱体最大延伸
uniform float uBarWidth;      // 角宽度（弧度）

out vec2  vUV;
out float vMagnitude;
out float vNormAngle;         // 0‑1 归一化角度

//  可视化顶点着色器
void main()
{
    vUV        = aPos;
    vMagnitude = aMagnitude;
    vNormAngle = aAngle / (2.0 * 3.14159265);

    // 把单元四边形映射为径向柱体
    float ang  = aAngle + (aPos.x - 0.5) * uBarWidth;
    float rad  = uRadius + aPos.y * aMagnitude * uBarMaxHeight;
    vec2  pos  = vec2(cos(ang), sin(ang)) * rad;

    gl_Position = uProjection * vec4(pos, 0.0, 1.0);
}
)glsl";

// 片段着色器
static constexpr const char* kFrag = R"glsl(
#version 430 core

in vec2  vUV;
in float vMagnitude;
in float vNormAngle;

out vec4 FragColor;

uniform float uTime;

// 可视化片段着色器
// ── HSV → RGB ──
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    // 霓虹渐变：cyan(0.50) → magenta(0.90)
    // 粉 0.95
    float hue = 0.70 + vNormAngle * 0.35;

    // 饱和度 0.85
    // 亮度 1.0
    vec3 neon  = hsv2rgb(vec3(hue, 0.95, 2.0));

    // 亮度随 音量 提升
    neon *= 0.55 + vMagnitude * 0.45;

    // 微弱脉动
    neon *= 0.96 + 0.04 * sin(uTime * 2.5 + vNormAngle * 6.2832);

    // 中心高光
    neon += neon * (1.0 - abs(vUV.x - 0.5) * 2.0) * 0.25;

    // 柔化四边
    float a  = smoothstep(0.0, 0.08, vUV.x) * smoothstep(1.0, 0.92, vUV.x);
    a       *= smoothstep(0.0, 0.12, vUV.y) * smoothstep(1.0, 0.88, vUV.y);

    FragColor = vec4(neon, a * vMagnitude);
}
)glsl";
}; // struct theme_bars
