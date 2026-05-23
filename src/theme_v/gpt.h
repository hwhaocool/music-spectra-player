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
    vec4 K = vec4(   1.0,   2.0/3.0,   1.0/3.0,   3.0    );
    vec3 p =   abs(  fract(c.xxx + K.xyz) * 6.0  - K.www   );
    return   c.z *   mix(  K.xxx,  clamp(p - K.xxx, 0.0, 1.0),  c.y   );
}

void main()
{
    vec2 uv =   vUV * 2.0 - 1.0;
    float r =   length(uv);
    float a =   atan(uv.y, uv.x);

    // 极坐标扭曲
    a +=   sin(r * 12.0 - uTime * 3.0) *   0.25;
    float ring =   smoothstep(0.45, 0.42, r) *   smoothstep(0.75, 0.70, r);
    float hue =   fract(  a / 6.28318 +  uTime * 0.05   );
    vec3 color =   hsv2rgb(  vec3( hue, 0.9, 2.0  )   );

    // glow
    color *= ring * 2.5;

    FragColor =   vec4(color, ring);
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
    result += texture(uSource, vUV).rgb* 4.0;
    result += texture(uSource, vUV + vec2( x,  0)).rgb * 2.0;

    result += texture(uSource, vUV + vec2(-x,  y)).rgb * 1.0;
    result += texture(uSource, vUV + vec2( 0,  y)).rgb * 2.0;
    result += texture(uSource, vUV + vec2( x,  y)).rgb * 1.0;

    result /= 16.0;

    FragColor = vec4(result, 1.0);
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
    return fract(  sin(dot(p, vec2(12.9898,78.233))) *  43758.5453  );
}

void main()
{
    vec3 scene = texture(uScene, vUV).rgb;

    vec3 bloom = texture(uBloom, vUV).rgb;

    // ========================================================
    // [OPT]
    // dirt bloom
    // ========================================================

    vec3 color = scene + bloom * uBloomStrength;

    color +=  bloom * bloom * 0.35;

    // ========================================================
    // [NEW]
    // radial fog
    // ========================================================

    float dist = length(vUV - 0.5);

    color += vec3(0.02, 0.01, 0.05) * (1.0 - dist);

    // ========================================================
    // [NEW]
    // vignette
    // ========================================================

    float vignette = smoothstep(1.1, 0.15, dist);

    color *= vignette;

    // ========================================================
    // [NEW]
    // film grain
    // ========================================================

    float grain = random(vUV + uTime) * 0.03;

    color += grain;

    // ========================================================
    // Tone Mapping
    // ========================================================

    color = color / (1.0 + color);

    // ========================================================
    // Gamma
    // ========================================================

    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
)glsl";





