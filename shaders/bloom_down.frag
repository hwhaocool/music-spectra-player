#version 430 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec2      uTexelSize;   // 1.0 / 纹理分辨率
uniform float     uThreshold;   // 亮度阈值（仅高于此值的像素保留）

// Bloom 降采样
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
