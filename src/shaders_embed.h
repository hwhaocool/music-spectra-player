#pragma once
// 内嵌 GLSL 着色器源码 — 构建时无需附带 shaders/ 文件夹

static constexpr const char* kFullscreenVert = R"glsl(
#version 430 core

out vec2 vUV;

void main()
{
    vec2 p = vec2(float((gl_VertexID & 1) << 2),
                  float((gl_VertexID & 2) << 1)) - 1.0;
    vUV = (p + 1.0) * 0.5;
    gl_Position = vec4(p, 0.0, 1.0);
}
)glsl";

// gpt bloom 降采样【顶部可以发光】
static constexpr const char* kBloomDownFrag2 = R"glsl(
#version 430 core

in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uTexture;
uniform vec2  uTexelSize;
uniform float uThreshold;

void main()
{
    vec3 a = texture( uTexture, vUV + vec2(-uTexelSize.x, -uTexelSize.y)).rgb;
    vec3 b = texture( uTexture, vUV + vec2( uTexelSize.x, -uTexelSize.y)).rgb;
    vec3 c = texture( uTexture, vUV + vec2(-uTexelSize.x, uTexelSize.y)).rgb;
    vec3 d = texture( uTexture, vUV + vec2( uTexelSize.x, uTexelSize.y)).rgb;

    vec3 color =  (a + b + c + d) * 0.25;
    float luma =  dot(  color,  vec3(0.299, 0.587, 0.114)   );

    // ========================================================
    // [OPT]
    // soft threshold
    // ========================================================

    float knee = 0.5;
    float soft =  clamp(  (luma - uThreshold + knee) /  (2.0 * knee),  0.0,  1.0   );
    soft = soft * soft * (3.0 - 2.0 * soft);
    color *= soft;

    FragColor =  vec4(color, 1.0);
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

// GPT Bloom 升采样， 柔和一些，最高点没有那么亮
static constexpr const char* kBloomUpFrag2 = R"glsl(
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
    result += texture(uSource, vUV).rgb* 4.0;
    result += texture(uSource, vUV + vec2( x,  0)).rgb * 2.0;

    result += texture(uSource, vUV + vec2(-x,  y)).rgb * 1.0;
    result += texture(uSource, vUV + vec2( 0,  y)).rgb * 2.0;
    result += texture(uSource, vUV + vec2( x,  y)).rgb * 1.0;

    result /= 16.0;

    FragColor = vec4(result, 1.0);
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
uniform float uBloomStrength;


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

// 默认粒子顶点（主题可覆写）
static constexpr const char* kDefaultParticleVert = R"glsl(
#version 430 core
// default

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


// 默认粒子片段（主题可覆写）
static constexpr const char* kDefaultParticleFrag = R"glsl(
#version 430 core
// default

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



// 中心能量旋涡 Shader
static constexpr const char* kCenterVortexFrag = R"glsl(
#version 430 core

in vec2 vUV;

out vec4 FragColor;

uniform float uTime;
uniform vec2  uInnerRadius;   // NDC 空间内圆半径 (x, y)，与条形图内圆对齐

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    // 居中并做长宽比修正，使圆在屏幕空间保持正圆
    vec2 uv = (vUV * 2.0 - 1.0) / uInnerRadius;
    float r = length(uv);
    float a = atan(uv.y, uv.x);

    // 极坐标扭曲（越靠外越强）
    a += sin(r * 8.0 - uTime * 3.0) * 0.20 * min(r, 1.0);

    // 涡旋环：内圈淡入，外圈淡出
    float ring = smoothstep(0.35, 0.40, r)
               * smoothstep(1.00, 0.88, r);

    float hue = fract(a / 6.28318 + uTime * 0.05);
    vec3 color = hsv2rgb(vec3(hue, 0.9, 1.5));

    // 超出内圆完全透明
    float alpha = ring * smoothstep(1.02, 0.92, r);

    color *= alpha * 2.5;

    FragColor = vec4(color, alpha);
}
)glsl";

