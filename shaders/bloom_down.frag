#version 430 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec2      uTexelSize;   // 1.0 / 纹理分辨率

// Bloom 降采样
void main()
{
    // 2×2 Box 滤波
    vec3 a = texture(uTexture, vUV + vec2(-uTexelSize.x, -uTexelSize.y) * 0.5).rgb;
    vec3 b = texture(uTexture, vUV + vec2( uTexelSize.x, -uTexelSize.y) * 0.5).rgb;
    vec3 c = texture(uTexture, vUV + vec2(-uTexelSize.x,  uTexelSize.y) * 0.5).rgb;
    vec3 d = texture(uTexture, vUV + vec2( uTexelSize.x,  uTexelSize.y) * 0.5).rgb;

    FragColor = vec4((a + b + c + d) * 0.25, 1.0);
}
