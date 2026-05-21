#pragma once

// ============================================================
// shaders_embed.h
// 高级霓虹音乐播放器 Shader 集合
//
// 改动说明：
// [NEW]  = 新增
// [OPT]  = 优化
// [TUNE] = 参数调优
// ============================================================


// ============================================================
// 全屏三角形
// ============================================================

static constexpr const char* kFullscreenVert = R"glsl(
#version 430 core

out vec2 vUV;

void main()
{
    vec2 p = vec2(
        float((gl_VertexID & 1) << 2),
        float((gl_VertexID & 2) << 1)
    ) - 1.0;

    vUV = (p + 1.0) * 0.5;

    gl_Position = vec4(p, 0.0, 1.0);
}
)glsl";


// ============================================================
// 可视化顶点 Shader
//
// [OPT]
// 1. 增加 radial warp
// 2. 增加 pulse
// 3. 增加 outer glow scale
// ============================================================

static constexpr const char* kVisualizerVert = R"glsl(
#version 430 core

layout(location = 0) in vec2 aPos;

layout(location = 1) in float aAngle;
layout(location = 2) in float aMagnitude;

uniform mat4  uProjection;

uniform float uRadius;
uniform float uBarMaxHeight;
uniform float uBarWidth;

uniform float uTime;

// [NEW]
uniform float uPulseStrength;

out vec2  vUV;
out float vMagnitude;
out float vNormAngle;
out float vRadialPos;

void main()
{
    vUV        = aPos;
    vMagnitude = aMagnitude;

    vNormAngle = aAngle / (2.0 * 3.14159265);

    // [NEW]
    // 微弱角度扭曲
    float warp =
        sin(aAngle * 6.0 + uTime * 2.5) *
        0.015 *
        aMagnitude;

    float ang =
        aAngle +
        (aPos.x - 0.5) * uBarWidth +
        warp;

    // [NEW]
    // pulse
    float pulse =
        sin(uTime * 4.0 + aAngle * 8.0) *
        uPulseStrength;

    float height =  aMagnitude *  (1.0 + pulse);

    float rad = uRadius + aPos.y * height * uBarMaxHeight;

    vRadialPos = rad;

    vec2 pos = vec2(cos(ang), sin(ang)) * rad;

    gl_Position =  uProjection * vec4(pos, 0.0, 1.0);
}
)glsl";


// ============================================================
// 可视化片段 Shader
//
// [OPT]
// 1. 高级霓虹边缘
// 2. HDR 发光
// 3. 动态色彩流动
// 4. 外圈高光
// 5. 柔性 alpha
// ============================================================

static constexpr const char* kVisualizerFrag = R"glsl(
#version 430 core

in vec2  vUV;
in float vMagnitude;
in float vNormAngle;
in float vRadialPos;

out vec4 FragColor;

uniform float uTime;


// ============================================================
// HSV → RGB
// ============================================================

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(
        1.0,
        2.0 / 3.0,
        1.0 / 3.0,
        3.0
    );

    vec3 p =
        abs(
            fract(c.xxx + K.xyz) * 6.0
            - K.www
        );

    return
        c.z *
        mix(
            K.xxx,
            clamp(p - K.xxx, 0.0, 1.0),
            c.y
        );
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

    neon *=
        0.5 +
        vMagnitude * 1.2;

    // ========================================================
    // [NEW]
    // 中心辉光
    // ========================================================

    float centerGlow =
        pow(
            1.0 - abs(vUV.x - 0.5) * 2.0,
            4.0
        );

    neon +=
        neon *
        centerGlow *
        1.5;

    // ========================================================
    // [NEW]
    // 外边缘 bloom
    // ========================================================

    float edgeGlow =
        smoothstep(0.6, 1.0, vUV.y);

    neon +=
        neon *
        edgeGlow *
        0.9;

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

    alpha *=
        0.2 +
        vMagnitude;

    FragColor =
        vec4(neon, alpha);
}
)glsl";


// ============================================================
// [NEW]
// 中心能量旋涡 Shader
//
// 这是高级感核心
// ============================================================

static constexpr const char* kCenterVortexFrag = R"glsl(
#version 430 core

in vec2 vUV;

out vec4 FragColor;

uniform float uTime;

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(
        1.0,
        2.0/3.0,
        1.0/3.0,
        3.0
    );

    vec3 p =
        abs(
            fract(c.xxx + K.xyz) * 6.0
            - K.www
        );

    return
        c.z *
        mix(
            K.xxx,
            clamp(p - K.xxx, 0.0, 1.0),
            c.y
        );
}

void main()
{
    vec2 uv =
        vUV * 2.0 - 1.0;

    float r =
        length(uv);

    float a =
        atan(uv.y, uv.x);

    // ========================================================
    // [NEW]
    // 极坐标扭曲
    // ========================================================

    a +=
        sin(r * 12.0 - uTime * 3.0) *
        0.25;

    float ring =
        smoothstep(0.45, 0.42, r) *
        smoothstep(0.75, 0.70, r);

    float hue =
        fract(
            a / 6.28318 +
            uTime * 0.05
        );

    vec3 color =
        hsv2rgb(
            vec3(
                hue,
                0.9,
                2.0
            )
        );

    // ========================================================
    // glow
    // ========================================================

    color *= ring * 2.5;

    FragColor =
        vec4(color, ring);
}
)glsl";


// ============================================================
// Bloom Downsample
//
// [OPT]
// 更强高亮提取
// ============================================================

static constexpr const char* kBloomDownFrag = R"glsl(
#version 430 core

in vec2 vUV;

out vec4 FragColor;

uniform sampler2D uTexture;

uniform vec2  uTexelSize;
uniform float uThreshold;

void main()
{
    vec3 a =
        texture(
            uTexture,
            vUV + vec2(-uTexelSize.x, -uTexelSize.y)
        ).rgb;

    vec3 b =
        texture(
            uTexture,
            vUV + vec2( uTexelSize.x, -uTexelSize.y)
        ).rgb;

    vec3 c =
        texture(
            uTexture,
            vUV + vec2(-uTexelSize.x,  uTexelSize.y)
        ).rgb;

    vec3 d =
        texture(
            uTexture,
            vUV + vec2( uTexelSize.x,  uTexelSize.y)
        ).rgb;

    vec3 color =
        (a + b + c + d) * 0.25;

    float luma =
        dot(
            color,
            vec3(0.299, 0.587, 0.114)
        );

    // ========================================================
    // [OPT]
    // soft threshold
    // ========================================================

    float knee = 0.5;

    float soft =
        clamp(
            (luma - uThreshold + knee) /
            (2.0 * knee),
            0.0,
            1.0
        );

    soft = soft * soft * (3.0 - 2.0 * soft);

    color *= soft;

    FragColor =
        vec4(color, 1.0);
}
)glsl";


// ============================================================
// Bloom Upsample
//
// [OPT]
// 更柔和 tent blur
// ============================================================

static constexpr const char* kBloomUpFrag = R"glsl(
#version 430 core

in vec2 vUV;

out vec4 FragColor;

uniform sampler2D uSource;

uniform vec2 uTexelSize;

void main()
{
    float x = uTexelSize.x;
    float y = uTexelSize.y;

    vec3 result = vec3(0.0);

    result += texture(uSource, vUV + vec2(-x, -y)).rgb * 1.0;
    result += texture(uSource, vUV + vec2( 0, -y)).rgb * 2.0;
    result += texture(uSource, vUV + vec2( x, -y)).rgb * 1.0;

    result += texture(uSource, vUV + vec2(-x,  0)).rgb * 2.0;
    result += texture(uSource, vUV).rgb               * 4.0;
    result += texture(uSource, vUV + vec2( x,  0)).rgb * 2.0;

    result += texture(uSource, vUV + vec2(-x,  y)).rgb * 1.0;
    result += texture(uSource, vUV + vec2( 0,  y)).rgb * 2.0;
    result += texture(uSource, vUV + vec2( x,  y)).rgb * 1.0;

    result /= 16.0;

    FragColor =
        vec4(result, 1.0);
}
)glsl";


// ============================================================
// [NEW]
// AfterImage 残影
//
// 高级感核心
// ============================================================

static constexpr const char* kAfterImageFrag = R"glsl(
#version 430 core

in vec2 vUV;

out vec4 FragColor;

uniform sampler2D uCurrent;
uniform sampler2D uHistory;

uniform float uFeedback;

void main()
{
    vec3 current =
        texture(uCurrent, vUV).rgb;

    vec3 history =
        texture(uHistory, vUV).rgb;

    // ========================================================
    // [NEW]
    // 残影混合
    // ========================================================

    vec3 color =
        mix(
            current,
            history,
            uFeedback
        );

    FragColor =
        vec4(color, 1.0);
}
)glsl";


// ============================================================
// [NEW]
// Chromatic Aberration
// ============================================================

static constexpr const char* kChromaticFrag = R"glsl(
#version 430 core

in vec2 vUV;

out vec4 FragColor;

uniform sampler2D uTexture;

uniform float uStrength;

void main()
{
    vec2 center =
        vec2(0.5);

    vec2 dir =
        (vUV - center) *
        uStrength;

    float r =
        texture(
            uTexture,
            vUV + dir
        ).r;

    float g =
        texture(
            uTexture,
            vUV
        ).g;

    float b =
        texture(
            uTexture,
            vUV - dir
        ).b;

    FragColor =
        vec4(r, g, b, 1.0);
}
)glsl";


// ============================================================
// 最终合成
//
// [OPT]
// 1. Dirt bloom
// 2. vignette
// 3. radial fog
// 4. film grain
// ============================================================

static constexpr const char* kCompositeFrag = R"glsl(
#version 430 core

in vec2 vUV;

out vec4 FragColor;

uniform sampler2D uScene;
uniform sampler2D uBloom;

uniform float uBloomStrength;
uniform float uTime;

float random(vec2 p)
{
    return fract(
        sin(dot(p, vec2(12.9898,78.233))) *
        43758.5453
    );
}

void main()
{
    vec3 scene =
        texture(uScene, vUV).rgb;

    vec3 bloom =
        texture(uBloom, vUV).rgb;

    // ========================================================
    // [OPT]
    // dirt bloom
    // ========================================================

    vec3 color =
        scene +
        bloom * uBloomStrength;

    color +=
        bloom * bloom * 0.35;

    // ========================================================
    // [NEW]
    // radial fog
    // ========================================================

    float dist =
        length(vUV - 0.5);

    color +=
        vec3(0.02, 0.01, 0.05) *
        (1.0 - dist);

    // ========================================================
    // [NEW]
    // vignette
    // ========================================================

    float vignette =
        smoothstep(
            1.1,
            0.15,
            dist
        );

    color *= vignette;

    // ========================================================
    // [NEW]
    // film grain
    // ========================================================

    float grain =
        random(vUV + uTime) * 0.03;

    color += grain;

    // ========================================================
    // Tone Mapping
    // ========================================================

    color =
        color /
        (1.0 + color);

    // ========================================================
    // Gamma
    // ========================================================

    color =
        pow(
            color,
            vec3(1.0 / 2.2)
        );

    FragColor =
        vec4(color, 1.0);
}
)glsl";


// ============================================================
// 粒子顶点 Shader
//
// [OPT]
// 粒子动态大小
// ============================================================

static constexpr const char* kParticleVert = R"glsl(
#version 430 core

layout(location = 0) in vec2  aPos;
layout(location = 1) in vec4  aColor;
layout(location = 2) in float aSize;

uniform mat4 uProjection;

out vec4 vColor;

void main()
{
    gl_Position =
        uProjection *
        vec4(aPos, 0.0, 1.0);

    gl_PointSize =
        aSize;

    vColor =
        aColor;
}
)glsl";


// ============================================================
// 粒子 Fragment
//
// [OPT]
// 更高级 glow
// ============================================================

static constexpr const char* kParticleFrag = R"glsl(
#version 430 core

in vec4 vColor;

out vec4 FragColor;

void main()
{
    vec2 uv =
        gl_PointCoord - 0.5;

    float d =
        length(uv);

    float alpha =
        smoothstep(
            0.5,
            0.02,
            d
        );

    float glow =
        exp(-d * d * 18.0);

    vec3 color =
        vColor.rgb *
        (1.0 + glow * 2.5);

    FragColor =
        vec4(
            color,
            alpha * vColor.a
        );
}
)glsl";