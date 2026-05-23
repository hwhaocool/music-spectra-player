#version 430 core

in vec2 vUV;
in float vMagnitude;
in float vNormAngle;

out vec4 FragColor;

uniform float uTime;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    // 动态 hue 流动

    float hue = 0.55 +
        vNormAngle * 0.45 +
        sin(uTime * 0.5 + vNormAngle * 8.0) * 0.03;

    vec3 neon = hsv2rgb(vec3(fract(hue), 0.95, 2.5));

    // 亮度随音量变化
    neon *= 0.5 + vMagnitude * 1.2;

    // 中心辉光
    float centerGlow = pow(1.0 - abs(vUV.x - 0.5) * 2.0, 4.0);

    neon += neon * centerGlow * 1.5;

    // 外边缘 bloom
    float edgeGlow = smoothstep(0.6, 1.0, vUV.y);

    neon += neon * edgeGlow * 0.9;

    // 动态 flicker
    float flicker = sin(uTime * 14.0 +
        vNormAngle * 30.0) * 0.04;

    neon *= 1.0 + flicker;

    // alpha 柔化
    float alpha = smoothstep(0.0, 0.08, vUV.x) *
        smoothstep(1.0, 0.92, vUV.x);

    alpha *= smoothstep(0.0, 0.08, vUV.y) *
        smoothstep(1.0, 0.95, vUV.y);

    alpha *= 0.2 + vMagnitude;

    FragColor = vec4(neon, alpha);
}