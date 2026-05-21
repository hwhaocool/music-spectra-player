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
