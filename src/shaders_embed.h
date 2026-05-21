#pragma once
// 内嵌 GLSL 着色器源码 — 构建时无需附带 shaders/ 文件夹

// 全屏三角形
static constexpr const char* kFullscreenVert = R"glsl(
#version 430 core

out vec2 vUV;


void main()
{
    // 3 个顶点覆盖整个屏幕
    vec2 p = vec2(float((gl_VertexID & 1) << 2),
                  float((gl_VertexID & 2) << 1)) - 1.0;
    vUV = (p + 1.0) * 0.5;
    gl_Position = vec4(p, 0.0, 1.0);
}
)glsl";

//  可视化顶点着色器
static constexpr const char* kVisualizerVert = R"glsl(
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

// 可视化片段着色器
static constexpr const char* kVisualizerFrag = R"glsl(
#version 430 core

in vec2  vUV;
in float vMagnitude;
in float vNormAngle;

out vec4 FragColor;

uniform float uTime;


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

// Bloom 降采样
static constexpr const char* kBloomDownFrag = R"glsl(
#version 430 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec2      uTexelSize;   // 1.0 / 纹理分辨率
uniform float     uThreshold;   // 亮度阈值（仅高于此值的像素保留）


void main()
{
    // 2×2 Box 滤波
    vec3 a = texture(uTexture, vUV + vec2(-uTexelSize.x, -uTexelSize.y) * 0.5).rgb;
    vec3 b = texture(uTexture, vUV + vec2( uTexelSize.x, -uTexelSize.y) * 0.5).rgb;
    vec3 c = texture(uTexture, vUV + vec2(-uTexelSize.x,  uTexelSize.y) * 0.5).rgb;
    vec3 d = texture(uTexture, vUV + vec2( uTexelSize.x,  uTexelSize.y) * 0.5).rgb;

    vec3 color = (a + b + c + d) * 0.25;

    // 亮度阈值：低于阈值的像素不参与 Bloom，避免暗部模糊导致雾感
    float luma = dot(color, vec3(0.299, 0.587, 0.114));
    float scale = clamp((luma - uThreshold) / max(luma, 1e-6), 0.0, 1.0);
    color *= scale;

    FragColor = vec4(color, 1.0);
}
)glsl";

//  Bloom 升采样 + 累加
static constexpr const char* kBloomUpFrag = R"glsl(
#version 430 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uSource;       // 低分辨率源
uniform sampler2D uDest;         // 当前层（累加目标）
uniform vec2      uTexelSize;    // 1.0 / 源纹理分辨率


void main()
{
    float x = uTexelSize.x;
    float y = uTexelSize.y;

    // 3×3 Tent 滤波
    vec3 a = texture(uSource, vUV + vec2(-x, -y)).rgb;
    vec3 b = texture(uSource, vUV + vec2( 0, -y)).rgb;
    vec3 c = texture(uSource, vUV + vec2( x, -y)).rgb;
    vec3 d = texture(uSource, vUV + vec2(-x,  0)).rgb;
    vec3 e = texture(uSource, vUV            ).rgb;
    vec3 f = texture(uSource, vUV + vec2( x,  0)).rgb;
    vec3 g = texture(uSource, vUV + vec2(-x,  y)).rgb;
    vec3 h = texture(uSource, vUV + vec2( 0,  y)).rgb;
    vec3 i = texture(uSource, vUV + vec2( x,  y)).rgb;

    vec3 upsampled = (e * 4.0 + (b+d+f+h) * 2.0 + (a+c+g+i)) / 16.0;

    //vec3 existing = texture(uDest, vUV).rgb;
    FragColor = vec4(upsampled, 1.0);
}
)glsl";

// 最终合成
static constexpr const char* kCompositeFrag = R"glsl(
#version 430 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uScene;
uniform sampler2D uBloom;
uniform float     uBloomStrength;


void main()
{
    vec3 scene = texture(uScene, vUV).rgb;
    vec3 bloom = texture(uBloom, vUV).rgb;

    vec3 color = scene + bloom * uBloomStrength;

    // Reinhard 色调映射
    color = color / (1.0 + color);

    // Gamma 校正
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
)glsl";

// 粒子顶点
static constexpr const char* kParticleVert = R"glsl(

#version 430 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in float aSize;

uniform mat4 uProjection;

out vec4 vColor;

void main()
{
    gl_Position  = uProjection * vec4(aPos, 0.0, 1.0);
    gl_PointSize = aSize;
    vColor       = aColor;
}
)glsl";


//粒子片段
static constexpr const char* kParticleFrag = R"glsl(

#version 430 core

in vec4 vColor;
out vec4 FragColor;

void main()
{
    float d     = length(gl_PointCoord - 0.5);
    float alpha = smoothstep(0.5, 0.08, d);
    float glow  = exp(-d * d * 10.0) * 0.6;

    FragColor = vec4(vColor.rgb + glow, vColor.a * alpha);
}
)glsl";
