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
    // 暖琥珀色身体，耳朵偏粉
    float hue = mix(0.105, 0.06, vIsEar);   // 琥珀 → 粉
    hue += sin(uTime * 0.35 + vNormAngle * 6.28) * 0.015;

    float sat = mix(0.70, 0.55, vIsEar);
    vec3 color = hsv2rgb(vec3(fract(hue), sat, 1.0));

    // 亮度随频谱强度变化
    float brightness = 0.30 + vMagnitude * 0.70;
    color *= brightness;

    // 耳朵内侧微光
    color += color * vIsEar * (1.0 - abs(vUV.y - 0.5) * 2.0) * 0.45;

    // 条柱中心高光
    float hot = 1.0 - abs(vUV.x - 0.5) * 2.0;
    hot = pow(hot, 2.5);
    color += color * hot * 0.25;

    // 微弱脉动
    color *= 0.94 + 0.06 * sin(uTime * 2.3 + vNormAngle * 12.57);

    // 边缘柔化
    float alpha  = smoothstep(0.0, 0.09, vUV.x) * smoothstep(1.0, 0.91, vUV.x);
    alpha       *= smoothstep(0.0, 0.10, vUV.y) * smoothstep(1.0, 0.90, vUV.y);
    alpha       *= 0.22 + vMagnitude * 0.78;

    FragColor = vec4(color, alpha);
}
