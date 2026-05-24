#version 430 core

in vec2  vUV;
in float vMagnitude;
in float vNormAngle;
in float vIsEar;

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
    // ── 霓虹渐变：橘 → 粉 → 紫，环绕猫轮廓 ──
    // hue 从 0.08 (橘) 递减穿过 0.93 (粉) 到 0.78 (紫)，fract 自动处理环绕
    float hue = 0.08
              - vNormAngle * 0.30             // 橘→粉→紫
              + vIsEar * 0.04                 // 耳朵偏粉
              + sin(uTime * 0.45 + vNormAngle * 7.5) * 0.025;

    float sat = 0.92 - vIsEar * 0.15;
    float val = 2.2;                          // HDR 高亮
    vec3 neon = hsv2rgb(vec3(fract(hue), sat, val));

    // 亮度随频谱强度变化
    neon *= 0.45 + vMagnitude * 1.1;

    // 条柱中心辉光
    float centerGlow = pow(1.0 - abs(vUV.x - 0.5) * 2.0, 3.5);
    neon += neon * centerGlow * 1.3;

    // 外边缘 bloom
    float edgeGlow = smoothstep(0.55, 1.0, vUV.y);
    neon += neon * edgeGlow * 0.7;

    // 耳朵内侧高亮
    float earInner = (1.0 - abs(vUV.y - 0.5) * 2.0);
    earInner = pow(earInner, 1.8);
    neon += neon * vIsEar * earInner * 0.55;

    // 动态 flicker
    float flicker = sin(uTime * 13.0 + vNormAngle * 28.0) * 0.035;
    neon *= 1.0 + flicker;

    // 边缘柔化
    float alpha  = smoothstep(0.0, 0.07, vUV.x) * smoothstep(1.0, 0.93, vUV.x);
    alpha       *= smoothstep(0.0, 0.08, vUV.y) * smoothstep(1.0, 0.94, vUV.y);
    alpha       *= 0.18 + vMagnitude * 0.82;

    FragColor = vec4(neon, alpha);
}
